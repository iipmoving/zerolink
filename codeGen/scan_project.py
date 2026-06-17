#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
scan_project.py — 纯 Python 实现：扫描源码 → project.json

替代 Claude CLI gen-json skill，直接从源文件解析模块/管道/字段。
不需要 AI，不需要 TTY，确定性强。

用法:
    python scan_project.py <项目路径> [--keil <uvprojx路径>]
"""

import os
import re
import json
import glob


# ── 正则表达式 ──

RE_LAYER = re.compile(r'@layer\s+(\S+)')

# @brief 行（取整行内容）
RE_BRIEF = re.compile(r'@brief\s+(.*)', re.MULTILINE)

# MODULE_IO_H(Name)  — 模块声明
RE_IO_H_NAME = re.compile(r'MODULE_IO_H\s*\(\s*(\w+)\s*\)')

# MODULE_SKELETON(Name)  — 仅在 .c 中
RE_MODULE_SKELETON = re.compile(r'MODULE_SKELETON\s*\(\s*(\w+)\s*\)')

# 独立 typedef struct { ... } TypeName;
RE_TYPEDEF = re.compile(r'typedef\s+struct\s*\{([^}]+)\}\s*(\w+);', re.DOTALL)

# 管道参数: struct { ... } MODULE_OUTPUT_PARAMS(From, To);
RE_OUTPUT_PARAMS = re.compile(r'struct\s*\{([^}]+)\}\s*MODULE_OUTPUT_PARAMS\s*\(\s*(\w+)\s*,\s*(\w+)\s*\)', re.DOTALL)
RE_INPUT_PARAMS  = re.compile(r'struct\s*\{([^}]+)\}\s*MODULE_INPUT_PARAMS\s*\(\s*(\w+)\s*,\s*(\w+)\s*\)', re.DOTALL)

# LINK 宏名匹配
RE_OUTPUT_LINK_MACRO = re.compile(r'MODULE_OUTPUT_LINK\s*\(\s*(\w+)\s*,\s*(\w+)\s*\)')
RE_INPUT_LINK_MACRO  = re.compile(r'MODULE_INPUT_LINK\s*\(\s*(\w+)\s*,\s*(\w+)\s*\)')

# 从 LINK 结构体中提取 params 声明: params[N] 或 *params
RE_LINK_PARAMS = re.compile(r'(\w+)\s*(\*?)\s*params\s*(?:\[(\d+(?:\s*\*\s*\d+)?)\])?')

# SLOT(Name) 在 data_switcher.c 中 — 只在 enum 定义中匹配
RE_SLOT_ENUM = re.compile(r'^\s*SLOT\s*\(\s*(\w+)\s*\)\s*=', re.MULTILINE)

# 字段行: type name; /* comment */
RE_FIELD_LINE = re.compile(r'^\s*(\S+(?:\s+\S+)*?)\s+(\w+)\s*;\s*(?:(?://|/\*)\s*(.*?)(?:\*/)?\s*)?$', re.MULTILINE)

# #include "..._io.h"
RE_INCLUDE_IO = re.compile(r'#include\s+"([^"]*_io\.h)"')


def to_snake_id(name):
    """PascalCase → snake_case id，处理带下划线的名称"""
    name = re.sub(r'([A-Z]+)([A-Z][a-z])', r'\1_\2', name)
    name = re.sub(r'([a-z0-9])([A-Z])', r'\1_\2', name)
    name = re.sub(r'[^a-zA-Z0-9]', '_', name)
    name = re.sub(r'_+', '_', name)
    name = name.strip('_').lower()
    return name


def read_file(path):
    try:
        with open(path, 'r', encoding='utf-8', errors='replace') as f:
            return f.read()
    except:
        return None


def parse_fields(struct_body):
    """从 struct 体解析字段列表"""
    fields = []
    for m in RE_FIELD_LINE.finditer(struct_body):
        ftype = m.group(1).strip()
        fname = m.group(2).strip()
        comment = (m.group(3) or '').strip()
        # 跳过 res/reserved
        if fname.startswith('res') or fname.startswith('reserved'):
            continue
        fields.append({"name": fname, "type": ftype, "comment": comment})
    return fields


def find_brief(content, default=""):
    """提取 @brief 行内容"""
    m = RE_BRIEF.search(content)
    if m:
        return m.group(1).strip()
    return default


def find_layer(content):
    """提取 @layer"""
    m = RE_LAYER.search(content)
    return m.group(1).strip() if m else "app"


def extract_link_style(content, from_mod, to_mod, link_type):
    """从 LINK 结构中提取 style 和 array_size"""
    macro = f'MODULE_{link_type}_LINK({from_mod}, {to_mod})'
    idx = content.find(macro)
    if idx == -1:
        return "pointer", 0
    # 在 macro 后面找 params 声明
    snippet = content[idx:idx+300]
    for m in RE_LINK_PARAMS.finditer(snippet):
        style = "pointer" if m.group(2) == '*' else "array"
        raw_size = m.group(3)
        if raw_size:
            try:
                # 支持 "4*20" 格式
                if '*' in raw_size:
                    parts = [int(x.strip()) for x in raw_size.split('*')]
                    array_size = parts[0] * parts[1] if len(parts) == 2 else parts[0]
                else:
                    array_size = int(raw_size)
            except:
                array_size = 0
        else:
            array_size = 0
        return style, array_size
    return "pointer", 0


def scan_io_file(path, proj_dir):
    """扫描单个 _io.h 文件，返回 (module_dict, pipe_list)"""
    content = read_file(path)
    if not content:
        return None

    rel_path = os.path.relpath(path, proj_dir).replace('\\', '/')

    # 模块名
    m = RE_IO_H_NAME.search(content)
    if not m:
        return None
    module_name = m.group(1)

    layer = find_layer(content)
    brief = find_brief(content)

    # 独立 typedef（排除宏包裹的）
    types = []
    for m in RE_TYPEDEF.finditer(content):
        tname = m.group(2).strip()
        if any(x in tname.upper() for x in ['OUTPUT', 'INPUT', 'LINK']):
            continue
        tbody = m.group(1)
        tfields = parse_fields(tbody)
        types.append({"name": tname, "comment": "", "fields": tfields})

    # 输出管道
    out_pipes = []
    for m in RE_OUTPUT_PARAMS.finditer(content):
        from_mod = m.group(2).strip()
        to_mod = m.group(3).strip()
        fields = parse_fields(m.group(1))
        out_link = extract_link_style(content, from_mod, to_mod, 'OUTPUT')
        in_link = extract_link_style(content, from_mod, to_mod, 'INPUT')
        pipe_id = to_snake_id(f"pipe_{from_mod}_{to_mod}")
        pipe = {
            "id": pipe_id,
            "from": from_mod,
            "to": to_mod,
            "callback_type": "pull",
            "comment": "",
            "out_link": {"style": out_link[0], "array_size": out_link[1]},
            "in_link": {"style": in_link[0], "array_size": in_link[1]},
            "fields": fields,
        }
        out_pipes.append(pipe)

    # 输入管道
    in_pipes = []
    for m in RE_INPUT_PARAMS.finditer(content):
        from_mod = m.group(2).strip()
        to_mod = m.group(3).strip()
        fields = parse_fields(m.group(1))
        out_link = extract_link_style(content, from_mod, to_mod, 'OUTPUT')
        in_link = extract_link_style(content, from_mod, to_mod, 'INPUT')
        pipe_id = to_snake_id(f"pipe_{from_mod}_{to_mod}")
        pipe = {
            "id": pipe_id,
            "from": from_mod,
            "to": to_mod,
            "callback_type": "pull",
            "comment": "",
            "out_link": {"style": out_link[0], "array_size": out_link[1]},
            "in_link": {"style": in_link[0], "array_size": in_link[1]},
            "fields": fields,
        }
        in_pipes.append(pipe)

    # 合并：输出管道优先（有完整字段），输入管道补充 in_link
    all_pipes = list(out_pipes)
    for ip in in_pipes:
        match = None
        for ep in all_pipes:
            if ep['from'] == ip['from'] and ep['to'] == ip['to']:
                match = ep
                break
        if match:
            # 用输入侧的 in_link 覆盖
            match['in_link'] = ip['in_link']
        else:
            all_pipes.append(ip)

    module = {
        "id": to_snake_id(module_name),
        "name": module_name,
        "layer": layer,
        "source_file": rel_path if not rel_path.endswith('.h') else "",
        "comment": brief,
        "types": types,
    }

    return module, all_pipes


def scan_source_files(proj_dir):
    """扫描 .c 文件中的 MODULE_SKELETON，返回 {name: source_file}"""
    result = {}
    for c_file in glob.glob(os.path.join(proj_dir, '**/*.c'), recursive=True):
        # 跳过 ProjectsOld
        if 'ProjectsOld' in c_file:
            continue
        content = read_file(c_file)
        if not content:
            continue
        for m in RE_MODULE_SKELETON.finditer(content):
            name = m.group(1)
            rel = os.path.relpath(c_file, proj_dir).replace('\\', '/')
            result[name] = rel
    return result


def scan_data_switcher(proj_dir):
    """从 data_switcher.c 提取 slot_order（只从 enum 定义中提取）"""
    sw_files = glob.glob(os.path.join(proj_dir, '**/data_switcher.c'), recursive=True)
    sw_files = [f for f in sw_files if 'ProjectsOld' not in f]
    if not sw_files:
        return []
    content = read_file(sw_files[0])
    if not content:
        return []
    slots = []
    for m in RE_SLOT_ENUM.finditer(content):
        name = m.group(1)
        if name != 'COUNT':
            slots.append(name)
    return slots


def scan_all_modules(proj_dir):
    """扫描所有 _io.h 文件，收集模块和管道"""
    all_modules = {}
    all_pipes = []

    io_files = glob.glob(os.path.join(proj_dir, '**/*_io.h'), recursive=True)
    io_files = [f for f in io_files if 'ProjectsOld' not in f]

    for io_path in sorted(io_files):
        result = scan_io_file(io_path, proj_dir)
        if result is None:
            continue
        module, pipes = result
        name = module['name']
        mod_id = module['id']

        # 去重：按 snake_case id 比较（处理 APP_POWER vs AppPower 差异）
        existing_key = None
        for k, m in all_modules.items():
            if m['id'] == mod_id:
                existing_key = k
                break

        if existing_key:
            # 同名碰撞 — 优先保留 PascalCase（替换 ALL_CAPS 版本）
            if existing_key == existing_key.upper() and name != name.upper():
                del all_modules[existing_key]
                all_modules[name] = module
            # 其他情况：保留第一个
            for p in pipes:
                _merge_pipe(all_pipes, p)
            continue

        all_modules[name] = module
        for p in pipes:
            _merge_pipe(all_pipes, p)

    return all_modules, all_pipes


def _merge_pipe(all_pipes, new_pipe):
    """管道去重"""
    key = (new_pipe['from'], new_pipe['to'])
    if not any(x['from'] == key[0] and x['to'] == key[1] for x in all_pipes):
        all_pipes.append(new_pipe)


def scan_project(proj_dir, keil_path=None, proj_name=None):
    """主入口：扫描项目并返回 project.json 数据结构"""
    proj_dir = os.path.abspath(proj_dir)

    # 1. 扫描 _io.h 获取模块和管道
    modules_dict, pipes = scan_all_modules(proj_dir)

    # 2. 扫描 MODULE_SKELETON 获取 source_file
    source_map = scan_source_files(proj_dir)

    # 3. 扫描 data_switcher.c 获取 slot_order
    slot_order = scan_data_switcher(proj_dir)

    # 4. 完善模块信息
    modules_list = []
    for name, mod in modules_dict.items():
        if name in source_map:
            mod['source_file'] = source_map[name]
        modules_list.append(mod)

    # 5. 按 slot_order 排序模块
    ordered = []
    remaining = list(modules_list)
    for slot_name in slot_order:
        for i, m in enumerate(remaining):
            if m['name'] == slot_name:
                ordered.append(m)
                remaining.pop(i)
                break
    ordered.extend(remaining)
    modules_list = ordered

    # 6. 管道排序：按 slot_order 中的 producer 顺序
    def pipe_sort_key(p):
        try:
            return slot_order.index(p['from'])
        except ValueError:
            return 999
    pipes.sort(key=pipe_sort_key)

    # 7. 构建输出
    if proj_name is None:
        proj_name = os.path.basename(proj_dir)

    project = {
        "schema_version": "2.3",
        "project": {
            "name": proj_name,
            "paths": {
                "io_dir": "include",
                "core_dir": "core",
                "app_dir": "app",
                "base_class_dir": "base_class",
                "proto_dir": "proto",
            },
            "std_module_path": "core/std_module.h",
            "pack": 4,
            "comment_guard": True,
            "output_root": proj_dir,
        },
        "modules": modules_list,
        "pipes": pipes,
        "slot_order": slot_order,
    }

    return project


def main():
    import sys
    if len(sys.argv) < 2:
        print("用法: python scan_project.py <项目路径> [--name <项目名称>] [--keil <uvprojx路径>]")
        sys.exit(1)

    proj_dir = sys.argv[1]
    proj_name = None
    keil_path = None
    if '--name' in sys.argv:
        idx = sys.argv.index('--name')
        proj_name = sys.argv[idx + 1]
    if '--keil' in sys.argv:
        idx = sys.argv.index('--keil')
        keil_path = sys.argv[idx + 1]

    data = scan_project(proj_dir, keil_path, proj_name)
    out_path = os.path.join(proj_dir, 'codeGen', 'out', data['project']['name'], 'project.json')
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    with open(out_path, 'w', encoding='utf-8') as f:
        json.dump(data, f, indent=2, ensure_ascii=False)
    print(f"[OK] project.json 已生成: {out_path}")
    print(f"     模块数: {len(data['modules'])}")
    print(f"     管道数: {len(data['pipes'])}")
    for m in data['modules']:
        print(f"       {m['name']:20s} layer={m['layer']:12s} src={m['source_file']}")
    for p in data['pipes']:
        print(f"       {p['from']:12s} → {p['to']:12s}  fields={len(p['fields'])}  out_link={p['out_link']['style']}")


if __name__ == "__main__":
    main()
