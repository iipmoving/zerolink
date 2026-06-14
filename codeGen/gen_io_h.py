#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
gen_io_h.py — v2.3 LINK+PARAMS io.h 生成器

为每个模块生成 include/{Module}_io.h：
  - OUTPUT 部分: MODULE_OUTPUT_PARAMS/LINK (模块是 Producer)
  - INPUT 部分:  MODULE_INPUT_PARAMS/LINK  (模块是 Consumer)
  - MODULE_OUTPUT(Module): {Consumer}_params 成员
  - MODULE_INPUT(Module):  {Producer}_params 成员
  - MODULE_IO_H(Module) 末尾声明

管道自动两端对应:
  pipe From→To → 在 From_io.h 生成 OUTPUT, 在 To_io.h 生成 INPUT
"""

import os
import re


def _rel_include(io_dir: str, target_path: str, base_dir: str = None) -> str:
    """计算 io.h 中 #include 的相对路径

    两个路径都相对于 base_dir（默认为 CWD）。
    base_dir 通常是 output_root，保证路径解析准确。
    """
    io = io_dir.replace("\\", "/").strip("/")
    tgt = target_path.replace("\\", "/").strip("/")
    if base_dir:
        io = os.path.join(base_dir, io).replace("\\", "/")
        tgt = os.path.join(base_dir, tgt).replace("\\", "/")
    try:
        rel = os.path.relpath(tgt, io)
        return rel.replace("\\", "/")
    except ValueError:
        return f"../{tgt}"


def _link_style_to_c(style: str, array_size: int = 0) -> str:
    """LINK 样式 → C 声明"""
    if style == "array":
        return f"params[{array_size}]"
    elif style == "pointer_array":
        return f"*params[{array_size}]"
    else:  # pointer
        return "*params"


def _include_guard(module_name: str, layer: str, comment_guard: bool, is_io_h: bool = False) -> str:
    """生成 include guard（APP/PROTO 层注释掉 #define，仅限普通模块 .h）"""
    guard = f"{module_name.upper()}_IO_H"
    if is_io_h:
        return f"#ifndef {guard}\n#define {guard}\n"
    if layer in ("app", "proto") and comment_guard:
        return (f"#ifndef {guard}\n"
                f"//#define {guard}   /* L0 阻断: 禁用 include guard */\n")
    else:
        return f"#ifndef {guard}\n#define {guard}\n"


def _gen_params_fields(fields: list, indent: int = 4) -> str:
    """从 fields[] 生成 PARAMS 结构体字段（支持递归嵌套 struct）"""
    import re
    if not fields:
        return " " * indent + "uint8_t  res[4];       /* TODO: 填写数据字段 */"
    lines = []
    prefix = " " * indent
    for f in fields:
        raw_type = f.get('type', 'uint16_t')
        name = f.get('name', 'field')
        comment = f.get("comment", "")

        # 递归处理嵌套结构体
        if raw_type == "struct" and f.get("fields"):
            sub = _gen_params_fields(f["fields"], indent + 4)
            if comment:
                lines.append(f"{prefix}struct {{  /* {comment} */")
            else:
                lines.append(f"{prefix}struct {{")
            lines.append(sub)
            lines.append(f"{prefix}}} {name};")
        else:
            # 处理数组类型: "uint8_t[3]" → base="uint8_t", suffix="[3]"
            m = re.match(r'^(\w+(?:\s+\w+)?)\s*\[(\d+)\]$', raw_type)
            if m:
                base_type = m.group(1)
                arr_suffix = f"[{m.group(2)}]"
            else:
                base_type = raw_type
                arr_suffix = ""

            if comment:
                lines.append(f"{prefix}{base_type} {name}{arr_suffix};     /* {comment} */")
            else:
                lines.append(f"{prefix}{base_type} {name}{arr_suffix};")
    return "\n".join(lines)


def _pascal_to_snake(name: str) -> str:
    """PascalCase → snake_case: AppAdc → app_adc"""
    s1 = re.sub(r'([A-Z]+)([A-Z][a-z])', r'\1_\2', name)
    return re.sub(r'([a-z0-9])([A-Z])', r'\1_\2', s1).lower()


