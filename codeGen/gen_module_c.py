#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
gen_module_c.py — v2.3 模块 .c 骨架生成器

生成 {layer}/{module}.c + {layer}/{module}.h：
  - MODULE_SKELETON(Module)
  - Init(): g_input.para = &s_in, g_output.para = &s_out
  - ProcessInput(): 三段式骨架 → 调用户函数 Module_Process(in, out)
  - MODULE_EXPORT(Module)

用户区保护:
  重生成时读取现有 .c 文件，识别 === [USER CODE] === 标记，保留中间内容。
"""

import os
import re


# ========== 用户区标记 ==========
USER_CODE_BEGIN = "// ===== [USER CODE] ====="
USER_CODE_END   = "// ===== [END USER CODE] ====="


def _extract_user_code(filepath: str) -> str:
    """从现有文件提取用户区代码"""
    if not os.path.isfile(filepath):
        return ""
    try:
        with open(filepath, "r", encoding="utf-8") as f:
            content = f.read()
    except Exception:
        return ""

    start = content.find(USER_CODE_BEGIN)
    end   = content.find(USER_CODE_END)

    if start >= 0 and end > start:
        return content[start + len(USER_CODE_BEGIN):end].strip("\n")
    return ""


def generate_module_c(module: dict, pipes: list, project: dict, existing_file: str = "") -> str:
    """生成模块 .c 文件"""
    mod_name = module["name"]
    mod_lower = mod_name.lower()
    layer = module.get("layer", "app")
    comment = module.get("comment", "")

    # 本模块作为 Producer 的管道
    out_pipes = [p for p in pipes if p["from"] == mod_name]
    # 本模块作为 Consumer 的管道
    in_pipes  = [p for p in pipes if p["to"] == mod_name]

    io_dir = project.get("paths", {}).get("io_dir", "include")

    # 读取现有用户代码
    user_code = _extract_user_code(existing_file) if existing_file else ""

    lines = []
    lines.append("/**")
    lines.append(f" * @file    {mod_lower}.c")
    lines.append(f" * @brief   {mod_name} 模块实现 (v2.3 LINK+PARAMS)")
    lines.append(f" * @layer   {layer}")
    if comment:
        lines.append(" *")
        lines.append(f" * {comment}")
    lines.append(" */")
    lines.append("")

    # ====== 非用户区: includes + MODULE_SKELETON ======
    lines.append("// ===== [AI GENERATED] — 每次重生成 ===== ")
    lines.append(f'#include "{io_dir}/{mod_lower}_io.h"')
    lines.append(f'#include "{mod_lower}.h"')
    lines.append("")
    lines.append(f"MODULE_SKELETON({mod_name});")
    lines.append("")

    # 管道就绪标志位域 (每 BIT 对应一个上游管道)
    p_names = [p["from"] for p in in_pipes]
    if p_names:
        lines.append(f"/* 管道就绪标志: 每 BIT 代表一个管道的 ST_NEW 状态 */")
        lines.append(f"typedef union {{")
        lines.append(f"    uint8_t all;           /* 全零 = 无数据就绪，跳过本次 */")
        lines.append(f"    struct {{")
        for p in in_pipes:
            lines.append(f"        uint8_t {p['from'].lower()}   : 1;  /* {p['from']} 数据就绪 */")
        lines.append(f"    }} bits;")
        lines.append(f"}} {mod_name}_PipeFlags_t;")
        lines.append("")

    # ====== 用户区 ======
    lines.append(USER_CODE_BEGIN)
    lines.append("// >>> 用户常量、变量、业务函数")
    lines.append("// 此区域由用户维护，AI 重生成时保留")
    lines.append("")

    if user_code:
        lines.append(user_code)
    else:
        # 默认用户区内容
        p_names = [p["from"] for p in in_pipes]
        flag_params = f"{mod_name}_PipeFlags_t flags"

        if in_pipes and out_pipes:
            lines.append("// 用户业务函数 — by ProcessInput 调用")
            in_type = f"MODULE_INPUT({mod_name})"
            out_type = f"MODULE_OUTPUT({mod_name})"
            args = f"{in_type} *in, {out_type} *out"
            if flag_params:
                args += f", {flag_params}"
            lines.append(f"void {mod_name}_Process({args})")
            lines.append("{")
            for pn in p_names:
                lines.append(f"    if (flags.bits.{pn.lower()}) {{ /* 处理 {pn} 输入 */ }}")
            out_targets = [p["to"] for p in out_pipes]
            lines.append(f"    /* TODO: 写入 {', '.join(out_targets)} 输出 */")
            lines.append("}")
        elif in_pipes:
            args = f"MODULE_INPUT({mod_name}) *in"
            if flag_params:
                args += f", {flag_params}"
            lines.append(f"void {mod_name}_Process({args})")
            lines.append("{")
            for pn in p_names:
                lines.append(f"    if (flags.bits.{pn.lower()}) {{ /* 处理 {pn} 输入 */ }}")
            lines.append("}")
        elif out_pipes:
            lines.append(f"void {mod_name}_Process(MODULE_OUTPUT({mod_name}) *out)")
            lines.append("{")
            lines.append(f"    /* TODO: 产生输出数据 */")
            lines.append("}")
        else:
            lines.append(f"void {mod_name}_Process(void)")
            lines.append("{")
            lines.append("    /* TODO: 独立处理逻辑 */")
            lines.append("}")

        lines.append("")
        lines.append("// >>> 用户常量")
        lines.append(f"// #define {mod_name.upper()}_THRESHOLD  100")
        lines.append("")
        lines.append("// >>> 用户变量")
        lines.append(f"// static uint32_t s_{mod_lower}_counter;")

    lines.append("")
    lines.append(USER_CODE_END)
    lines.append("")

    # ====== AI 管控区: Init + ProcessInput + MODULE_EXPORT ======
    lines.append("// ===== [AI GENERATED] — 每次重生成 ===== ")
    lines.append("")

    # Init
    lines.append("static void Init(void)")
    lines.append("{")
    if in_pipes:
        lines.append(f"    g_input.para  = &s_in;")
    else:
        lines.append(f"    g_input.para  = NULL;  /* 无输入 */")
    if out_pipes:
        lines.append(f"    g_output.para = &s_out;")
    else:
        lines.append(f"    g_output.para = NULL;  /* 无输出 */")
    lines.append("")
    lines.append("    // >>> [USER: 添加初始化代码]")
    lines.append("}")
    lines.append("")

    # ProcessInput
    lines.append("static void ProcessInput(void)")
    lines.append("{")
    if in_pipes:
        lines.append(f"    MODULE_INPUT({mod_name}) *in = (MODULE_INPUT({mod_name})*)g_input.para;")
    if out_pipes:
        lines.append(f"    MODULE_OUTPUT({mod_name}) *out = (MODULE_OUTPUT({mod_name})*)g_output.para;")
    lines.append("")
    lines.append("    /* === 输入段: 数据有效检查 === */")
    if in_pipes:
        lines.append(f"    if (!in) return;")
        # 各管道独立就绪标志 (位域)
        p_names = [p["from"] for p in in_pipes]
        lines.append(f"    {mod_name}_PipeFlags_t flags = {{0}};")
        for pn in p_names:
            pn_lower = pn.lower()
            lines.append(f"    flags.bits.{pn_lower} = (in->{pn}_params.status & ST_NEW) ? 1 : 0;")
        # 只要有任一管道就绪就继续
        lines.append(f"    if (!flags.all) return;")
    else:
        lines.append("    // 无输入依赖，直接处理")

    lines.append("")
    lines.append("    /* === 计算段: 用户业务 === */")
    # 构建用户函数参数: in/out + 各管道标志
    proc_args = []
    if in_pipes:
        proc_args.append("in")
    if out_pipes:
        proc_args.append("out")
    if p_names:
        proc_args.append("flags")

    if proc_args:
        lines.append(f"    {mod_name}_Process({', '.join(proc_args)});")
    else:
        lines.append(f"    {mod_name}_Process();")

    lines.append("")
    lines.append("    /* === 输出段: 状态管理 === */")
    if in_pipes:
        for pipe in in_pipes:
            p_name = pipe["from"]
            lines.append(f"    in->{p_name}_params.status  &= ~ST_NEW;  /* 消费完成 */")
    if out_pipes:
        for pipe in out_pipes:
            c_name = pipe["to"]
            lines.append(f"    out->{c_name}_params.status |= ST_OUT;  /* 输出就绪 */")
    lines.append("}")
    lines.append("")

    # MODULE_EXPORT
    lines.append(f"MODULE_EXPORT({mod_name});")
    lines.append("")

    return "\n".join(lines)


def generate_module_h(module: dict) -> str:
    """生成模块 .h 文件"""
    mod_name = module["name"]
    layer = module.get("layer", "app")
    comment = module.get("comment", "")
    guard = f"{mod_name.upper()}_H"

    lines = []
    lines.append("/**")
    lines.append(f" * @file    {mod_name.lower()}.h")
    lines.append(f" * @brief   {mod_name} 模块公共接口")
    lines.append(f" * @layer   {layer}")
    if comment:
        lines.append(" *")
        lines.append(f" * {comment}")
    lines.append(" */")
    lines.append("")
    lines.append(f"#ifndef {guard}")

    # APP/PROTO 层注释 #define
    if layer in ("app", "proto"):
        lines.append(f"//#define {guard}   /* L0 阻断 */")
    else:
        lines.append(f"#define {guard}")

    lines.append("")
    lines.append('#include <stdint.h>')
    lines.append("")
    lines.append("/* ---- v2.3 统一接口声明 ---- */")
    lines.append(f"MODULE_IO_H({mod_name});")
    lines.append("")
    lines.append("#endif /* guard */")
    lines.append("")

    return "\n".join(lines)
