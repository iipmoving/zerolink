#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
gen_module_c.py — v2.3 模块 .c 骨架生成器 (单块模式)

文件结构:
  /**
   * @file ...
   */
  [AI GENERATED] — 全部 AI 内容在文件头部, 每次重生成
  [用户代码]     — 保持不变

AI 块内容:
  - #include "module_io.h"
  - MODULE_SKELETON(Module);
  - PipeFlags_t (按输入管道)
  - void Module_Process(...) 前向声明
  - static void Init(void)
  - static void ProcessInput(void)
  - MODULE_EXPORT(Module)
"""

import os
import re

# ========== 块标记 ==========
AI_BLOCK_BEGIN = "// ===== [AI GENERATED] 范式接入+骨架, 可被PY替换 ====="
AI_BLOCK_END   = "// ===== [END AI GENERATED] ====="

# 旧标记 (兼容清理)
_OLD_MARKERS = [
    ("// ===== [AI INSERTED] 范式接入声明 =====", "// ===== [END AI INSERTED] ====="),
    ("// ===== [AI GENERATED] 范式骨架, 可被PY替换 =====", "// ===== [END AI GENERATED] ====="),
]


def _pascal_to_snake(name: str) -> str:
    s1 = re.sub(r'([A-Z]+)([A-Z][a-z])', r'\1_\2', name)
    return re.sub(r'([a-z0-9])([A-Z])', r'\1_\2', s1).lower()


# ========== 生成 ==========

def _generate_ai_block(mod_name: str, in_pipes: list, out_pipes: list,
                       io_dir: str = "include", source_file: str = "") -> str:
    """生成单个 AI 块: include + skeleton + PipeFlags_t + user_Process 前向声明 + ProcessInput + MODULE_EXPORT"""
    p_names = [p["from"] for p in in_pipes]
    lines = []

    # 计算 #include 路径: 从源文件目录到 io.h 的相对路径
    snake = _pascal_to_snake(mod_name)
    include_line = f'#include "{snake}_io.h"'
    if source_file and io_dir:
        src_dir = os.path.dirname(source_file)
        io_path = os.path.join(io_dir, f"{snake}_io.h")
        try:
            rel = os.path.relpath(io_path, src_dir).replace("\\", "/")
            include_line = f'#include "{rel}"'
        except ValueError:
            pass  # 回退到裸文件名

    lines.append(AI_BLOCK_BEGIN)
    lines.append(include_line)
    lines.append("")
    lines.append(f"MODULE_SKELETON({mod_name});")
    lines.append("")

    # PipeFlags_t — 始终定义, 使函数签名统一
    lines.append("/* 管道就绪标志: 每 BIT 代表一个管道的 ST_NEW 状态 */")
    lines.append("typedef union {")
    lines.append("    uint8_t all;")
    lines.append("    struct {")
    if p_names:
        for p in in_pipes:
            lines.append(f"        uint8_t {p['from'].lower()}   : 1;  /* {p['from']} 数据就绪 */")
    else:
        lines.append("        uint8_t _unused;")
    lines.append("    } bits;")
    lines.append(f"}} {mod_name}_PipeFlags_t;")
    lines.append("")

    # user_Process 前向声明 — 统一签名, NULL 由用户内部处理
    lines.append(f"static void user_Process(MODULE_INPUT({mod_name}) *in, MODULE_OUTPUT({mod_name}) *out, {mod_name}_PipeFlags_t flags);")
    lines.append("")

    # ProcessInput
    lines.append("static void ProcessInput(void)")
    lines.append("{")
    lines.append(f"    MODULE_INPUT({mod_name}) *in  = (MODULE_INPUT({mod_name})*)g_input.para;")
    lines.append(f"    MODULE_OUTPUT({mod_name}) *out = (MODULE_OUTPUT({mod_name})*)g_output.para;")
    lines.append("")
    lines.append("    /* === 输入段: 数据有效检查 === */")
    lines.append(f"    {mod_name}_PipeFlags_t flags = {{0}};")
    if in_pipes:
        for pn in p_names:
            lines.append(f"    flags.bits.{pn.lower()} = (in->{pn}_params->status & ST_NEW) ? 1 : 0;")
    lines.append("")
    lines.append("    /* === 计算段: 用户业务 === */")
    lines.append(f"    user_Process(in, out, flags);")
    lines.append("}")
    lines.append("")

    lines.append(f"MODULE_EXPORT({mod_name});")
    lines.append("")
    lines.append(AI_BLOCK_END)

    return "\n".join(lines)


# ========== 块处理工具 ==========

def _strip_ai_blocks(content: str) -> str:
    """去除文件中所有 AI 标记块 (当前 + 旧版兼容)"""
    # 当前标记
    pat = re.compile(re.escape(AI_BLOCK_BEGIN) + r".*?" + re.escape(AI_BLOCK_END), re.DOTALL)
    content = pat.sub("", content)

    # 旧版兼容
    for begin, end in _OLD_MARKERS:
        pat = re.compile(re.escape(begin) + r".*?" + re.escape(end), re.DOTALL)
        content = pat.sub("", content)

    return content.strip("\n")


def strip_ai_blocks(content: str) -> str:
    """公开接口: 去除 AI 标记块"""
    return _strip_ai_blocks(content)


def wrap_in_ai_block(content: str) -> str:
    """将内容包裹在 AI GENERATED 标记中"""
    return f"{AI_BLOCK_BEGIN}\n{content}\n{AI_BLOCK_END}"


def find_ai_block(content: str) -> tuple:
    """查找 AI 块位置: 返回 (start, end), 未找到返回 (-1, -1)"""
    start = content.find(AI_BLOCK_BEGIN)
    if start < 0:
        return (-1, -1)
    end = content.find(AI_BLOCK_END, start)
    if end < 0:
        return (-1, -1)
    return (start, end + len(AI_BLOCK_END))


def replace_ai_block(content: str, new_block: str) -> str:
    """替换文件中已有的 AI 块为新块; 无 AI 块时插入到文件头部"""
    start, end = find_ai_block(content)
    if start >= 0:
        return content[:start] + new_block + content[end:]
    else:
        return new_block + "\n" + content


# ========== 主接口 ==========

def generate_module_c(module: dict, pipes: list, project: dict, existing_file: str = "") -> str:
    """生成全新模块 .c 文件内容"""
    mod_name = module["name"]
    mod_lower = _pascal_to_snake(mod_name)
    layer = module.get("layer", "app")
    comment = module.get("comment", "")

    out_pipes = [p for p in pipes if p["from"] == mod_name]
    in_pipes  = [p for p in pipes if p["to"] == mod_name]

    parts = []

    # 文件头
    header = []
    header.append("/**")
    header.append(f" * @file    {mod_lower}.c")
    header.append(f" * @brief   {mod_name} 模块实现 (v2.3 LINK+PARAMS)")
    header.append(f" * @layer   {layer}")
    if comment:
        header.append(" *")
        header.append(f" * {comment}")
    header.append(" */")
    parts.append("\n".join(header))

    # AI 块 (全部在文件头部)
    io_dir = project.get("paths", {}).get("io_dir", "include")
    parts.append(_generate_ai_block(mod_name, in_pipes, out_pipes, io_dir, module.get("source_file", "")))

    # 用户代码占位
    parts.append("// (新模块 — 在此插入业务代码)")

    return "\n\n".join(parts) + "\n"


def generate_module_c_refactored(
    module: dict,
    pipes: list,
    project: dict,
    original_path: str,
) -> str:
    """生成重构后的 .c 内容: 替换头部 AI 块，保留原内容"""
    mod_name = module["name"]
    out_pipes = [p for p in pipes if p["from"] == mod_name]
    in_pipes  = [p for p in pipes if p["to"] == mod_name]

    original_content = ""
    if original_path:
        try:
            with open(original_path, "r", encoding="utf-8") as f:
                original_content = f.read()
        except (IOError, UnicodeDecodeError):
            original_content = ""

    clean_content = _strip_ai_blocks(original_content).strip("\n")

    parts = []

    # [1] AI 块 (文件头部)
    io_dir = project.get("paths", {}).get("io_dir", "include")
    parts.append(_generate_ai_block(mod_name, in_pipes, out_pipes, io_dir, module.get("source_file", "")))

    # [2] 原用户代码
    if clean_content:
        parts.append(clean_content)
    else:
        parts.append("// (新模块 — 在此插入业务代码)")

    return "\n\n".join(parts) + "\n"


def generate_module_h(module: dict) -> str:
    """生成模块 .h 文件"""
    mod_name = module["name"]
    snake_name = _pascal_to_snake(mod_name)
    layer = module.get("layer", "app")
    comment = module.get("comment", "")
    guard = f"{mod_name.upper()}_H"

    lines = []
    lines.append("/**")
    lines.append(f" * @file    {snake_name}.h")
    lines.append(f" * @brief   {mod_name} 模块公共接口")
    lines.append(f" * @layer   {layer}")
    if comment:
        lines.append(" *")
        lines.append(f" * {comment}")
    lines.append(" */")
    lines.append("")
    lines.append(f"#ifndef {guard}")

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
