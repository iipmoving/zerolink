#!/usr/bin/env python3
"""
check_io_convention.py — v2.2 I/O 接口规范检查工具

检查 xxx_io.h 文件是否符合命名规范：
1. 类型命名: {Module}_Input_t, {Module}_Output_t
2. 禁止在 xxx_io.h 中声明 status/res 字段
3. Link 函数: {Src}_to_{Dst}_Link 参数类型正确
4. Consumer 回调: {Dst}_On{Src}Data 存在
5. xxx.c 中的 g_input.para / g_output.para 类型转换正确
6. 验证与数据流向表的一致性

Usage:
    python check_io_convention.py <module_name> [--project <project>]
    python check_io_convention.py --all
    python check_io_convention.py --self-test

Examples:
    python check_io_convention.py app_power
    python check_io_convention.py app_power --project four_head
    python check_io_convention.py --all
"""

import re
import sys
import os
import json
from pathlib import Path
from typing import List, Tuple, Optional, Dict

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


def warn(msg: str) -> str:
    return color(f"[WARN] {msg}", YELLOW)


def info(msg: str) -> str:
    return color(f"[INFO] {msg}", CYAN)


class IOConventionChecker:
    """I/O 接口规范检查器"""

    # 允许的类型名后缀（从类型名末尾提取）
    VALID_TYPE_SUFFIXES = [
        r'_Input_t$',
        r'_Output_t$',
        r'_From\w+_t$',       # _FromAdc_t, _FromComm_t
        r'_IO_DEF$',
    ]

    # 禁止在 xxx_io.h 中出现的字段名（框架字段）
    FORBIDDEN_FIELDS_IN_IO = [
        r'\bstatus\b',
        r'\b_res\b',
        r'\binfo\b',
        r'\broute\b',
    ]

    # Link 函数模式
    LINK_PATTERN = re.compile(
        r'(?P<src>\w+)_to_(?P<dst>\w+)_Link\s*\([^)]*\)'
    )

    # Consumer 回调模式
    CONSUMER_CALLBACK_PATTERN = re.compile(
        r'(?P<dst>\w+)_On(?P<src>\w+)Data\s*\(\s*Para_Grp_t\s*\*\s*\w+\s*\)'
    )

    # Para 类型转换模式
    PARA_CAST_PATTERN = re.compile(
        r'\(\s*(?P<module>\w+)_(?P<type>Input|Output)_t\s*\*\s*\)\s*(?:g_input|g_output)\.para'
    )

    def __init__(self, module_name: str, project_root: Optional[str] = None):
        self.module_name = module_name
        self.module_upper = module_name.upper()
        self.module_cap = self._to_cap(module_name)

        # 搜索路径
        if project_root:
            self.project_root = Path(project_root)
        else:
            # 自动检测项目根目录
            self.project_root = self._find_project_root()

        self.violations: List[Tuple[str, str, int]] = []  # (file, msg, line_num)
        self.warnings: List[Tuple[str, str, int]] = []

    def _to_cap(self, name: str) -> str:
        """snake_case 转 CamelCase"""
        return ''.join(word.capitalize() for word in name.split('_'))

    def _find_project_root(self) -> Path:
        """自动查找项目根目录"""
        # 查找包含 deps_config.json 或 CLAUDE.md 的目录
        current = Path.cwd()
        for parent in [current] + list(current.parents):
            if (parent / 'deps_config.json').exists() or \
               (parent / 'CLAUDE.md').exists():
                return parent
        return current

    def _find_io_file(self) -> Optional[Path]:
        """查找 xxx_io.h 文件"""
        search_dirs = [
            self.project_root / 'include',
            self.project_root / 'src' / 'include',
            self.project_root / 'app',
            self.project_root / 'src',
            self.project_root / 'core',
        ]

        # 尝试多种命名方式
        candidates = [
            f"{self.module_name}_io.h",
            f"{self.module_cap}_io.h",
        ]

        for search_dir in search_dirs:
            if not search_dir.exists():
                continue
            for candidate in candidates:
                io_file = search_dir / candidate
                if io_file.exists():
                    return io_file

        # 递归搜索
        for search_dir in search_dirs:
            if not search_dir.exists():
                continue
            for io_file in search_dir.rglob(f"*{self.module_name}*_io.h"):
                return io_file
            for io_file in search_dir.rglob(f"*{self.module_cap}*_io.h"):
                return io_file

        return None

    def _find_c_file(self) -> Optional[Path]:
        """查找 xxx.c 文件"""
        search_dirs = [
            self.project_root / 'app',
            self.project_root / 'drv',
            self.project_root / 'hal',
            self.project_root / 'proto',
            self.project_root / 'core',
            self.project_root / 'src',
        ]

        candidates = [
            f"{self.module_name}.c",
            f"{self.module_cap}.c",
        ]

        for search_dir in search_dirs:
            if not search_dir.exists():
                continue
            for candidate in candidates:
                c_file = search_dir / candidate
                if c_file.exists():
                    return c_file

        # 递归搜索
        for search_dir in search_dirs:
            if not search_dir.exists():
                continue
            for c_file in search_dir.rglob(f"{self.module_name}.c"):
                return c_file

        return None

    def _find_switcher_file(self) -> Optional[Path]:
        """查找 data_switcher.c 文件"""
        for pattern in ['data_switcher.c', 'switcher.c', 'DataSwitcher.c']:
            switcher = self.project_root / 'src' / 'core' / pattern
            if switcher.exists():
                return switcher
            switcher = self.project_root / 'core' / pattern
            if switcher.exists():
                return switcher
        return None

    def _check_type_name(self, type_name: str) -> bool:
        """检查类型名是否符合规范"""
        for pattern in self.VALID_TYPE_SUFFIXES:
            if re.search(pattern, type_name):
                return True
        return False

    def check_io_header(self, io_file: Path) -> bool:
        """检查 xxx_io.h 文件"""
        print(f"\n{color('--- 检查 I/O 头文件 ---', CYAN)}")
        print(f"{color('文件:', BOLD)} {io_file}")

        with open(io_file, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
            lines = content.split('\n')

        all_pass = True

        # 1. 检查类型命名
        type_defs = re.findall(r'typedef\s+struct\s*\{[^}]*\}\s*(\w+)\s*;', content, re.DOTALL)
        for type_name in type_defs:
            if not self._check_type_name(type_name):
                self.violations.append((str(io_file),
                    f"类型名 '{type_name}' 不符合规范，应以 {self.VALID_TYPE_SUFFIXES} 之一结尾", 0))
                all_pass = False

        if all_pass:
            print(ok(f"类型命名规范"))

        # 2. 检查禁止的字段（status/res/info/route）
        for line_num, line in enumerate(lines, 1):
            # 跳过注释
            if re.match(r'^\s*//', line) or re.match(r'^\s*/\*', line):
                continue

            for forbid in self.FORBIDDEN_FIELDS_IN_IO:
                if re.search(forbid, line) and 'typedef struct' not in line:
                    # 确认是在结构体内部（不是在注释中）
                    # 查找前面的 typedef struct
                    context_start = max(0, line_num - 20)
                    context = '\n'.join(lines[context_start:line_num])
                    if 'typedef struct' in context and '{' in context:
                        self.violations.append((str(io_file),
                            f"行 {line_num}: 禁止在 xxx_io.h 中声明框架字段 '{forbid}'", line_num))
                        all_pass = False
                        break

        if all_pass:
            print(ok(f"无框架字段 (status/res/info/route)"))

        # 3. 检查必须有 Input_t 和 Output_t
        has_input = bool(re.search(rf'{self.module_cap}_Input_t', content))
        has_output = bool(re.search(rf'{self.module_cap}_Output_t', content))

        if not has_input:
            self.violations.append((str(io_file),
                f"缺少 {self.module_cap}_Input_t 定义", 0))
            all_pass = False

        if not has_output:
            self.violations.append((str(io_file),
                f"缺少 {self.module_cap}_Output_t 定义", 0))
            all_pass = False

        if has_input and has_output:
            print(ok(f"Input/Output 类型已定义"))

        # 4. 检查 MODULE_IO_H 宏
        if not re.search(r'MODULE_IO_H\s*\(', content):
            self.warnings.append((str(io_file),
                f"缺少 MODULE_IO_H({self.module_cap}) 宏定义", 0))

        return all_pass

    def check_c_file(self, c_file: Path) -> bool:
        """检查 xxx.c 文件"""
        print(f"\n{color('--- 检查实现文件 ---', CYAN)}")
        print(f"{color('文件:', BOLD)} {c_file}")

        with open(c_file, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
            lines = content.split('\n')

        all_pass = True

        # 1. 检查 MODULE_SKELETON 宏
        skeleton_match = re.search(r'MODULE_SKELETON\s*\(\s*(\w+)\s*\)', content)
        if skeleton_match:
            module_arg = skeleton_match.group(1)
            if module_arg == self.module_cap:
                print(ok(f"MODULE_SKELETON({module_arg})"))
            else:
                self.violations.append((str(c_file),
                    f"MODULE_SKELETON 参数应为 {self.module_cap}，实际为 {module_arg}", 0))
                all_pass = False
        else:
            self.warnings.append((str(c_file), "缺少 MODULE_SKELETON 宏", 0))

        # 2. 检查 g_input.para / g_output.para 类型转换
        casts = self.PARA_CAST_PATTERN.findall(content)
        if casts:
            for module, io_type in casts:
                if module == self.module_cap:
                    print(ok(f"Para 类型转换: {module}_{io_type}_t * g_xxx.para"))
                else:
                    self.warnings.append((str(c_file),
                        f"Para 类型转换使用了 {module}_{io_type}_t，可能是错误的模块名", 0))
        else:
            self.warnings.append((str(c_file),
                "未找到 g_input.para / g_output.para 类型转换", 0))

        # 3. 检查 Consumer 回调
        consumer_cbs = self.CONSUMER_CALLBACK_PATTERN.findall(content)
        if consumer_cbs:
            for dst, src in consumer_cbs:
                if dst == self.module_cap:
                    print(ok(f"Consumer 回调: {dst}_On{src}Data"))
        else:
            self.warnings.append((str(c_file),
                "未找到 Consumer 回调 ({Module}_On{Source}Data)", 0))

        return all_pass

    def check_switcher(self, switcher_file: Path) -> bool:
        """检查 data_switcher.c 中的 Link 函数"""
        print(f"\n{color('--- 检查 Switcher ---', CYAN)}")
        print(f"{color('文件:', BOLD)} {switcher_file}")

        with open(switcher_file, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()

        all_pass = True

        # 1. 检查 Link 函数
        links = self.LINK_PATTERN.findall(content)
        module_links = [(src, dst) for src, dst in links
                       if dst.lower() == self.module_name.lower() or
                          dst.lower() == self.module_cap.lower()]

        if module_links:
            for src, dst in module_links:
                print(ok(f"Link 函数: {src}_to_{dst}_Link"))
        else:
            self.warnings.append((str(switcher_file),
                f"未找到 {self.module_cap} 的 Link 函数 ({Src}_to_{self.module_cap}_Link)", 0))

        # 2. 检查 Slot 枚举
        slot_pattern = re.compile(rf'SLOT_{self.module_upper}|SLOT_{self.module_cap}\b')
        if slot_pattern.search(content):
            print(ok(f"Slot 枚举: SLOT_{self.module_upper}"))
        else:
            self.warnings.append((str(switcher_file),
                f"未找到 SLOT_{self.module_upper} 枚举定义", 0))

        return all_pass

    def check_flow_consistency(self) -> bool:
        """检查与数据流向表的一致性"""
        flow_file = self.project_root / 'data_flow_table.json'
        if not flow_file.exists():
            self.warnings.append((str(flow_file), "数据流向表不存在", 0))
            return True

        print(f"\n{color('--- 检查数据流向表一致性 ---', CYAN)}")
        print(f"{color('文件:', BOLD)} {flow_file}")

        with open(flow_file, 'r', encoding='utf-8') as f:
            flow_data = json.load(f)

        all_pass = True

        # 查找当前模块在流向表中的定义
        module_info = next((m for m in flow_data.get('modules', [])
                           if m['name'] == self.module_name), None)
        if not module_info:
            self.warnings.append((str(flow_file),
                f"模块 '{self.module_name}' 未在数据流向表中定义", 0))
            return True

        # 检查输入源对应的子结构体是否存在
        io_file = self._find_io_file()
        if io_file:
            with open(io_file, 'r', encoding='utf-8', errors='ignore') as f:
                io_content = f.read()

            for inp in module_info.get('inputs', []):
                src_cap = inp['source'].replace('_', '').capitalize()
                expected_type = f"{self.module_cap}_From{src_cap}_t"
                if expected_type not in io_content:
                    self.violations.append((str(io_file),
                        f"缺少输入子结构体 {expected_type}（来自 {inp['source']}）", 0))
                    all_pass = False

        print(ok(f"数据流向表一致性检查通过"))
        return all_pass

    def check(self) -> bool:
        """执行所有检查"""
        print(f"\n{color('='*50, BOLD)}")
        print(f"{color('I/O 接口规范检查', BOLD)}")
        print(f"{color('='*50, BOLD)}")
        print(f"{color('模块:', BOLD)} {self.module_name} ({self.module_cap})")
        print(f"{color('项目:', BOLD)} {self.project_root}")

        # 查找文件
        io_file = self._find_io_file()
        c_file = self._find_c_file()
        switcher_file = self._find_switcher_file()

        if not io_file:
            print(error(f"未找到 {self.module_name}_io.h 或 {self.module_cap}_io.h"))
            print(info("请确认 I/O 头文件存在，或使用 --project 指定项目路径"))
            return False

        all_pass = True

        # 执行检查
        all_pass &= self.check_io_header(io_file)
        all_pass &= self.check_flow_consistency()

        if c_file:
            all_pass &= self.check_c_file(c_file)

        if switcher_file:
            all_pass &= self.check_switcher(switcher_file)

        # 输出结果
        print(f"\n{color('='*50, BOLD)}")
        if self.violations:
            print(f"\n{color('违规项:', RED)} {len(self.violations)}")
            for file, msg, line in self.violations:
                print(f"  {error(msg)}")
                if line > 0:
                    print(f"    {file}:{line}")

        if self.warnings:
            print(f"\n{color('警告项:', YELLOW)} {len(self.warnings)}")
            for file, msg, line in self.warnings:
                print(f"  {warn(msg)}")
                if line > 0:
                    print(f"    {file}:{line}")

        print(f"\n{color('='*50, BOLD)}")
        if all_pass and not self.violations:
            print(color("Result: CLEAN", GREEN))
            return True
        else:
            print(color(f"Result: {len(self.violations)} violations — FIX REQUIRED", RED))
            return False

    @staticmethod
    def check_all(project_root: Optional[str] = None) -> bool:
        """检查所有模块"""
        project_path = Path(project_root) if project_root else Path.cwd()
        flow_file = project_path / 'data_flow_table.json'

        if not flow_file.exists():
            print(error(f"数据流向表不存在: {flow_file}"))
            return False

        with open(flow_file, 'r', encoding='utf-8') as f:
            flow_data = json.load(f)

        print(f"\n{color('='*60, BOLD)}")
        print(f"{color('批量检查所有模块', BOLD)}")
        print(f"{color('='*60, BOLD)}")
        print(f"{color('项目:', BOLD)} {project_path}")

        all_clean = True
        for module in flow_data.get('modules', []):
            checker = IOConventionChecker(module['name'], project_root)
            if not checker.check():
                all_clean = False

        print(f"\n{color('='*60, BOLD)}")
        if all_clean:
            print(color("ALL MODULES: CLEAN", GREEN))
        else:
            print(color("SOME MODULES HAVE VIOLATIONS", RED))

        return all_clean

    @staticmethod
    def self_test() -> bool:
        """自检测试"""
        import tempfile
        import shutil

        print(f"\n{color('='*50, BOLD)}")
        print(f"{color('SELF-TEST', BOLD)}")
        print(f"{color('='*50, BOLD)}")

        # 创建临时测试目录
        test_dir = Path(tempfile.mkdtemp())
        include_dir = test_dir / 'include'
        include_dir.mkdir()

        passed = 0
        failed = 0

        # 测试 1: 合法的 I/O 头文件
        good_io = include_dir / 'app_power_io.h'
        good_io.write_text('''
#ifndef APP_POWER_IO_H
#define APP_POWER_IO_H

#include <stdint.h>
#include "std_module.h"

typedef struct {
    uint16_t current;
    uint16_t voltage;
} AppPower_FromAdc_t;

typedef struct {
    uint8_t power_on;
} AppPower_FromComm_t;

typedef struct {
    AppPower_FromAdc_t fromAdc;
    AppPower_FromComm_t fromComm;
} AppPower_Input_t;

typedef struct {
    uint8_t  has_output;
    uint8_t  res[3];
    int16_t  ppg_delta;
} AppPower_Output_t;

MODULE_IO_H(AppPower);

#endif
''')

        checker = IOConventionChecker('app_power', test_dir)
        if checker.check():
            print(ok("测试 1: 合法的 I/O 头文件 — PASS"))
            passed += 1
        else:
            print(error("测试 1: 合法的 I/O 头文件 — FAIL"))
            failed += 1

        # 测试 2: 包含禁止字段的 I/O 头文件
        bad_io = include_dir / 'bad_module_io.h'
        bad_io.write_text('''
#ifndef BAD_MODULE_IO_H
#define BAD_MODULE_IO_H

#include <stdint.h>

typedef struct {
    uint8_t status;     /* 禁止: 框架字段 */
    uint8_t res[3];     /* 禁止: 框架字段 */
    uint16_t value;
} BadModule_Input_t;

typedef struct {
    uint8_t has_output;
} BadModule_Output_t;

#endif
''')

        checker2 = IOConventionChecker('bad_module', test_dir)
        checker2.check_io_header(bad_io)
        if checker2.violations:
            print(ok("测试 2: 检测到禁止字段 — PASS"))
            passed += 1
        else:
            print(error("测试 2: 检测到禁止字段 — FAIL"))
            failed += 1

        # 测试 3: 类型命名不规范
        bad_io2 = include_dir / 'wrong_name_io.h'
        bad_io2.write_text('''
#ifndef WRONG_NAME_IO_H
#define WRONG_NAME_IO_H

#include <stdint.h>

typedef struct {
    uint16_t value;
} WrongNameInputData;     /* 错误: 应为 WrongName_Input_t */

typedef struct {
    uint8_t flag;
} WrongNameOutputData;    /* 错误: 应为 WrongName_Output_t */

#endif
''')

        checker3 = IOConventionChecker('wrong_name', test_dir)
        checker3.check_io_header(bad_io2)
        if checker3.violations:
            print(ok("测试 3: 检测到类型命名不规范 — PASS"))
            passed += 1
        else:
            print(error("测试 3: 检测到类型命名不规范 — FAIL"))
            failed += 1

        # 清理
        shutil.rmtree(test_dir)

        print(f"\n{color('='*50, BOLD)}")
        print(f"SELF-TEST: {passed}/{passed+failed} PASSED")
        if failed == 0:
            print(color("Result: ALL PASSED", GREEN))
            return True
        else:
            print(color(f"Result: {failed} FAILED", RED))
            return False


def main():
    import argparse

    parser = argparse.ArgumentParser(
        description='I/O 接口规范检查工具',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog='''
Examples:
  python check_io_convention.py app_power
  python check_io_convention.py app_power --project ../four_head
  python check_io_convention.py --all
  python check_io_convention.py --self-test
'''
    )

    parser.add_argument('module', nargs='?', help='模块名称 (snake_case)')
    parser.add_argument('--project', '-p', help='项目根目录路径')
    parser.add_argument('--all', action='store_true', help='检查所有模块')
    parser.add_argument('--self-test', action='store_true', help='运行自检测试')

    args = parser.parse_args()

    if args.self_test:
        success = IOConventionChecker.self_test()
        sys.exit(0 if success else 1)

    if args.all:
        success = IOConventionChecker.check_all(args.project)
        sys.exit(0 if success else 1)

    if not args.module:
        parser.print_help()
        sys.exit(1)

    checker = IOConventionChecker(args.module, args.project)
    success = checker.check()
    sys.exit(0 if success else 1)


if __name__ == '__main__':
    main()
    int16_t  ppg_delta;
} AppPower_Output_t;

MODULE_IO_H(AppPower);

#endif
''')

        checker = IOConventionChecker('app_power', test_dir)
        if checker.check():
            print(ok("测试 1: 合法的 I/O 头文件 — PASS"))
            passed += 1
        else:
            print(error("测试 1: 合法的 I/O 头文件 — FAIL"))
            failed += 1

        # 测试 2: 包含禁止字段的 I/O 头文件
        bad_io = include_dir / 'bad_module_io.h'
        bad_io.write_text('''
#ifndef BAD_MODULE_IO_H
#define BAD_MODULE_IO_H

#include <stdint.h>

typedef struct {
    uint8_t status;     /* 禁止: 框架字段 */
    uint8_t res[3];     /* 禁止: 框架字段 */
    uint16_t value;
} BadModule_Input_t;

typedef struct {
    uint8_t has_output;
} BadModule_Output_t;

#endif
''')

        checker2 = IOConventionChecker('bad_module', test_dir)
        checker2.check_io_header(bad_io)
        if checker2.violations:
            print(ok("测试 2: 检测到禁止字段 — PASS"))
            passed += 1
        else:
            print(error("测试 2: 检测到禁止字段 — FAIL"))
            failed += 1

        # 测试 3: 类型命名不规范
        bad_io2 = include_dir / 'wrong_name_io.h'
        bad_io2.write_text('''
#ifndef WRONG_NAME_IO_H
#define WRONG_NAME_IO_H

#include <stdint.h>

typedef struct {
    uint16_t value;
} WrongNameInputData;     /* 错误: 应为 WrongName_Input_t */

typedef struct {
    uint8_t flag;
} WrongNameOutputData;    /* 错误: 应为 WrongName_Output_t */

#endif
''')

        checker3 = IOConventionChecker('wrong_name', test_dir)
        checker3.check_io_header(bad_io2)
        if checker3.violations:
            print(ok("测试 3: 检测到类型命名不规范 — PASS"))
            passed += 1
        else:
            print(error("测试 3: 检测到类型命名不规范 — FAIL"))
            failed += 1

        # 清理
        shutil.rmtree(test_dir)

        print(f"\n{color('='*50, BOLD)}")
        print(f"SELF-TEST: {passed}/{passed+failed} PASSED")
        if failed == 0:
            print(color("Result: ALL PASSED", GREEN))
            return True
        else:
            print(color(f"Result: {failed} FAILED", RED))
            return False


def main():
    import argparse

    parser = argparse.ArgumentParser(
        description='I/O 接口规范检查工具',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog='''
Examples:
  python check_io_convention.py app_power
  python check_io_convention.py app_power --project ../four_head
  python check_io_convention.py --self-test
'''
    )

    parser.add_argument('module', nargs='?', help='模块名称 (snake_case)')
    parser.add_argument('--project', '-p', help='项目根目录路径')
    parser.add_argument('--self-test', action='store_true', help='运行自检测试')

    args = parser.parse_args()

    if args.self_test:
        success = IOConventionChecker.self_test()
        sys.exit(0 if success else 1)

    if not args.module:
        parser.print_help()
        sys.exit(1)

    checker = IOConventionChecker(args.module, args.project)
    success = checker.check()
    sys.exit(0 if success else 1)


if __name__ == '__main__':
    main()
