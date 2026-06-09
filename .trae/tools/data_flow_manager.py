#!/usr/bin/env python3
"""
data_flow_manager.py — 数据流向管理工具

功能：
1. 生成数据流向表模板
2. 验证数据流向的一致性
3. 生成 Link 函数代码
4. 生成 Switcher 注册代码

Usage:
    python data_flow_manager.py init <project>
    python data_flow_manager.py add <module> --input <source> --output <target>
    python data_flow_manager.py validate
    python data_flow_manager.py generate link
    python data_flow_manager.py generate switcher
"""

import json
import os
import argparse
from pathlib import Path
from typing import Dict, List, Optional

# ANSI color codes
RED = '\033[31m'
GREEN = '\033[32m'
YELLOW = '\033[33m'
CYAN = '\033[36m'
RESET = '\033[0m'
BOLD = '\033[1m'


def color(text: str, code: str) -> str:
    return f"{code}{text}{RESET}"


def error(msg: str) -> str:
    return color(f"[FAIL] {msg}", RED)


def ok(msg: str) -> str:
    return color(f"[PASS] {msg}", GREEN)


def info(msg: str) -> str:
    return color(f"[INFO] {msg}", CYAN)


class DataFlowManager:
    """数据流向管理器"""

    def __init__(self, project_root: Optional[str] = None):
        self.project_root = Path(project_root) if project_root else Path.cwd()
        self.flow_file = self.project_root / 'data_flow_table.json'
        self.flow_data: Dict = {}

    def load_flow(self) -> bool:
        """加载数据流向表"""
        if not self.flow_file.exists():
            print(error(f"数据流向表不存在: {self.flow_file}"))
            return False

        with open(self.flow_file, 'r', encoding='utf-8') as f:
            self.flow_data = json.load(f)
        return True

    def save_flow(self):
        """保存数据流向表"""
        with open(self.flow_file, 'w', encoding='utf-8') as f:
            json.dump(self.flow_data, f, indent=2, ensure_ascii=False)
        print(ok(f"数据流向表已保存: {self.flow_file}"))

    def init_project(self, project_name: str):
        """初始化数据流向表"""
        if self.flow_file.exists():
            print(warn(f"数据流向表已存在，将覆盖: {self.flow_file}"))

        self.flow_data = {
            "project": project_name,
            "version": "2.2",
            "modules": [],
            "slots": {}
        }
        self.save_flow()
        print(info(f"项目 '{project_name}' 数据流向表已初始化"))

    def add_module(self, module_name: str, layer: str, 
                   inputs: List[str] = None, outputs: List[str] = None):
        """添加模块及其输入输出"""
        if not self.flow_data:
            self.load_flow()

        # 检查模块是否已存在
        existing = next((m for m in self.flow_data['modules'] 
                        if m['name'] == module_name), None)
        if existing:
            print(warn(f"模块 '{module_name}' 已存在，将更新"))
            module = existing
        else:
            module = {
                "name": module_name,
                "layer": layer,
                "inputs": [],
                "outputs": []
            }
            self.flow_data['modules'].append(module)

        # 添加输入
        if inputs:
            for src in inputs:
                if not any(i['source'] == src for i in module['inputs']):
                    module['inputs'].append({
                        "source": src,
                        "slot": f"SLOT_{src.upper()}",
                        "fields": []
                    })
                    print(info(f"添加输入: {module_name} ← {src}"))

        # 添加输出
        if outputs:
            for dst in outputs:
                if not any(o['target'] == dst for o in module['outputs']):
                    module['outputs'].append({
                        "target": dst,
                        "slot": f"SLOT_{dst.upper()}",
                        "fields": []
                    })
                    print(info(f"添加输出: {module_name} → {dst}"))

        self.save_flow()

    def validate(self) -> bool:
        """验证数据流向的一致性"""
        if not self.load_flow():
            return False

        print(f"\n{color('='*60, BOLD)}")
        print(f"{color('数据流向一致性验证', BOLD)}")
        print(f"{color('='*60, BOLD)}")

        all_pass = True
        errors = []

        # 检查1: 输入来源必须是已定义的模块
        for module in self.flow_data['modules']:
            for inp in module['inputs']:
                src_exists = any(m['name'] == inp['source'] 
                               for m in self.flow_data['modules'])
                if not src_exists:
                    errors.append(f"模块 '{module['name']}' 的输入来源 '{inp['source']}' 未定义")
                    all_pass = False

        # 检查2: 输出目标必须是已定义的模块
        for module in self.flow_data['modules']:
            for out in module['outputs']:
                dst_exists = any(m['name'] == out['target'] 
                               for m in self.flow_data['modules'])
                if not dst_exists:
                    errors.append(f"模块 '{module['name']}' 的输出目标 '{out['target']}' 未定义")
                    all_pass = False

        # 检查3: 层依赖检查（app → drv → hal）
        layer_order = {'core': 0, 'proto': 1, 'hal': 2, 'drv': 3, 'app': 4}
        for module in self.flow_data['modules']:
            dst_layer = layer_order.get(module['layer'], 5)
            for inp in module['inputs']:
                src_module = next((m for m in self.flow_data['modules'] 
                                  if m['name'] == inp['source']), None)
                if src_module:
                    src_layer = layer_order.get(src_module['layer'], 5)
                    if src_layer > dst_layer:
                        errors.append(f"层依赖违规: {module['layer']}::{module['name']} ← {src_module['layer']}::{inp['source']}")
                        all_pass = False

        # 检查4: 无循环依赖
        for module in self.flow_data['modules']:
            visited = set()
            if self._has_cycle(module['name'], visited):
                errors.append(f"循环依赖检测: {module['name']}")
                all_pass = False

        # 输出结果
        if errors:
            print(f"\n{color('违规项:', RED)}")
            for err in errors:
                print(f"  {error(err)}")
        else:
            print(ok("所有检查通过"))

        print(f"\n{color('='*60, BOLD)}")
        return all_pass

    def _has_cycle(self, module_name: str, visited: set) -> bool:
        """检测循环依赖"""
        if module_name in visited:
            return True
        visited.add(module_name)

        module = next((m for m in self.flow_data['modules'] 
                      if m['name'] == module_name), None)
        if not module:
            return False

        for inp in module['inputs']:
            if self._has_cycle(inp['source'], visited.copy()):
                return True
        return False

    def generate_link_functions(self) -> str:
        """生成 Link 函数代码"""
        if not self.flow_data:
            self.load_flow()

        code = []
        code.append("/**")
        code.append(" * @file    link_functions.c")
        code.append(" * @brief   Auto-generated Link functions")
        code.append(" * @note    Generated by data_flow_manager.py")
        code.append(" */")
        code.append("")
        code.append("#include \"data_switcher.h\"")
        code.append("#include \"std_module.h\"")
        code.append("")

        for module in self.flow_data['modules']:
            for inp in module['inputs']:
                src = inp['source']
                dst = module['name']
                src_cap = src.replace('_', '').capitalize()
                dst_cap = dst.replace('_', '').capitalize()

                code.append(f"/* Link: {src} → {dst} */")
                code.append(f"void {src}_to_{dst}_Link(void)")
                code.append("{")
                code.append(f"    {src_cap}_Output_t *src = ({src_cap}_Output_t *)s_slot[{inp['slot']}].pOut->para;")
                code.append(f"    {dst_cap}_Input_t *dst = ({dst_cap}_Input_t *)s_slot[SLOT_{dst.upper()}].pIn->para;")
                code.append("")
                if inp['fields']:
                    for field in inp['fields']:
                        code.append(f"    dst->from{src_cap}.{field} = src->{field};")
                else:
                    code.append("    /* TODO: 填写字段映射 */")
                code.append("}")
                code.append("")

        return '\n'.join(code)

    def generate_switcher_registration(self) -> str:
        """生成 Switcher 注册代码"""
        if not self.flow_data:
            self.load_flow()

        code = []
        code.append("/**")
        code.append(" * @file    switcher_registration.c")
        code.append(" * @brief   Auto-generated Switcher registrations")
        code.append(" * @note    Generated by data_flow_manager.py")
        code.append(" */")
        code.append("")
        code.append("#include \"data_switcher.h\"")
        code.append("#include \"std_module.h\"")
        code.append("")
        code.append("void Switcher_Init(void)")
        code.append("{")

        # 模块注册
        code.append("    /* === 模块注册 === */")
        for module in self.flow_data['modules']:
            cap = module['name'].replace('_', '').capitalize()
            code.append(f"    {cap}_GetIO(&pIn, &pOut, &pDoWork);")
            code.append(f"    Switcher_Register(SLOT_{module['name'].upper()}, pDoWork, pOut);")
            code.append("")

        # Link 注册
        code.append("    /* === Link 注册 === */")
        for module in self.flow_data['modules']:
            for inp in module['inputs']:
                src = inp['source']
                dst = module['name']
                code.append(f"    Switcher_RegisterLink({inp['slot']}, SLOT_{dst.upper()}, {src}_to_{dst}_Link);")

        code.append("}")
        return '\n'.join(code)

    def generate_io_header(self, module_name: str) -> str:
        """生成 xxx_io.h 模板"""
        module = next((m for m in self.flow_data['modules'] 
                      if m['name'] == module_name), None)
        if not module:
            return error(f"模块 '{module_name}' 未找到")

        cap = module_name.replace('_', '').capitalize()
        upper = module_name.upper()

        code = []
        code.append(f"/**")
        code.append(f" * @file    {module_name}_io.h")
        code.append(f" * @brief   {cap} Data Switcher IO interface")
        code.append(f" * @layer   {module['layer']} (Data Switcher IO)")
        code.append(f" */")
        code.append(f"#ifndef {upper}_IO_H")
        code.append(f"#define {upper}_IO_H")
        code.append("")
        code.append("#include <stdint.h>")
        code.append("#include \"std_module.h\"")
        code.append("")

        # 输入子结构体
        for inp in module['inputs']:
            src_cap = inp['source'].replace('_', '').capitalize()
            code.append(f"/* ---- 输入: 来自 {inp['source']} ---- */")
            code.append(f"typedef struct {{")
            if inp['fields']:
                for field in inp['fields']:
                    code.append(f"    uint16_t {field};")
            else:
                code.append(f"    /* TODO: 添加 {inp['source']} 字段 */")
            code.append(f"}} {cap}_From{src_cap}_t;")
            code.append("")

        # 输入结构体
        code.append("/* ---- 输入结构体 ---- */")
        code.append(f"typedef struct {{")
        for inp in module['inputs']:
            src_cap = inp['source'].replace('_', '').capitalize()
            code.append(f"    {cap}_From{src_cap}_t from{src_cap};   /* → {inp['source']} */")
        code.append(f"}} {cap}_Input_t;")
        code.append("")

        # 输出结构体
        code.append("/* ---- 输出结构体 ---- */")
        code.append(f"typedef struct {{")
        code.append(f"    uint8_t  has_output;")
        code.append(f"    uint8_t  res[3];")
        code.append(f"    /* TODO: 添加输出字段 */")
        for out in module['outputs']:
            code.append(f"    /* → {out['target']} */")
        code.append(f"}} {cap}_Output_t;")
        code.append("")

        code.append(f"MODULE_IO_H({cap});")
        code.append("")
        code.append(f"#endif")

        return '\n'.join(code)

    def print_flow(self):
        """打印数据流向表"""
        if not self.flow_data:
            self.load_flow()

        print(f"\n{color('='*60, BOLD)}")
        print(f"{color('数据流向表', BOLD)}")
        print(f"{color('='*60, BOLD)}")
        print(f"项目: {self.flow_data.get('project', '未知')}")
        print(f"版本: {self.flow_data.get('version', '未知')}")
        print(f"模块数: {len(self.flow_data.get('modules', []))}")

        for module in self.flow_data.get('modules', []):
            print(f"\n{color(f\"[{module['layer']}] {module['name']}\", CYAN)}")
            if module['inputs']:
                print(f"  {color('输入:', BOLD)}")
                for inp in module['inputs']:
                    fields = ', '.join(inp['fields']) if inp['fields'] else '(未定义)'
                    print(f"    ← {inp['source']} [{inp['slot']}]")
                    print(f"      字段: {fields}")
            if module['outputs']:
                print(f"  {color('输出:', BOLD)}")
                for out in module['outputs']:
                    fields = ', '.join(out['fields']) if out['fields'] else '(未定义)'
                    print(f"    → {out['target']} [{out['slot']}]")
                    print(f"      字段: {fields}")

        print(f"\n{color('='*60, BOLD)}")


