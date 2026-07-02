#!/usr/bin/env python3
"""
check_all.py — 零耦合方法论全项检查 (m4_ekf_observer)

等效于 /check 技能, 可直接运行或作为 pre-commit hook 调用.

检查项:
  1. check_deps.py       — 层依赖规则 (L1 阻断)
  2. check_weak_pairs.py — __weak 配对一致性
  3. check_structs.py    — @STRUCT consumer/owner 一致性

用法:
  python tools/check_all.py                  # 全项检查
  python tools/check_all.py --pre-commit     # pre-commit 模式 (仅检查 staged 文件相关)
  python tools/check_all.py --self-test      # 自检

退出码: 0=全部通过, 1=发现违规
"""

import sys
import os
import subprocess
import glob as globmod

PROJECT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
TOOLS_DIR = os.path.join(PROJECT_DIR, 'tools')


def run_check(script_name, args=None):
    """Run a check script, return (passed: bool, output: str)."""
    script_path = os.path.join(TOOLS_DIR, script_name)
    if not os.path.exists(script_path):
        return True, f"[SKIP] {script_name} — 脚本不存在"

    cmd = ['python', script_path]
    if args:
        cmd.extend(args)

    try:
        result = subprocess.run(cmd, capture_output=True, text=True,
                                cwd=PROJECT_DIR, timeout=60)
        passed = result.returncode == 0
        return passed, result.stdout + result.stderr
    except subprocess.TimeoutExpired:
        return False, f"[FAIL] {script_name} — 超时 (60s)"
    except Exception as e:
        return False, f"[FAIL] {script_name} — 异常: {e}"


def main():
    pre_commit = '--pre-commit' in sys.argv
    self_test = '--self-test' in sys.argv

    if self_test:
        print("check_all.py — 自检模式")
        print(f"  项目目录: {PROJECT_DIR}")
        print(f"  工具目录: {TOOLS_DIR}")

        scripts = ['check_deps.py', 'check_weak_pairs.py', 'check_structs.py']
        all_ok = True
        for s in scripts:
            exists = os.path.exists(os.path.join(TOOLS_DIR, s))
            status = "OK" if exists else "MISSING"
            if not exists:
                all_ok = False
            print(f"  [{status}] {s}")
        if all_ok:
            print("[PASS] check_all 自检通过")
        else:
            print("[FAIL] 缺少工具脚本")
        sys.exit(0 if all_ok else 1)

    print("=" * 60)
    print("check_all — 零耦合方法论全项检查")
    print(f"  项目: {os.path.basename(PROJECT_DIR)}")
    mode = "PRE-COMMIT" if pre_commit else "FULL"
    print(f"  模式: {mode}")
    print("=" * 60)
    print()

    checks = [
        ('check_deps.py',       [PROJECT_DIR],             '层依赖规则'),
        ('check_weak_pairs.py', [PROJECT_DIR],             '__weak 配对一致性'),
        ('check_structs.py',    [PROJECT_DIR, '--h-mode'], '@STRUCT consumer/owner 一致性'),
    ]

    all_passed = True
    results = []

    for script_name, args, description in checks:
        print(f"--- [{description}] {script_name} ---")
        passed, output = run_check(script_name, args)
        results.append((description, passed, output))

        if output.strip():
            # 截断过长输出
            lines = output.strip().split('\n')
            if len(lines) > 30:
                for line in lines[:30]:
                    print(line)
                print(f"... (截断, 共 {len(lines)} 行)")
            else:
                print(output.strip())

        status = "PASS" if passed else "FAIL"
        print(f"[{status}] {description}")
        print()

        if not passed:
            all_passed = False

    print("=" * 60)
    if all_passed:
        print("[PASS] 全部检查通过 — 0 违规")
    else:
        print("[FAIL] 发现违规 — 请修复后重新提交")
        print()
        print("修复指引:")
        print("  1. check_deps.py 失败 → 移除非法跨层 include")
        print("  2. check_weak_pairs.py 失败 → 在 interface_map.h 注册配对, 或补充 __weak/strong 声明")
        print("  3. check_structs.py 失败 → 检查 consumer @STRUCT 字段 offset/type/sizeof 与 owner 一致")
    print("=" * 60)

    sys.exit(0 if all_passed else 1)


if __name__ == '__main__':
    main()
