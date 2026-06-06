#!/usr/bin/env python3
"""
check_all.py — four_head 一键合规审计
在 pre-commit hook 中自动执行，阻断违规提交。

检查项:
  1. check_deps.py    — 层依赖审计
  2. check_msgs.py    — 消息通道一致性
"""
import subprocess
import sys
import os

ROOT = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(ROOT, "..", "src")

def run(script, *args):
    cmd = [sys.executable, os.path.join(ROOT, script)] + list(args)
    r = subprocess.run(cmd, cwd=ROOT)
    return r.returncode

def main():
    fails = 0

    print("")
    print("========================================")
    print("  four_head — 合规审计")
    print("========================================")
    print("")

    print("[1/2] check_deps.py ... ", end="", flush=True)
    if run("check_deps.py", SRC) == 0:
        print("PASS")
    else:
        print("FAIL")
        fails += 1

    print("[2/2] check_msgs.py ... ", end="", flush=True)
    if run("check_msgs.py", SRC) == 0:
        print("PASS")
    else:
        print("FAIL")
        fails += 1

    print("")
    if fails:
        print(f"结果: {fails} 项失败 — 提交阻断")
        sys.exit(1)
    else:
        print("结果: 全部通过")
        sys.exit(0)

if __name__ == "__main__":
    main()
