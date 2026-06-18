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

# AI 块标记 (与 gen_module_c.py 一致)
AI_BLOCK_BEGIN = "// ===== [AI GENERATED] 范式接入+骨架, 可被PY替换 ====="
AI_BLOCK_END   = "// ===== [END AI GENERATED] ====="


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


def _link_style_to_c(style: str, dims: str = "") -> str:
    """LINK 样式 → C 声明

    Args:
        style: "pointer" | "array" | "pointer_array"
        dims: 完整维度字符串, e.g. "[4]", "[4][20]", ""
    """
    if style == "pointer":
        return "*params"
    elif style == "pointer_array":
        return f"*params{dims}"
    else:  # array
        if dims:
            return f"params{dims}"
        else:
            return "params[4]"  # default single dimension


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
    """从 fields[] 生成 PARAMS 结构体字段

    支持的字段类型:
    - 简单类型: uint16_t, int32_t, float, ...
    - 数组字段: name="res[3]", type="uint8_t" → "uint8_t res[3];"
    - 多维数组: name="params[4][20]", type="T" → "T params[4][20];"
    - 指针类型: type="uint16_t**" → "uint16_t** name;"
    - 自定义指针: type="const MODULE_INPUT_PARAMS(A, B) *" → 原样输出, *紧贴name
    - 嵌套 struct: type="struct", fields=[...]
    """
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
            continue

        # 处理指针类型:
        # - type="uint16_t**"                → "uint16_t** name"  (简单指针: 加空格)
        # - type="AppAdc_OutputParams_t*"    → "AppAdc_OutputParams_t* name"  (简单指针: 加空格)
        # - type="const T(...) *"            → "const T(...) *name"  (复杂指针: *前有空格, *紧贴name)
        # 规则: 若 * 前是空格, 则 * 紧贴 name; 否则 type 和 name 之间加空格
        type_str = raw_type
        if type_str.endswith('*'):
            # 检查 * 前面是否有空格
            star_idx = type_str.rfind('*')
            if star_idx > 0 and type_str[star_idx - 1] == ' ':
                # 复杂指针: * 紧贴 name
                if comment:
                    lines.append(f"{prefix}{type_str}{name};     /* {comment} */")
                else:
                    lines.append(f"{prefix}{type_str}{name};")
            else:
                # 简单指针: type 和 name 之间加空格
                if comment:
                    lines.append(f"{prefix}{type_str} {name};     /* {comment} */")
                else:
                    lines.append(f"{prefix}{type_str} {name};")
        else:
            # 非指针: type + 空格 + name
            if comment:
                lines.append(f"{prefix}{type_str} {name};     /* {comment} */")
            else:
                lines.append(f"{prefix}{type_str} {name};")
    return "\n".join(lines)


def _pascal_to_snake(name: str) -> str:
    """PascalCase → snake_case: AppAdc → app_adc"""
    s1 = re.sub(r'([A-Z]+)([A-Z][a-z])', r'\1_\2', name)
    return re.sub(r'([a-z0-9])([A-Z])', r'\1_\2', s1).lower()


# 标准 C 类型白名单 — 这些类型不需要在本模块 io.h 中重新声明
STANDARD_TYPES = {
    'void', 'char', 'signed char', 'unsigned char',
    'int8_t', 'int16_t', 'int32_t', 'int64_t',
    'uint8_t', 'uint16_t', 'uint32_t', 'uint64_t',
    'float', 'double',
    'bool', '_Bool',
    'size_t', 'ssize_t', 'ptrdiff_t',
    'int', 'short', 'long', 'unsigned', 'unsigned int',
}


def _is_standard_type(type_str: str) -> bool:
    """判断是否标准 C 类型（不需要重新声明）"""
    # 去掉指针和空格
    bare = type_str.replace('*', '').strip()
    return bare in STANDARD_TYPES


