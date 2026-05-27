#!/usr/bin/env python3
"""
check_weak_pairs.py — __weak 回调配对一致性检查

扫描 interface_map.h 中记录的配对，验证:
  1. 发送方文件存在对应的 __weak 声明
  2. 接收方文件存在对应的强符号实现
  3. 函数签名一致（参数类型/顺序/返回值）
  4. 孤儿 __weak: 存在 __weak 声明但 interface_map.h 无记录
  5. 孤儿强符号: 存在符合命名约定的强符号但无 __weak 发送方

用法: python check_weak_pairs.py <source_dir> [--interface-map <path>]
退出码: 0=通过, 1=发现违规

interface_map.h 中的配对格式 (注释块):
    /* Pair {ID}: {desc}
     * 发送方: {file}.c  WEAK {return_type} {func_name}({params}) {}
     * 接收方: {file}.c  {return_type} {func_name}({params})
     */

如果 interface_map.h 不存在或无法解析，工具只做孤儿扫描 (模式 B)。
"""

import sys
import os
import re
from pathlib import Path
from collections import defaultdict


# ============================================================
# 接口: 从 interface_map.h 解析配对
# ============================================================

PAIR_START_RE = re.compile(r'/[*]\s*Pair\s+(\S+)\s*:\s*(.+?)\s*[*]/')
SENDER_RE = re.compile(
    r'发送方\s*:\s*(\S+\.c)\s+WEAK\s+'
    r'(\w+(?:\s*\*)?)\s+'           # return type
    r'(\w+)\s*[(]'                    # function name
    r'([^)]*)[)]\s*[{]'               # params
)
RECEIVER_RE = re.compile(
    r'接收方\s*:\s*(\S+\.c)\s+'
    r'(\w+(?:\s*\*)?)\s+'            # return type
    r'(\w+)\s*[(]'                     # function name
    r'([^)]*)[)]'                      # params
)
NOTE_RE = re.compile(r'注\s*:\s*(.+?)\s*[*]/')


def parse_interface_map(filepath):
    """从 interface_map.h 提取所有配对记录。
    返回: [(pair_id, description, sender_file, return_type, func_name, params, receiver_file, return_type_r, func_name_r, params_r, note), ...]
    """
    pairs = []
    if not os.path.exists(filepath):
        return pairs

    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()

    lines = content.split('\n')

    i = 0
    while i < len(lines):
        line = lines[i].strip()
        pm = PAIR_START_RE.match(line)
        if not pm:
            i += 1
            continue

        pair_id = pm.group(1)
        description = pm.group(2)

        sender_info = None
        receiver_info = None
        note = None

        # 读取后续注释行
        j = i + 1
        while j < len(lines) and j < i + 10:
            cline = lines[j].strip()
            if not cline.startswith('*') and not cline.startswith('/*'):
                break

            sm = SENDER_RE.search(cline)
            if sm:
                sender_info = (sm.group(1), sm.group(2).strip(),
                               sm.group(3), sm.group(4))

            rm = RECEIVER_RE.search(cline)
            if rm:
                receiver_info = (rm.group(1), rm.group(2).strip(),
                                 rm.group(3), rm.group(4))

            nm = NOTE_RE.search(cline)
            if nm:
                note = nm.group(1).strip()

            j += 1

        if sender_info and receiver_info:
            pairs.append((pair_id, description,
                          sender_info[0], sender_info[1], sender_info[2], sender_info[3],
                          receiver_info[0], receiver_info[1], receiver_info[2], receiver_info[3],
                          note))

        i = j

    return pairs


# ============================================================
# 函数签名解析
# ============================================================

# Matches WEAK declarations in .h: WEAK void FuncName(params);
WEAK_DECL_H_RE = re.compile(
    r'(?:WEAK|__weak)\s+'              # WEAK macro or __weak keyword
    r'(\w+(?:\s*\*)?)\s+'              # return type
    r'(\w+)\s*[(]'                      # function name
    r'([^)]*)[)]'                       # params
)

