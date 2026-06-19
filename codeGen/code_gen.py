#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
code_gen.py — v2.3 LINK+PARAMS 框架代码自动生成器 主入口

双模式:
  --init   简化流向表 → AI 填充字段 → 输出完整 JSON
  --gen    完整 JSON → 生成 io.h + data_switcher.c + module.c

输出目录结构 (由 project.paths 控制):
  {output_root}/
    include/    ← io.h 文件
    core/       ← data_switcher.c
    app/        ← app 层模块 .c
    base_class/ ← base_class 层模块 .c
    proto/      ← proto 层模块 .c

使用:
  python code_gen.py --init flow.json --output project.json
  python code_gen.py --gen project.json --output ./out
"""

import argparse
import json
import os
import re
import shutil
import sys
from gen_io_h import generate_io_h
from gen_switcher import generate_switcher
from gen_module_c import generate_module_c, generate_module_h


def _pascal_to_snake(name: str) -> str:
    """PascalCase → snake_case: AppAdc → app_adc"""
    s1 = re.sub(r'([A-Z]+)([A-Z][a-z])', r'\1_\2', name)
    return re.sub(r'([a-z0-9])([A-Z])', r'\1_\2', s1).lower()


def load_json(path: str) -> dict:
    """加载 JSON 文件"""
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


def _backup(path: str):
    """若文件已存在，备份为 path.bak（覆盖已有 .bak）"""
    if os.path.isfile(path):
        shutil.copy2(path, path + ".bak")
        print(f"  [BACKUP] {path}.bak")


def save_json(path: str, data: dict):
    """保存 JSON 文件（先备份）"""
    os.makedirs(os.path.dirname(path) or ".", exist_ok=True)
    _backup(path)
    with open(path, "w", encoding="utf-8") as f:
        json.dump(data, f, ensure_ascii=False, indent=2)
    print(f"  [SAVE] {path}")


def save_file(path: str, content: str):
    """保存生成文件（先备份）"""
    os.makedirs(os.path.dirname(path) or ".", exist_ok=True)
    _backup(path)
    with open(path, "w", encoding="utf-8") as f:
        f.write(content)
    print(f"  [GEN] {path}")


def cmd_init(args):
    """--init 模式: 简化流向表 → 完整 JSON"""
    flow = load_json(args.config)
    print(f"初始化项目: {flow.get('project', {}).get('name', 'Unnamed')}")

    # 确保有 slot_order (若未填则按 modules 顺序)
    if "slot_order" not in flow:
        flow["slot_order"] = [m["name"] for m in flow.get("modules", [])]

    # 为没有 fields 的 pipe 添加 stub 字段
    for pipe in flow.get("pipes", []):
        if "fields" not in pipe:
            pipe["fields"] = []
        if "out_link" not in pipe:
            pipe["out_link"] = {"style": "array", "array_size": 4}
        if pipe["out_link"]["style"] == "array" and "array_size" not in pipe["out_link"]:
            pipe["out_link"]["array_size"] = 4

    # 输出
    save_json(args.output, flow)
    print("完成! 建议字段已填充，可通过 GUI 编辑后重新生成。")


def cmd_gen(args):
    """--gen 模式: 完整 JSON → 生成代码"""
    config = load_json(args.config)
    output_root = args.output
    project = config.get("project", {})
    paths = project.get("paths", {})
    modules = config.get("modules", [])
    pipes = config.get("pipes", [])
    slot_order = config.get("slot_order", [m["name"] for m in modules])
    slot_chains = config.get("slot_chains", [])

    io_dir    = paths.get("io_dir", "include")
    core_dir  = paths.get("core_dir", "core")
    app_dir   = paths.get("app_dir", "app")
    drv_dir   = paths.get("drv_dir", "drv")
    base_dir  = paths.get("base_class_dir", "base_class")
    proto_dir = paths.get("proto_dir", "proto")

    # 确定某模块的输出目录
    def _mod_output_dir(module: dict) -> str:
        layer = module.get("layer", "app")
        if layer == "app":
            return os.path.join(output_root, app_dir)
        elif layer == "drv":
            return os.path.join(output_root, drv_dir)
        elif layer == "base_class":
            return os.path.join(output_root, base_dir)
        elif layer == "proto":
            return os.path.join(output_root, proto_dir)
        else:
            return os.path.join(output_root, layer)

    print(f"生成代码: {project.get('name', 'Unnamed')}")
    print(f"  输出根目录: {output_root}")

    # ---- 1. 生成 io.h ----
    if not args.only_switcher and not args.only_modules:
        print("\n--- io.h ---")
        io_out = os.path.join(output_root, io_dir)
        for mod in modules:
            content = generate_io_h(mod, pipes, project)
            fname = f"{_pascal_to_snake(mod['name'])}_io.h"
            save_file(os.path.join(io_out, fname), content)

    # ---- 2. 生成 data_switcher.c ----
    if not args.only_io and not args.only_modules:
        print("\n--- data_switcher.c ---")
        core_out = os.path.join(output_root, core_dir)
        content = generate_switcher(modules, pipes, slot_order, project, slot_chains)
        save_file(os.path.join(core_out, "data_switcher.c"), content)

    # ---- 3. 生成模块 .c + .h ----
    if not args.only_io and not args.only_switcher:
        print("\n--- 模块 .c / .h ---")
        for mod in modules:
            mod_out = _mod_output_dir(mod)
            c_path = os.path.join(mod_out, f"{_pascal_to_snake(mod['name'])}.c")

            # 生成 .c (检测现有文件，保留用户区)
            content_c = generate_module_c(mod, pipes, project, c_path)
            save_file(c_path, content_c)

            # 生成 .h
            content_h = generate_module_h(mod)
            save_file(os.path.join(mod_out, f"{_pascal_to_snake(mod['name'])}.h"), content_h)

    print(f"\n[DONE] 生成完成!")


def main():
    parser = argparse.ArgumentParser(description="v2.3 LINK+PARAMS 框架代码生成器")
    sub = parser.add_subparsers(dest="mode", help="操作模式")

    # --init 模式
    p_init = sub.add_parser("init", help="简化流向表 → 完整 JSON")
    p_init.add_argument("--config", "-c", required=True, help="流向表 JSON")
    p_init.add_argument("--output", "-o", default="project.json", help="输出 JSON 路径")

    # --gen 模式
    p_gen = sub.add_parser("gen", help="完整 JSON → 生成代码")
    p_gen.add_argument("--config", "-c", required=True, help="项目 JSON 配置 (JSON/project.json)")
    p_gen.add_argument("--output", "-o", default=".", help="输出根目录 (项目 src/ 根, io.h 写入 include_io/)")
    p_gen.add_argument("--only-io", action="store_true", help="仅生成 io.h")
    p_gen.add_argument("--only-switcher", action="store_true", help="仅生成 data_switcher.c")
    p_gen.add_argument("--only-modules", action="store_true", help="仅生成模块 .c/.h")

    args = parser.parse_args()
    if args.mode == "init":
        cmd_init(args)
    elif args.mode == "gen":
        cmd_gen(args)
    else:
        parser.print_help()


if __name__ == "__main__":
    main()
