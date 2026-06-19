#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
gen_switcher.py — v2.3 data_switcher.c 生成器

根据 pipes[] 和 slot_order[] 生成 data_switcher.c：
  - #include 全部模块 io.h
  - SLOT 枚举 + SLOT_GETIO 注册
  - INPUT_CALLBACK (pull / edge / field_copy)
  - Switcher_Run 按 slot 顺序调用 DoWork
"""


import os
import re


def _pascal_to_snake(name: str) -> str:
    """PascalCase → snake_case: AppAdc → app_adc"""
    s1 = re.sub(r'([A-Z]+)([A-Z][a-z])', r'\1_\2', name)
    return re.sub(r'([a-z0-9])([A-Z])', r'\1_\2', s1).lower()


def _resolve_io_path(module_name: str, project: dict) -> str:
    """计算 io.h 在 data_switcher.c 中的 #include 路径

    根据 core_dir 和 io_dir 计算相对路径。
    文件名采用 snake_case: AppAdc → app_adc_io.h
    """
    core_dir = project.get("paths", {}).get("core_dir", "core")
    io_dir = project.get("paths", {}).get("io_dir", "include")
    output_root = project.get("output_root", ".")
    snake = _pascal_to_snake(module_name)

    abs_core = os.path.normpath(os.path.join(output_root, core_dir))
    abs_io = os.path.normpath(os.path.join(output_root, io_dir))
    rel = os.path.relpath(abs_io, abs_core).replace("\\", "/")
    return f"{rel}/{snake}_io.h"


def generate_switcher(modules: list, pipes: list, slot_order: list, project: dict, slot_chains: list = None) -> str:
    """生成 data_switcher.c"""
    lines = []

    # 文件头
    lines.append("/**")
    lines.append(" * @file    data_switcher.c")
    lines.append(" * @brief   Data Switcher — PULL 路由调度器 (v2.3 LINK+PARAMS)")
    lines.append(" * @layer   core")
    lines.append(" *")
    lines.append(" * ================================================================")
    lines.append(" * [AI GENERATED] 此文件由 codeGen 自动生成，请勿手动修改")
    lines.append(" * 修改方式: 编辑 project.json → 运行 GUI / code_gen.py 重新生成")
    lines.append(" * ================================================================")
    lines.append(" */")
    lines.append("")

    # Includes
    lines.append('#include "std_module.h"')
    lines.append('#include "data_switcher.h"')
    lines.append("")
    lines.append("/* IO 接口文件 — 全模块接入 */")
    for mod in modules:
        io_path = _resolve_io_path(mod["name"], project)
        lines.append(f'#include "{io_path}"')
    lines.append("")

    # 生成 SLOT 枚举
    lines.append("/* ===== 模块槽位索引 ===== */")
    lines.append("typedef enum {")
    for i, s in enumerate(slot_order):
        lines.append(f"    SLOT_{s} = {i},")
    lines.append("    SLOT_COUNT")
    lines.append("} SwitcherSlot_t;")
    lines.append("")

    # Slot 数组
    lines.append("static ModuleSlotDef s_slot[SLOT_COUNT];")
    lines.append("")

    # Switcher_Init
    lines.append("/* ================================================================")
    lines.append(" * Switcher_Init — 注册全部模块的 GetIO")
    lines.append(" * ================================================================ */")
    lines.append("void Switcher_Init(void)")
    lines.append("{")
    for s in slot_order:
        lines.append(f"    SLOT_GETIO({s});")
    lines.append("}")
    lines.append("")

    # InputCallbacks
    lines.append("/* ================================================================")
    lines.append(" * InputCallback 强符号实现 — 覆盖 MODULE_SKELETON 生成的 weak 空壳")
    lines.append(" * 数据流: Producer → Consumer (PULL)")
    lines.append(" * ================================================================ */")

    # 按 Consumer 分组 pipes，同一个 Consumer 的多个 InputCallback 合并
    consumer_pipes = {}
    for pipe in pipes:
        to_name = pipe["to"]
        if to_name not in consumer_pipes:
            consumer_pipes[to_name] = []
        consumer_pipes[to_name].append(pipe)

    for consumer_name, c_pipes in sorted(consumer_pipes.items()):
        lines.append("")
        lines.append(f"INPUT_CALLBACK({consumer_name})")
        lines.append("{")

        for pipe in c_pipes:
            from_name = pipe["from"]
            ptype = pipe.get("callback_type", "pull")

            if ptype == "pull":
                lines.append(f"    INPUT_GET_SLOT({from_name}, {consumer_name});")
            elif ptype == "edge":
                member = pipe.get("edge_member", "params")
                lines.append(f"    INPUT_EDGE_PULL({from_name}, {consumer_name}, {member});")
            elif ptype == "field_copy":
                # 生成逐字段搬运模板
                fields = pipe.get("fields", [])
                out_style = pipe.get("out_link", {}).get("style", "array")
                in_style = pipe.get("in_link", {}).get("style", pipe.get("out_link", {}).get("style", "array"))

                lines.append(f"    /* {from_name} → {consumer_name} field-by-field copy */")
                lines.append(f"    MODULE_OUTPUT({from_name}) *out = (MODULE_OUTPUT({from_name})*)s_slot[SLOT_{from_name}].pOut->para;")
                lines.append(f"    MODULE_INPUT({consumer_name}) *in = (MODULE_INPUT({consumer_name})*)s_slot[SLOT_{consumer_name}].pIn->para;")
                lines.append("")
                lines.append("    if (out && in) {")
                # 简单搬运: 遍历字段
                if fields:
                    for fi, f in enumerate(fields):
                        fname = f["name"]
                        lines.append(f"        in->{from_name}_params->params[0].{fname} = out->{consumer_name}_params->params[0].{fname};")
                    lines.append(f"        in->{from_name}_params->status |= ST_NEW;")
                else:
                    lines.append("        /* TODO: 填写逐字段搬运 */")
                lines.append("    }")

        lines.append("}")
        lines.append("")

    # Switcher_Slot_{Module} — 单模块独立执行
    lines.append("/* ================================================================")
    lines.append(" * Switcher_Slot_{Module} — 单模块独立执行")
    lines.append(" * ================================================================ */")
    lines.append("")
    for s in slot_order:
        lines.append(f"void Switcher_Slot_{s}(void) {{ s_slot[SLOT_{s}].pDoWork(); }}")
    lines.append("")

    # Switcher_Run_All — 批量执行
    lines.append("/* ================================================================")
    lines.append(" * Switcher_Run_All — 一次执行全部模块 (批量模式)")
    lines.append(" * ================================================================ */")
    lines.append("void Switcher_Run_All(void)")
    lines.append("{")
    for s in slot_order:
        lines.append(f"    Switcher_Slot_{s}();")
    lines.append("}")
    lines.append("")

    # ---- SLOT 调用链条 (来自 slot_chains 配置) ----
    if slot_chains:
        lines.append("/* ================================================================")
        lines.append(" * Switcher_Run_{name} — SLOT 调用链条 (由 project.json slot_chains 定义)")
        lines.append(" * ================================================================ */")
        lines.append("")
        for chain in slot_chains:
            cname = chain["name"]
            ccomment = chain.get("comment", "")
            cmodules = chain["modules"]
            lines.append(f"/* {cname} — {ccomment} */")
            lines.append(f"void Switcher_Run_{cname}(void)")
            lines.append("{")
            for m in cmodules:
                lines.append(f"    Switcher_Slot_{m}();")
            lines.append("}")
            lines.append("")

    return "\n".join(lines)
