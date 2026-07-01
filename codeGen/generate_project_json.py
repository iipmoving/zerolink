#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
generate_project_json.py — 通过 Claude Code CLI 自动生成 project.json

用法:
    python codeGen/generate_project_json.py <项目路径> [项目名称]

功能:
    1. 自动解析 KEIL 项目文件，提取源文件列表
    2. 调用 Claude Code CLI，传入 /gen-json skill 指令
    3. 在新窗口中运行 Claude（保持 TTY 交互）
    4. 轮询检查 project.json 是否生成

依赖:
    - Claude Code CLI 已安装并可用
    - 目标项目目录包含 .uvprojx 文件
"""

import os
import sys
import shutil
import subprocess
import json
import time


def find_keil_project(proj_dir):
    """在项目目录中查找 .uvprojx 文件"""
    uvprojx_files = []
    for root, dirs, files in os.walk(proj_dir):
        for f in files:
            if f.endswith(".uvprojx"):
                uvprojx_files.append(os.path.join(root, f))
    return uvprojx_files


def get_project_name(proj_dir):
    """从目录名推断项目名称"""
    return os.path.basename(os.path.abspath(proj_dir))


def _build_prompt(project_name, abs_proj_dir, keil_path):
    """构建发送给 Claude 的 prompt"""
    return f"""/gen-json {project_name}

请按照 gen-json SKILL 的要求，扫描当前项目并生成 project.json。

项目根目录: {abs_proj_dir}
KEIL 项目文件: {keil_path}