# Matches strong symbol definitions in .c: void FuncName(params) {
# Must NOT be preceded by WEAK/__weak, and must be a definition (brace follows)
STRONG_SYMBOL_DEF_RE = re.compile(
    r'^(?!.*(?:WEAK|__weak|extern|typedef))\s*'
    r'(?:inline\s+)?(?:static\s+)?'
    r'(\w+(?:\s*\*)?)\s+'              # return type
    r'(\w+)\s*[(]'                      # function name
    r'([^)]*)[)]'                       # params
)

# Public API entry points (called from main.c, not __weak pairs):
# Match: {Module}_OnTick*, engine_*, _Inject, _Init (public module API)
PUBLIC_API_RE = re.compile(
    r'^(?:engine_|'
    r'.*_OnTick|.*_OnSecondTick|.*_OnInit|'
    r'.*_Inject'
    r')'
)

# Pattern for callbacks that SHOULD be in __weak pairs:
# {Prefix}_On{Event} where Event is NOT a public API entry
CALLBACK_NAME_RE = re.compile(r'^(\w+)_On(\w+)$')


def normalize_params(params_str):
    """规范化参数字符串用于比较: 去除空格差异、void 统一处理"""
    s = params_str.strip()
    if s == 'void' or s == '':
        return 'void'
    # 去除多余空格
    s = re.sub(r'\s+', ' ', s)
    # 去除参数名，只保留类型 (approximate: 去掉最后一个空格后的标识符)
    # 这不够精确，但对于 C 的简单类型基本够用
    parts = s.split(',')
    normalized = []
    for p in parts:
        p = p.strip()
        # 去掉最后的标识符参数名 (如果存在)
        # const Type *name → const Type *
        tokens = p.split()
        # 如果最后一个 token 看起来像参数名 (不以 * 结尾, 不是关键字)
        if tokens and not tokens[-1].endswith('*') and tokens[-1] not in \
                ('const', 'volatile', 'void', 'struct', 'union', 'enum', 'unsigned', 'signed'):
            # 可能是参数名，去掉
            tokens = tokens[:-1]
        normalized.append(' '.join(tokens))
    return ', '.join(normalized).strip()


def find_function_in_file(filepath, func_name, is_weak):
    """搜索指定函数 (多行感知)。

    is_weak=True:  搜索 WEAK 声明
    is_weak=False: 搜索强符号定义
    """
    if not os.path.exists(filepath):
        return None

    if is_weak:
        base = os.path.splitext(filepath)[0]
        search_files = [filepath]
        h_path = base + '.h'
        if os.path.exists(h_path):
            search_files.insert(0, h_path)

        for sf in search_files:
            result = _find_weak_in_content(sf, func_name)
            if result:
                return result
        return None
    else:
        # 强符号: 按全文件内容搜索 (支持多行)
        return _find_strong_in_content(filepath, func_name)


def _find_strong_in_content(filepath, func_name):
    """搜索强符号定义 (多行感知)。"""
    if not os.path.exists(filepath):
        return None

    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()

    # 搜索 func_name( — 排除前面有 WEAK/__weak/extern 的行
    # 允许跨多行的参数列表 (匹配到第一个完整的 {)
    pattern = re.compile(
        r'(?<!\bWEAK\s)(?<!\b__weak\s)(?<!\bextern\s)'
        r'(?:inline\s+)?(?:static\s+)?'
        r'(\w+(?:\s*\*)?)\s+'                    # return type
        + re.escape(func_name) +
        r'\s*[(]'                                 # opening paren
        r'([^)]*(?:[(][^)]*[)][^)]*)*)'          # params (supports nested parens)
        r'[)]'                                    # closing paren
    )

    m = pattern.search(content)
    if m:
        lineno = content[:m.start()].count('\n') + 1
        return (m.group(1).strip(), m.group(2), lineno)
    return None


