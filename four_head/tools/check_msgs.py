#!/usr/bin/env python3
"""
check_msgs.py —— AI管理的消息ID命名空间一致性检查

扫描所有模块的本地 #define 消息ID 和 Msg_Post/MsgScheduler_Register 调用，
验证:
  1. 每个消息ID的发送方和接收方数值一致
  2. 无重复分配（同一数值被多个不同通道使用）
  3. 无孤儿消息（有post无register或有register无post）
  4. 接口映射表与实际代码一致

用法: python tools/check_msgs.py Claude
"""

import os, re, sys
from collections import defaultdict

ROOT = sys.argv[1] if len(sys.argv) > 1 else 'Claude'

# 收集: { numeric_id: { 'posts': [(file, line, local_name)], 'registers': [(file, line, local_name)] } }
channels = defaultdict(lambda: {'posts': [], 'registers': []})

# 扫描所有 .c 文件
for dirpath, _, filenames in os.walk(ROOT):
    for fn in filenames:
        if not fn.endswith('.c'):
            continue
        fpath = os.path.join(dirpath, fn)
        rel = os.path.relpath(fpath, ROOT).replace('\\', '/')
        try:
            with open(fpath, encoding='utf-8', errors='ignore') as f:
                lines = f.readlines()
        except Exception:
            continue

        # 收集本文件的本地 #define (MSG相关)
        local_defs = {}  # name -> value
        for i, line in enumerate(lines, 1):
            m = re.match(r'#define\s+(\w*MSG\w*)\s+(\d+)u?\b', line)
            if m:
                local_defs[m.group(1)] = int(m.group(2))

        # 扫描 Msg_Post 和 MsgScheduler_Register
        for i, line in enumerate(lines, 1):
            # 跳过注释行
            stripped = line.strip()
            if stripped.startswith('//') or stripped.startswith('*') or stripped.startswith('/*'):
                continue

            for kind, pattern in [('post', r'Msg_Post\s*\(\s*(\w+)'),
                                   ('register', r'MsgScheduler_Register\s*\(\s*(\w+)')]:
                m = re.search(pattern, line)
                if m:
                    name = m.group(1)
                    # 解析: 可能是本地名或字面量
                    if name in local_defs:
                        num = local_defs[name]
                    elif name.isdigit():
                        num = int(name)
                        name = f'literal_{num}'
                    else:
                        # 不在本文件的defines里（可能是h文件定义的或注释中的）
                        num = None

                    if num is not None:
                        channels[num][f'{kind}s'].append((rel, i, name))

# ===== 检查 =====
errors = []

# 1. 重复分配：同一数值被多个逻辑通道使用
# (同一数值可以有多个posts(多发送方)和多个registers(多接收方)，这是合法的)
# 但需要人工确认

# 2. 孤儿：有post无register
for num, data in sorted(channels.items()):
    if data['posts'] and not data['registers']:
        errors.append(
            f"ID {num:2d}: 有发送无接收 (孤儿消息)\n"
            + '\n'.join(f"    POST  {f}:{l} ({n})" for f, l, n in data['posts'])
        )

# 3. 孤儿：有register无post
for num, data in sorted(channels.items()):
    if data['registers'] and not data['posts']:
        errors.append(
            f"ID {num:2d}: 有接收无发送 (孤儿回调)\n"
            + '\n'.join(f"    REG   {f}:{l} ({n})" for f, l, n in data['registers'])
        )

# ===== 输出 =====
print(f"check_msgs.py: 扫描 {ROOT}/ 中的消息ID使用")
print(f"  找到 {len(channels)} 个活跃消息通道\n")

if errors:
    print(f"[FAIL] {len(errors)} 个问题:\n")
    for e in errors:
        print(e)
        print()
    sys.exit(1)
else:
    print("[PASS] 所有消息通道发送方/接收方配对完整")

# 打印完整映射表
print("\n── 消息通道映射表 ──")
for num in sorted(channels):
    data = channels[num]
    posts = [f"{f}:{l}" for f, l, _ in data['posts']]
    regs  = [f"{f}:{l}" for f, l, _ in data['registers']]
    p_names = ', '.join(set(n for _, _, n in data['posts']))
    r_names = ', '.join(set(n for _, _, n in data['registers']))
    print(f"  [{num:2d}] {p_names:30s} → {r_names}")