注意:
- output_root 必须使用绝对路径: {abs_proj_dir}
- 最终 JSON 写入 codeGen/out/{project_name}/project.json
- 严格遵循 SKILL 的铁律: 未读完所有源文件前不得写入 JSON
- 生成后用 python -c 验证 JSON 语法
- 完成后报告: 模块数、管道数、嵌套 struct 处理情况
"""


def _find_claude_exe():
    """查找 claude.exe 的实际路径"""
    # 优先使用已知的 Windows npm 全局路径
    candidates = [
        r'C:\Users\moving\AppData\Roaming\npm\node_modules\@anthropic-ai\claude-code\bin\claude.exe',
    ]
    for c in candidates:
        if os.path.exists(c):
            return c
    # 其次尝试 which
    which_path = shutil.which("claude")
    if which_path:
        return which_path
    return "claude"


def generate_project_json(proj_dir, project_name=None, wait=True, poll_interval=5, keil_dir=None, output_root_override=None):
    """调用 Claude Code CLI 生成 project.json

    Args:
        proj_dir: 项目根目录（用于找 .uvprojx）
        project_name: 项目名称
        wait: CLI 模式下是否等待生成完成（GUI 模式下设为 False）
        poll_interval: 轮询间隔（秒）
        keil_dir: KEIL 项目文件所在目录（用于精确找 .uvprojx），不传则从 proj_dir 递归查找
        output_root_override: 输出目录覆盖（project.json 写入此位置，不传则用 proj_dir）

    Returns:
        project.json 的绝对路径，或 None
    """

    if project_name is None:
        project_name = get_project_name(proj_dir)

    # 使用覆盖的输出目录，或默认用 proj_dir
    effective_output = output_root_override if output_root_override else proj_dir

    # 1. 查找 KEIL 项目文件
    search_dir = keil_dir if keil_dir else proj_dir
    keil_files = find_keil_project(search_dir)
    if not keil_files:
        print(f"[ERROR] 在 {search_dir} 中未找到 .uvprojx 文件")
        print("请先在 GUI 中解析 KEIL 项目")
        return None

    keil_path = keil_files[0]
    print(f"[INFO] 找到 KEIL 项目: {keil_path}")

    abs_proj_dir = os.path.abspath(proj_dir)
    abs_output_dir = os.path.abspath(effective_output)
    prompt = _build_prompt(project_name, abs_proj_dir, keil_path)
    out_dir = os.path.join(abs_output_dir, "codeGen", "out", project_name)
    json_path = os.path.join(out_dir, "project.json")

    print(f"[DEBUG] abs_proj_dir: {abs_proj_dir}")
    print(f"[DEBUG] abs_output_dir: {abs_output_dir}")

    try:
        claude_exe = _find_claude_exe()
        creationflags = subprocess.CREATE_NEW_CONSOLE if os.name == 'nt' else 0

        if os.path.exists(claude_exe) and claude_exe.endswith('.exe'):
            # 直接调用 .exe，新窗口
            proc = subprocess.Popen(
                [claude_exe, "-p", prompt],
                cwd=abs_proj_dir,
                creationflags=creationflags,
            )
        else:
            # .CMD/.BAT 包装文件，需要 cmd /c
            cmd_line = f'"{claude_exe}" -p "{prompt}"'
            proc = subprocess.Popen(
                ["cmd", "/c", cmd_line],
                cwd=abs_proj_dir,
                creationflags=creationflags,
            )

        print(f"\n[INFO] Claude 进程已启动 (PID: {proc.pid})")
        print(f"[INFO] 项目目录: {abs_proj_dir}")
        print(f"[INFO] 项目名称: {project_name}")
        print(f"[INFO] Claude 将在弹出的新窗口中运行，请耐心等待 (通常 1-3 分钟)...")
        print()

        if not wait:
            # GUI 模式: 只启动进程，不等待
            return json_path

        # CLI 模式: 轮询检查 project.json
        print(f"[INFO] 正在等待生成完成 (每 {poll_interval}s 检查一次)...")
        elapsed = 0
        max_wait = 600  # 10 分钟

        while elapsed < max_wait:
            if os.path.exists(json_path):
                try:
                    with open(json_path, "r", encoding="utf-8") as f:
                        data = json.load(f)
                    print(f"\n[OK] project.json 已生成: {json_path}")
                    print(f"[OK] JSON 语法验证通过")
                    print(f"     模块数: {len(data.get('modules', []))}")
                    print(f"     管道数: {len(data.get('pipes', []))}")
                    print(f"     slot_order: {data.get('slot_order', [])}")
                    return json_path
                except json.JSONDecodeError as e:
                    print(f"\n[WARN] project.json 存在但 JSON 格式错误: {e}")
                    return None

            time.sleep(poll_interval)
            elapsed += poll_interval
            if elapsed % 30 == 0:
                print(f"  ... 已等待 {elapsed}s / {max_wait}s")

        print(f"\n[ERROR] 等待超时 ({max_wait}s)")
        print(f"[HINT] Claude 可能仍在运行。请检查弹出的窗口。")
        return None

    except FileNotFoundError:
        print("[ERROR] 未找到 claude 命令，请确认 Claude Code CLI 已安装")
        print("       运行: npm install -g @anthropic-ai/claude")
        return None
    except Exception as e:
        print(f"[ERROR] 生成失败: {e}")
        return None


def main():
    if len(sys.argv) < 2:
        print("用法: python generate_project_json.py <项目路径> [项目名称]")
        print()
        print("示例:")
        print("  python generate_project_json.py ../m4_ekf_observer")
        print("  python generate_project_json.py ../m4_ekf_observer m4_ekf")
        sys.exit(1)

    proj_dir = sys.argv[1]
    project_name = sys.argv[2] if len(sys.argv) > 2 else None

    if not os.path.isdir(proj_dir):
        print(f"[ERROR] 目录不存在: {proj_dir}")
        sys.exit(1)

    json_path = generate_project_json(proj_dir, project_name)

    if json_path:
        print(f"\n[SUCCESS] project.json 已生成: {json_path}")
        sys.exit(0)
    else:
        print(f"\n[FAILED] 未能生成 project.json")
        sys.exit(1)


if __name__ == "__main__":
    main()