def _find_weak_in_content(filepath, func_name):
    """在文件内容中搜索 WEAK 声明 (多行感知, 从上下文中提取完整参数列表)。

    读取整个文件, 搜索函数名 + 参数列表, 验证前面有 WEAK 关键字。
    """
    if not os.path.exists(filepath):
        return None

    # 跳过 interface_map.h (纯文档, 不是实际声明)
    if os.path.basename(filepath) == 'interface_map.h':
        return None

    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()

    # 搜索模式: WEAK ... FuncName(...)
    # 允许 WEAK 和 FuncName 之间有换行和任意内容
    # 先找 WEAK 单词, 然后在后续内容中找 func_name
    weak_positions = [m.end() for m in re.finditer(r'\b(?:WEAK|__weak)\b', content)]

    for wpos in weak_positions:
        # 从 WEAK 之后 500 字符内搜索 func_name
        window = content[wpos:wpos + 500]
        fn_pattern = re.compile(
            r'(\w+(?:\s*\*)?)\s+'            # return type
            + re.escape(func_name) +
            r'\s*[(]([^)]*)[)]'              # params (single-line only, 近似)
        )
        m = fn_pattern.search(window)
        if m:
            lineno = content[:wpos].count('\n') + 1
            return (m.group(1).strip(), m.group(2), lineno)

    return None


def find_weak_call_in_file(filepath, func_name):
    """检查文件中是否有对此函数的调用 (作为弱符号发送方)。"""
    if not os.path.exists(filepath):
        return False

    call_pattern = re.compile(r'\b' + re.escape(func_name) + r'\s*[(]')
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        for line in f:
            # 排除函数定义本身和 __weak 声明
            if '__weak' in line or 'WEAK' in line:
                continue
            if call_pattern.search(line):
                return True
    return False


# ============================================================
# 签名比较
# ============================================================

def compare_signatures(sender_sig, receiver_sig):
    """比较发送方和接收方的函数签名。
    sender_sig: (return_type, params_str)
    receiver_sig: (return_type, params_str)
    返回: (is_match, diff_description)
    """
    s_ret, s_params = sender_sig
    r_ret, r_params = receiver_sig

    diffs = []

    # 比较返回类型
    if s_ret != r_ret:
        diffs.append(f"返回类型: '{s_ret}' vs '{r_ret}'")

    # 比较参数
    s_norm = normalize_params(s_params)
    r_norm = normalize_params(r_params)
    if s_norm != r_norm:
        diffs.append(f"参数: '{s_params}' vs '{r_params}'")

    if diffs:
        return False, '; '.join(diffs)
    return True, None


# ============================================================
# 模式 A: 从 interface_map.h 验证
# ============================================================

def verify_from_interface_map(pairs, source_dir):
    """根据 interface_map.h 中的记录验证代码。"""
    violations = []

    for (pair_id, desc,
         s_file, s_ret, s_name, s_params,
         r_file, r_ret, r_name, r_params,
         note) in pairs:

        # 检查发送方文件
        s_path = find_file(source_dir, s_file)
        if s_path is None:
            violations.append({
                'pair': pair_id,
                'type': 'sender_file_missing',
                'desc': f"Pair {pair_id} ({desc}): 发送方文件 '{s_file}' 不存在",
            })
            continue

        s_func = find_function_in_file(s_path, s_name, is_weak=True)
        if s_func is None:
            violations.append({
                'pair': pair_id,
                'type': 'sender_weak_missing',
                'desc': f"Pair {pair_id} ({desc}): {s_file} 中未找到 __weak {s_name}() 声明",
                'file': s_path,
            })
            continue

        s_actual_ret, s_actual_params, s_line = s_func

        # 比较发送方签名
        ok, diff = compare_signatures((s_ret, s_params), (s_actual_ret, s_actual_params))
        if not ok:
            violations.append({
                'pair': pair_id,
                'type': 'sender_signature_mismatch',
                'desc': f"Pair {pair_id} ({desc}): {s_file}:{s_line} __weak {s_name}() 签名不匹配 — {diff}",
                'file': s_path,
                'line': s_line,
            })

        # 检查接收方文件
        r_path = find_file(source_dir, r_file)
        if r_path is None:
            violations.append({
                'pair': pair_id,
                'type': 'receiver_file_missing',
                'desc': f"Pair {pair_id} ({desc}): 接收方文件 '{r_file}' 不存在",
            })
            continue

        r_func = find_function_in_file(r_path, r_name, is_weak=False)
        if r_func is None:
            violations.append({
                'pair': pair_id,
                'type': 'receiver_strong_missing',
                'desc': f"Pair {pair_id} ({desc}): {r_file} 中未找到强符号 {r_name}() 实现",
                'file': r_path,
            })
            continue

        r_actual_ret, r_actual_params, r_line = r_func

        # 比较接收方签名
        ok, diff = compare_signatures((r_ret, r_params), (r_actual_ret, r_actual_params))
        if not ok:
            violations.append({
                'pair': pair_id,
                'type': 'receiver_signature_mismatch',
                'desc': f"Pair {pair_id} ({desc}): {r_file}:{r_line} {r_name}() 签名不匹配 — {diff}",
                'file': r_path,
                'line': r_line,
            })

        # 比较发送方和接收方的签名是否一致 (interface_map.h 中记录的)
        ok, diff = compare_signatures((s_ret, s_params), (r_ret, r_params))
        if not ok:
            violations.append({
                'pair': pair_id,
                'type': 'cross_signature_mismatch',
                'desc': f"Pair {pair_id} ({desc}): 收发双方签名不匹配 — {diff}",
            })

    return violations


