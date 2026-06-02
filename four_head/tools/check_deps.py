#!/usr/bin/env python3
"""
check_deps.py — 层依赖规则合规检查

扫描 Claude/ 下所有 .c 和 .h 文件的 #include 语句，
验证是否符合模块分层架构的依赖规则。

铁律:
  APP:  不得 include drv/ 或 hal/
  DRV:  不得 include app/ (cfg/ 例外: 纯数据描述)
  HAL:  零业务依赖 (不得 include core/ app/ drv/ proto/ cfg/)
  PROTO: 不得 include app/ drv/ hal/
  CORE:  不得 include app/ drv/ hal/ proto/ cfg/

已知例外:
  - cfg/hmi_data.h 是纯数据描述，可供 APP 和 DRV 共同 include
  - HAL 可以 include lib/ (第三方触摸/显示库) 和 MCU 固件库 (sc32f1xxx_*)

用法: python check_deps.py <Claude_dir>
退出码: 0=通过, 1=发现违规
"""

import sys
import os
import re
from pathlib import Path

# ============================================================
# 层定义
# ============================================================
# 每个层: (allowed_patterns, forbidden_patterns)
# allowed 中的路径前缀表示可以 include
# forbidden 中的路径前缀表示禁止 include
# 路径中的 ../ 会被规范化去除

RULES = {
    'app': {
        'allowed': [
            'app/',        # 自己的模块头文件
            'core/',       # 消息调度器 + msg_def
            'proto/',      # MODBUS 协议共享类型
            'cfg/',        # hmi_data 纯数据 (例外)
            '<',           # 标准库 <stdint.h> 等
        ],
        'forbidden': [
            'drv/',        # 驱动层
            'hal/',        # 硬件抽象层
        ],
    },
    'drv': {
        'allowed': [
            'drv/',        # 自己的模块头文件
            'core/',       # 消息调度器
            'hal/',        # 硬件抽象 (DRV 唯一可调 HAL)
            'cfg/',        # hmi_data 纯数据 (例外)
            '<',           # 标准库
        ],
        'forbidden': [
            'app/',        # 业务层
            'proto/',      # 协议层
        ],
    },
    'hal': {
        'allowed': [
            'hal/',        # 自己的模块头文件
            '<',           # 标准库
            # MCU 固件库
            'sc32f1xxx_',
            'sc32L14xx',
            'system_',
            # 第三方库 (触摸/显示)
            'lib/',
            'SMG_Disp_',
            'SC_TK_Scan',
            'TKDriver',
        ],
        'forbidden': [
            'core/',       # 消息总线
            'app/',        # 业务层
            'drv/',        # 驱动层
            'proto/',      # 协议层
            'cfg/',        # 配置数据
        ],
    },
    'proto': {
        'allowed': [
            'proto/',      # 自己的模块头文件
            'core/',       # msg_def.h 共享类型
            '<',           # 标准库
        ],
        'forbidden': [
            'app/',
            'drv/',
            'hal/',
            'cfg/',
        ],
    },
    'core': {
        'allowed': [
            'core/',       # 自己的模块头文件
            '<',           # 标准库
        ],
        'forbidden': [
            'app/',
            'drv/',
            'hal/',
            'proto/',
            'cfg/',
        ],
    },
    'cfg': {
        # cfg 是特殊层：纯数据描述，可被 APP/DRV include
        # cfg 自身的 .c/.h 只能依赖 app/ (类型定义) 和自身
        'allowed': [
            'cfg/',
            'app/',        # HmiHead_t 等类型定义
            '<',
        ],
        'forbidden': [
            'drv/',
            'hal/',
            'proto/',
            'core/',
        ],
    },
}

# ============================================================
# 文件分类
# ============================================================
def classify_file(filepath):
    """根据路径判断文件属于哪个层"""
    parts = filepath.replace('\\', '/').split('/')
    # 在路径中查找层目录
    for layer in ['app', 'drv', 'hal', 'proto', 'core', 'cfg']:
        if layer in parts:
            # 确保是顶层目录 (Claude/app/... 而不是 Claude/lib/app/...)
            idx = parts.index(layer)
            if idx > 0 and parts[idx - 1] == 'Claude':
                return layer
            if idx == 0:
                return layer
    return None

# ============================================================
# Include 解析
# ============================================================
INCLUDE_RE = re.compile(r'^\s*#include\s+[<"]([^>"]+)[>"]')

def parse_includes(filepath):
    """解析文件中的所有 #include 路径"""
    includes = []
    try:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            for line in f:
                m = INCLUDE_RE.match(line)
                if m:
                    includes.append(m.group(1))
    except Exception as e:
        print(f"  [WARN] 无法读取 {filepath}: {e}")
    return includes

def normalize_include(inc_path):
    """规范化 include 路径: 去除 ../ 前缀"""
    # 将 ../dir/file.h 转为 dir/file.h
    while inc_path.startswith('../'):
        inc_path = inc_path[3:]
    # 将 ./ 去除
    if inc_path.startswith('./'):
        inc_path = inc_path[2:]
    return inc_path

# ============================================================
# 规则检查
# ============================================================
def is_system_header(inc_path):
    """判断是否为系统/标准库头文件"""
    # 尖括号包含的
    if inc_path.startswith('<'):
        return True
    # 双引号包含的标准库头文件 (stdint.h, string.h, stddef.h, stdarg.h, stdbool.h 等)
    name = inc_path.strip('"')
    system_headers = {
        'stdint.h', 'stddef.h', 'stdarg.h', 'stdbool.h', 'string.h',
        'stdio.h', 'stdlib.h', 'math.h', 'assert.h', 'limits.h',
        'ctype.h', 'errno.h', 'float.h', 'inttypes.h',
    }
    return name in system_headers