def _collect_custom_types(module: dict, pipes: list, all_modules: list) -> list:
    """收集本模块 io.h 中需要重新声明的自定义结构体类型

    扫描本模块作为 producer/consumer 的所有管道字段，
    找出 type 是自定义类型（非标准 C 类型）的字段，
    从全局模块列表中找到原类型定义，重命名为本模块前缀。

    返回: [{"orig_name": "AppAdc_OutputParams_t",
            "local_name": "Calculator_OutputParams_t",
            "fields": [...]}]
    """
    mod_name = module["name"]
    out_pipes = [p for p in pipes if p["from"] == mod_name]
    in_pipes = [p for p in pipes if p["to"] == mod_name]

    # 收集所有字段引用的自定义类型名
    custom_type_refs = []  # [(orig_name, in_consumer_side)]
    for pipe in out_pipes:
        for f in pipe.get("fields", []):
            t = f.get("type", "")
            bare = t.replace('*', '').strip()
            if bare and not _is_standard_type(t):
                # producer 侧: 用 producer 自己的命名（本地 types）
                if bare not in [x[0] for x in custom_type_refs]:
                    custom_type_refs.append((bare, False))
    for pipe in in_pipes:
        for f in pipe.get("fields", []):
            t = f.get("type", "")
            bare = t.replace('*', '').strip()
            if bare and not _is_standard_type(t):
                # consumer 侧: 重命名为本模块前缀
                if bare not in [x[0] for x in custom_type_refs]:
                    custom_type_refs.append((bare, True))

    # 从全局模块列表中找到原类型定义
    result = []
    for orig_name, is_consumer in custom_type_refs:
        # 在所有模块的 types 中找
        orig_def = None
        orig_owner = None
        for m in all_modules:
            for t in m.get("types", []):
                if t["name"] == orig_name:
                    orig_def = t
                    orig_owner = m["name"]
                    break
            if orig_def:
                break
        if not orig_def:
            continue

        # 重命名规则:
        # - producer 侧 (本模块): 沿用原名 (本模块自己声明的)
        # - consumer 侧: {本模块名}_InputParams_t (本模块独立声明)
        # 但实际项目里 Calculator_InputParams_t 是用 _InputParams_t 后缀
        # 简化: 提取原名的后缀部分, 替换前缀为本模块名
        # AppAdc_OutputParams_t → Calculator_OutputParams_t? 或 Calculator_InputParams_t?
        # 实际 calculator_io.h 用的是 Calculator_InputParams_t
        # 规则: 若原名以 _OutputParams_t 结尾, 改为 {Consumer}_InputParams_t (语义对调)
        #       否则用 {Consumer}_{原名去掉原 owner 前缀}
        if orig_name.endswith('_OutputParams_t'):
            local_name = f"{mod_name}_InputParams_t"
        elif orig_name.startswith(orig_owner + '_'):
            local_name = orig_name.replace(orig_owner + '_', mod_name + '_', 1)
        else:
            local_name = f"{mod_name}_{orig_name}"

        result.append({
            "orig_name": orig_name,
            "local_name": local_name,
            "fields": orig_def.get("fields", []),
            "is_consumer_side": is_consumer,
            "orig_owner": orig_owner,
        })
    return result


def _rename_field_type(type_str: str, custom_types: list) -> str:
    """把字段类型中的自定义类型名替换为本地重命名后的名字

    e.g. "AppAdc_OutputParams_t*" + custom_types 含 AppAdc_OutputParams_t→Calculator_InputParams_t
         → "Calculator_InputParams_t*"
    """
    bare = type_str.replace('*', '').strip()
    for ct in custom_types:
        if bare == ct["orig_name"]:
            # 保留原有的指针符号
            star_count = type_str.count('*')
            star = '*' * star_count
            space_before_star = ' ' if ' *' in type_str else ''
            return f"{ct['local_name']}{space_before_star}{star}"
    return type_str


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

    # 收集本模块需要独立声明的自定义结构体类型（头文件自治规则）
    all_modules = project.get("modules", [])
    custom_types = _collect_custom_types(module, pipes, all_modules)

    # 收集本模块 io.h 中需要重新声明的自定义结构体类型
    # (_io.h 不能跨模块 include, 所以 consumer 侧必须自包含声明)
    custom_types = _collect_custom_types(module, pipes, project.get("modules", []))

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
        out_dims = pipe.get("out_link", {}).get("dims", "")
        alias_of = pipe.get("alias_of")

        lines.append("")
        lines.append(f"/* ------------------------------------------------------------------")
        lines.append(f" * {mod_name} → {to_name}  输出参数  ({pipe.get('comment', '')})")
        lines.append(f" * ------------------------------------------------------------------ */")

        if alias_of:
            # alias 管道: typedef MODULE_OUTPUT_PARAMS(A, B) MODULE_OUTPUT_PARAMS(A, C);
            # alias_of 存的是原始 to 名 (如 ElecParams)
            lines.append(f"/* 直接重命名 OUTPUT_PARAMS，不复述成员 */")
            lines.append(f"typedef MODULE_OUTPUT_PARAMS({mod_name}, {alias_of}) MODULE_OUTPUT_PARAMS({mod_name}, {to_name});")
            lines.append("")
            lines.append(f"/* 直接重命名 OUTPUT_LINK，不复述成员 */")
            lines.append(f"typedef MODULE_OUTPUT_LINK({mod_name}, {alias_of}) MODULE_OUTPUT_LINK({mod_name}, {to_name});")
        else:
            # 完整 struct 定义
            lines.append(f"typedef struct {{")
            lines.append(_gen_params_fields(fields))
            lines.append(f"}} MODULE_OUTPUT_PARAMS({mod_name}, {to_name});")

            # MODULE_OUTPUT_LINK
            link_member = _link_style_to_c(out_style, out_dims)
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
            suffix = "*" if pipe.get("alias_of") else ""
            lines.append(f"    MODULE_OUTPUT_LINK({mod_name}, {c_name}) {suffix}{c_name}_params;  /* → {c_name} */")
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
        # 优先用 in_fields (consumer 视角，自定义类型用本模块命名)
        # 回退到 fields (producer 视角)
        fields = pipe.get("in_fields") or pipe.get("fields", [])
        in_style = pipe.get("in_link", {}).get("style", pipe.get("out_link", {}).get("style", "array"))
        in_dims = pipe.get("in_link", {}).get("dims", pipe.get("out_link", {}).get("dims", ""))

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
        link_member = _link_style_to_c(in_style, in_dims)
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