# ============================================================
# 模式 B: 孤儿扫描
# ============================================================

def scan_orphans(source_dir, known_pairs):
    """扫描所有 .c 文件中的 __weak 声明和强符号回调，检测孤儿。"""
    violations = []

    # 收集已知配对中的函数名 (用函数名匹配, 因为声明可能在 .h 而记录是 .c)
    known_sender_funcs = set()   # just func_name
    known_receiver_funcs = set() # just func_name
    for p in known_pairs:
        known_sender_funcs.add(p[4])   # sender_func_name
        known_receiver_funcs.add(p[8])  # receiver_func_name

    # 扫描所有 .c 文件
    all_c_files = []
    for root, dirs, files in os.walk(source_dir):
        for f in files:
            if f.endswith('.c'):
                all_c_files.append(os.path.join(root, f))

    # 收集所有 __weak 声明 (来自 .h 和 .c 文件)
    found_weaks = []  # (file, func_name, return_type, params, line)
    found_callbacks = []  # (file, func_name, return_type, params, line)

    all_files_to_scan = list(all_c_files)
    # 也扫描 .h 文件中的 WEAK 声明
    for root, dirs, files in os.walk(source_dir):
        for f in files:
            if f.endswith('.h'):
                all_files_to_scan.append(os.path.join(root, f))

    for cfile in all_files_to_scan:
        # 跳过 interface_map.h (纯文档, 不参与编译)
        if os.path.basename(cfile) == 'interface_map.h':
            continue

        try:
            with open(cfile, 'r', encoding='utf-8', errors='ignore') as f:
                for lineno, line in enumerate(f, 1):
                    wm = WEAK_DECL_H_RE.search(line)
                    if wm:
                        fname = wm.group(2)
                        if '_On' in fname:
                            found_weaks.append(
                                (os.path.basename(cfile), fname,
                                 wm.group(1), wm.group(3), lineno, cfile))

                    # 检查 .c 中的强符号回调定义
                    if cfile.endswith('.c'):
                        sm = STRONG_SYMBOL_DEF_RE.match(line.strip())
                        if sm:
                            fname = sm.group(2)
                            if CALLBACK_NAME_RE.match(fname):
                                # 排除 public API (main.c 直接调用的入口)
                                if not PUBLIC_API_RE.search(fname):
                                    # 排除已知的非回调 _On 函数
                                    if 'WEAK' not in line and '__weak' not in line:
                                        found_callbacks.append(
                                            (os.path.basename(cfile), fname,
                                             sm.group(1), sm.group(3), lineno, cfile))
        except Exception:
            continue

    # 检测孤儿 __weak
    for wf, wn, wr, wp, wl, wpath in found_weaks:
        if wn not in known_sender_funcs:
            violations.append({
                'pair': 'ORPHAN',
                'type': 'orphan_weak',
                'desc': f"孤儿 __weak: {wf}:{wl} WEAK {wn}() — interface_map.h 中未记录此发送方",
                'file': wpath,
                'line': wl,
            })

    # 检测孤儿强符号 (有 OnXxx 实现但无对应 __weak 声明)
    for cf, cn, cr, cp, cl, cpath in found_callbacks:
        if cn not in known_receiver_funcs:
            has_sender = any(wf[1] == cn for wf in found_weaks)
            if not has_sender:
                violations.append({
                    'pair': 'ORPHAN',
                    'type': 'orphan_strong',
                    'desc': f"孤儿强符号: {cf}:{cl} {cn}() — 无对应 __weak 声明且 interface_map.h 中未记录",
                    'file': cpath,
                    'line': cl,
                })

    return violations