def generate_io_h(module: dict, pipes: list, project: dict) -> str:
    """生成模块的 io.h 内容"""
    mod_name = module["name"]
    snake_name = _pascal_to_snake(mod_name)
    layer = module.get("layer", "app")
    comment = module.get("comment", "")
    io_dir = project.get("paths", {}).get("io_dir", "include")
    std_module_path = project.get("std_module_path", "core/std_module.h")
    pack = project.get("pack", 0)
    comment_guard = project.get("comment_guard", True)

    # 本模块作为 Producer 的管道 (output)
    out_pipes = [p for p in pipes if p["from"] == mod_name]
    # 本模块作为 Consumer 的管道 (input)
    in_pipes  = [p for p in pipes if p["to"] == mod_name]

    # std_module.h 用裸文件名（编译器 -I 路径处理目录）
    std_rel = os.path.basename(std_module_path)

    # ====== 构建文件内容 ======
    lines = []
    lines.append("/**")
    lines.append(f" * @file    {snake_name}_io.h")
    lines.append(f" * @brief   {mod_name} Data Switcher IO interface (v2.3 LINK+PARAMS)")
    lines.append(f" * @layer   {layer}")
    if comment:
        lines.append(" *")
        lines.append(f" * {comment}")
    # 数据流文档
    if in_pipes:
        lines.append(" *")
        lines.append(" * 输入源:")
        for p in in_pipes:
            lines.append(f" *   {p['from']} → {mod_name}  ({p.get('comment', '')})")
    if out_pipes:
        lines.append(" *")
        lines.append(" * 输出目标:")
        for p in out_pipes:
            lines.append(f" *   {mod_name} → {p['to']}  ({p.get('comment', '')})")
    lines.append(" */")

    # Include guard
    guard_name = f"{mod_name.upper()}_IO_H"
    lines.append("")
    lines.append(_include_guard(mod_name, layer, comment_guard, is_io_h=True).rstrip())

    # Includes
    lines.append("")
    lines.append("#include <stdint.h>")
    lines.append(f'#include "{std_rel}"')

    # Pack
    if pack:
        lines.append("")
        lines.append(f"#pragma pack({pack})")

    # ========== 模块专属类型 typedef (独立于管道) ==========
    if module.get("types"):
        for t in module["types"]:
            t_name = t["name"]
            t_comment = t.get("comment", "")
            t_fields = t.get("fields", [])
            lines.append("")
            lines.append(f"/* ================================================================")
            lines.append(f" * {t_name} — {t_comment}")
            lines.append(f" * ================================================================ */")
            lines.append("")
            lines.append(f"typedef struct {{")
            lines.append(_gen_params_fields(t_fields))
            lines.append(f"}} {t_name};")

    # ========== OUTPUT 部分 (本模块是 Producer) ==========
    if out_pipes:
        lines.append("")
        lines.append("/* ================================================================")
        lines.append(" * OUTPUT — 本模块输出的数据管道")
        lines.append(" * ================================================================ */")
    else:
        lines.append("")
        lines.append("/* ========== OUTPUT (无) — 本模块没有输出 ========== */")

    for pipe in out_pipes:
        to_name = pipe["to"]
        fields = pipe.get("fields", [])
        out_style = pipe.get("out_link", {}).get("style", "array")
        out_size = pipe.get("out_link", {}).get("array_size", 4)

        lines.append("")
        lines.append(f"/* ------------------------------------------------------------------")
        lines.append(f" * {mod_name} → {to_name}  输出参数  ({pipe.get('comment', '')})")
        lines.append(f" * ------------------------------------------------------------------ */")

        # MODULE_OUTPUT_PARAMS
        lines.append(f"typedef struct {{")
        lines.append(_gen_params_fields(fields))
        lines.append(f"}} MODULE_OUTPUT_PARAMS({mod_name}, {to_name});")

        # MODULE_OUTPUT_LINK
        link_member = _link_style_to_c(out_style, out_size)
        lines.append("")
        lines.append(f"typedef struct {{")
        lines.append(f"    uint8_t  status;           /* ST_NEW / ST_OUT */")
        lines.append(f"    uint8_t  max_count;        /* 最大数量 */")
        lines.append(f"    uint8_t  count;            /* 当前周期索引 */")
        lines.append(f"    uint8_t  res[1];")
        lines.append(f"    MODULE_OUTPUT_PARAMS({mod_name}, {to_name}) {link_member};")
        lines.append(f"}} MODULE_OUTPUT_LINK({mod_name}, {to_name});")

    # MODULE_OUTPUT 聚合
    if out_pipes:
        lines.append("")
        lines.append(f"/* {mod_name}_Output — 输出聚合 (对称命名: 成员 = {{Consumer}}_params) */")
        lines.append(f"typedef struct {{")
        for pipe in out_pipes:
            c_name = pipe["to"]
            lines.append(f"    MODULE_OUTPUT_LINK({mod_name}, {c_name})  {c_name}_params;  /* → {c_name} */")
        lines.append(f"}} MODULE_OUTPUT({mod_name});")
    else:
        lines.append("")
        lines.append(f"/* {mod_name}_Output — 无输出 */")
        lines.append(f"typedef struct {{")
        lines.append(f"    uint8_t  res[4];")
        lines.append(f"}} MODULE_OUTPUT({mod_name});")

    # ========== INPUT 部分 (本模块是 Consumer) ==========
    if in_pipes:
        lines.append("")
        lines.append("/* ================================================================")
        lines.append(" * INPUT — 本模块输入的数据管道")
        lines.append(" * ================================================================ */")
    else:
        lines.append("")
        lines.append("/* ========== INPUT (无) — 本模块没有输入 ========== */")

    for pipe in in_pipes:
        from_name = pipe["from"]
        fields = pipe.get("fields", [])
        in_style = pipe.get("in_link", {}).get("style", pipe.get("out_link", {}).get("style", "array"))
        in_size = pipe.get("in_link", {}).get("array_size", pipe.get("out_link", {}).get("array_size", 4))

        # 对于 input，使用 in_link 配置；如果没填，用 out_link 配置（同一管道两端兼容）
        # 但如果 in_link 明确指定了 style，用 in_link；否则继承 out_link

        lines.append("")
        lines.append(f"/* ------------------------------------------------------------------")
        lines.append(f" * {from_name} → {mod_name}  输入参数  ({pipe.get('comment', '')})")
        lines.append(f" * ------------------------------------------------------------------ */")

        # MODULE_INPUT_PARAMS (布局与 OUTPUT_PARAMS 兼容)
        lines.append(f"typedef struct {{")
        lines.append(_gen_params_fields(fields))
        lines.append(f"}} MODULE_INPUT_PARAMS({from_name}, {mod_name});")

        # MODULE_INPUT_LINK
        link_member = _link_style_to_c(in_style, in_size)
        lines.append("")
        lines.append(f"typedef struct {{")
        lines.append(f"    uint8_t  status;           /* ST_NEW / ST_OUT */")
        lines.append(f"    uint8_t  max_count;        /* 最大数量 */")
        lines.append(f"    uint8_t  count;            /* 当前周期索引 */")
        lines.append(f"    uint8_t  res[1];")
        lines.append(f"    MODULE_INPUT_PARAMS({from_name}, {mod_name}) {link_member};")
        lines.append(f"}} MODULE_INPUT_LINK({from_name}, {mod_name});")

    # MODULE_INPUT 聚合
    if in_pipes:
        lines.append("")
        lines.append(f"/* {mod_name}_Input — 输入聚合 (对称命名: 成员 = {{Producer}}_params) */")
        lines.append(f"typedef struct {{")
        for pipe in in_pipes:
            p_name = pipe["from"]
            lines.append(f"    MODULE_INPUT_LINK({p_name}, {mod_name}) *{p_name}_params;  /* 指向 {p_name} 输出的 LINK 列 */")
        lines.append(f"}} MODULE_INPUT({mod_name});")
    else:
        lines.append("")
        lines.append(f"/* {mod_name}_Input — 无输入 */")
        lines.append(f"typedef struct {{")
        lines.append(f"    uint8_t  res[4];")
        lines.append(f"}} MODULE_INPUT({mod_name});")

    # Unpack
    if pack:
        lines.append("")
        lines.append("#pragma pack()")

    # MODULE_IO_H
    lines.append("")
    lines.append(f"/* ---- v2.3 统一接口 ---- */")
    lines.append(f"MODULE_IO_H({mod_name});")

    # End guard
    lines.append("")
    lines.append(f"#endif /* {guard_name} */")
    lines.append("")

    return "\n".join(lines)