def is_same_dir_include(inc_path, filepath, layer):
    """判断是否为同目录 include (例如 app_hmi.c include "app_hmi.h")"""
    norm = normalize_include(inc_path)
    # 不含 / 的路径 = 同目录
    if '/' not in norm:
        return True
    # 以 ../ 开头且解析后在同一层目录下
    file_dir = os.path.dirname(filepath.replace('\\', '/'))
    inc_dir = os.path.dirname(norm)
    if inc_dir == '':
        return True
    # 规范化后检查
    return False

def check_include(inc_path, layer, filepath):
    """检查单个 include 是否合规。
    返回 (is_ok, violation_desc)"""
    norm = normalize_include(inc_path)
    rule = RULES[layer]

    # 系统头文件 — 所有层都允许
    if is_system_header(inc_path):
        return True, None

    # 同目录 include (不含路径分隔符) — 始终允许
    if is_same_dir_include(inc_path, filepath, layer):
        return True, None

    # 检查 forbidden 模式
    for pat in rule['forbidden']:
        if norm.startswith(pat):
            return False, f"禁止 include '{pat}' 层: {inc_path}"

    # 检查 allowed 模式
    for pat in rule['allowed']:
        if norm.startswith(pat):
            return True, None

    # 不在 allowed 列表中，视为违规
    return False, f"不在 {layer} 层允许列表中: {inc_path}"

# ============================================================
# extern 交叉引用检查 (.c 文件绝对禁止 extern)
# ============================================================
EXTERN_RE = re.compile(r'^\s*extern\s+')

def check_extern(filepath, relpath):
    """检查 .c 文件是否包含 extern 声明 (跨模块引用).
    返回 violation list."""
    violations = []
    try:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            for lineno, line in enumerate(f, 1):
                # 跳过注释行
                stripped = line.strip()
                if stripped.startswith('//') or stripped.startswith('/*'):
                    continue
                if EXTERN_RE.match(line):
                    violations.append({
                        'file': relpath,
                        'line': lineno,
                        'text': line.strip(),
                        'desc': '.c 文件禁止 extern — 跨模块引用破坏零耦合, 改用 __weak 回调',
                    })
    except Exception as e:
        pass
    return violations

# ============================================================
# 主逻辑
# ============================================================
def scan_directory(claude_dir):
    """扫描 Claude 目录下所有源文件"""
    violations = []
    extern_violations = []
    file_count = 0

    # 要扫描的目录
    scan_dirs = ['app', 'drv', 'hal', 'proto', 'core', 'cfg', 'src']

    for scan_dir in scan_dirs:
        dir_path = os.path.join(claude_dir, scan_dir)
        if not os.path.isdir(dir_path):
            continue

        for root, dirs, files in os.walk(dir_path):
            for fname in files:
                if not (fname.endswith('.c') or fname.endswith('.h')):
                    continue

                filepath = os.path.join(root, fname)
                relpath = os.path.relpath(filepath, claude_dir).replace('\\', '/')

                layer = classify_file(relpath)
                if layer is None:
                    continue  # 跳过 lib/ 等外部文件

                file_count += 1

                # extern 检查 (仅 .c 文件)
                if fname.endswith('.c'):
                    extern_violations.extend(check_extern(filepath, relpath))
                includes = parse_includes(filepath)

                for inc in includes:
                    ok, desc = check_include(inc, layer, relpath)
                    if not ok:
                        violations.append({
                            'file': relpath,
                            'layer': layer,
                            'include': inc,
                            'desc': desc,
                        })

    return file_count, violations, extern_violations

def main():
    if len(sys.argv) < 2:
        print("用法: python check_deps.py <Claude_directory>")
        print("示例: python check_deps.py ../Claude")
        sys.exit(2)

    claude_dir = os.path.abspath(sys.argv[1])
    if not os.path.isdir(claude_dir):
        print(f"错误: 目录不存在: {claude_dir}")
        sys.exit(2)

    print(f"check_deps.py — 层依赖规则检查")
    print(f"扫描目录: {claude_dir}")
    print()

    file_count, violations, extern_violations = scan_directory(claude_dir)
    total = len(violations) + len(extern_violations)

    if total == 0:
        print(f"[PASS] 全部通过 — 扫描 {file_count} 个文件, 0 违规")
        print("  层依赖 + extern 规则符合架构要求。")
        sys.exit(0)

    print(f"[FAIL] 发现 {total} 项违规 (共扫描 {file_count} 个文件):")
    print()

    # 按层分组输出 include 违规
    if violations:
        by_layer = {}
        for v in violations:
            layer = v['layer']
            if layer not in by_layer:
                by_layer[layer] = []
            by_layer[layer].append(v)

        for layer in ['app', 'drv', 'hal', 'proto', 'core', 'cfg']:
            if layer not in by_layer:
                continue
            print(f"  [{layer}/] — {len(by_layer[layer])} 项 include 违规:")
            for v in by_layer[layer]:
                print(f"    {v['file']}")
                print(f"      #include \"{v['include']}\"")
                print(f"      → {v['desc']}")
            print()

    # extern 违规
    if extern_violations:
        print(f"  [extern] — {len(extern_violations)} 项跨模块 extern 违规 (.c 文件绝对禁止):")
        for v in extern_violations:
            print(f"    {v['file']}:{v['line']}")
            print(f"      {v['text']}")
            print(f"      → {v['desc']}")
        print()

    print("修复指引:")
    print("  include → 移除跨层引用, 改用消息/回调")
    print("  extern  → 改用 __weak 函数 (消费者写 weak 默认, 生产者强覆盖)")
    print()
    sys.exit(1)

if __name__ == '__main__':
    main()