def main():
    parser = argparse.ArgumentParser(
        description='数据流向管理工具',
        formatter_class=argparse.RawDescriptionHelpFormatter
    )

    subparsers = parser.add_subparsers(dest='command', help='命令')

    # init 命令
    init_parser = subparsers.add_parser('init', help='初始化数据流向表')
    init_parser.add_argument('project', help='项目名称')

    # add 命令
    add_parser = subparsers.add_parser('add', help='添加模块')
    add_parser.add_argument('module', help='模块名称')
    add_parser.add_argument('--layer', required=True, help='所属层')
    add_parser.add_argument('--input', action='append', help='输入来源模块')
    add_parser.add_argument('--output', action='append', help='输出目标模块')

    # validate 命令
    subparsers.add_parser('validate', help='验证数据流向')

    # generate 命令
    gen_parser = subparsers.add_parser('generate', help='生成代码')
    gen_parser.add_argument('what', choices=['link', 'switcher', 'io'], help='生成类型')
    gen_parser.add_argument('--module', help='指定模块（仅 io）')

    # print 命令
    subparsers.add_parser('print', help='打印数据流向表')

    args = parser.parse_args()

    manager = DataFlowManager()

    if args.command == 'init':
        manager.init_project(args.project)

    elif args.command == 'add':
        manager.add_module(
            args.module,
            args.layer,
            args.input or [],
            args.output or []
        )

    elif args.command == 'validate':
        manager.validate()

    elif args.command == 'generate':
        if args.what == 'link':
            code = manager.generate_link_functions()
            print(code)
        elif args.what == 'switcher':
            code = manager.generate_switcher_registration()
            print(code)
        elif args.what == 'io':
            if args.module:
                code = manager.generate_io_header(args.module)
                print(code)
            else:
                print(error("请指定 --module 参数"))

    elif args.command == 'print':
        manager.print_flow()

    else:
        parser.print_help()


if __name__ == '__main__':
    main()