# ============================================================
# 辅助函数
# ============================================================

def find_file(source_dir, filename):
    """在 source_dir 下搜索文件名。"""
    for root, dirs, files in os.walk(source_dir):
        if filename in files:
            return os.path.join(root, filename)
        # 也检查 basename 匹配
        for f in files:
            if f == filename or f == os.path.basename(filename):
                return os.path.join(root, f)
    return None


# ============================================================
# 主逻辑
# ============================================================

def main():
    if len(sys.argv) < 2:
        print("用法: python check_weak_pairs.py <source_directory> [--interface-map <path>]")
        print("示例: python check_weak_pairs.py ../Claude")
        print("      python check_weak_pairs.py ../Claude --interface-map core/interface_map.h")
        sys.exit(2)

    source_dir = os.path.abspath(sys.argv[1])
    if not os.path.isdir(source_dir):
        print(f"错误: 目录不存在: {source_dir}")
        sys.exit(2)

    # 查找 interface_map.h
    imap_path = None
    if '--interface-map' in sys.argv:
        idx = sys.argv.index('--interface-map')
        imap_path = sys.argv[idx + 1]
    else:
        # 自动搜索
        for root, dirs, files in os.walk(source_dir):
            if 'interface_map.h' in files:
                imap_path = os.path.join(root, 'interface_map.h')
                break

    print(f"check_weak_pairs.py — __weak 回调配对一致性检查")
    print(f"扫描目录: {source_dir}")

    violations = []

    if imap_path and os.path.exists(imap_path):
        print(f"接口映射表: {imap_path}")
        print()

        pairs = parse_interface_map(imap_path)
        if pairs:
            print(f"从 interface_map.h 解析到 {len(pairs)} 对 __weak 配对")
            for p in pairs:
                print(f"  Pair {p[0]}: {p[1]} — {p[4]}() [{p[2]} → {p[6]}]")

            v1 = verify_from_interface_map(pairs, source_dir)
            violations.extend(v1)
        else:
            print("  (未解析到任何配对，仅做孤儿扫描)")
            pairs = []

        v2 = scan_orphans(source_dir, pairs)
        violations.extend(v2)
    else:
        print("  (未找到 interface_map.h，仅做孤儿扫描)")
        print()
        violations = scan_orphans(source_dir, [])

    print()

    if not violations:
        print("[PASS] 全部通过 — __weak 配对一致，0 违规")
        sys.exit(0)

    # 按类型分组输出
    by_type = defaultdict(list)
    for v in violations:
        by_type[v['type']].append(v)

    print(f"[FAIL] 发现 {len(violations)} 项违规:")
    print()

    type_labels = {
        'sender_file_missing': '发送方文件不存在',
        'sender_weak_missing': '发送方缺少 __weak 声明',
        'sender_signature_mismatch': '发送方签名不匹配',
        'receiver_file_missing': '接收方文件不存在',
        'receiver_strong_missing': '接收方缺少强符号实现',
        'receiver_signature_mismatch': '接收方签名不匹配',
        'cross_signature_mismatch': '收发双方签名不一致',
        'orphan_weak': '孤儿 __weak 声明 (interface_map.h 未记录)',
        'orphan_strong': '孤儿强符号 (无对应 __weak)',
    }

    for vtype, label in type_labels.items():
        if vtype in by_type:
            print(f"  [{label}] — {len(by_type[vtype])} 项:")
            for v in by_type[vtype]:
                print(f"    {v['desc']}")
            print()

    print("修复指引:")
    print("  → 新增配对: 在 interface_map.h 中注册，发送方加 __weak 空壳，接收方加强实现")
    print("  → 签名不匹配: 检查 interface_map.h 记录与实际代码的参数类型/顺序")
    print("  → 孤儿: 要么补充 interface_map.h 记录，要么删除无用的声明/实现")
    print()
    sys.exit(1)


if __name__ == '__main__':
    main()
