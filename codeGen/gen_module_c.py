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
    lines.append(f"static void Init(void);")
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

    # ---- 数据实体（模块私有）----
    lines.append("/* ---- 数据实体（模块私有）---- */")
    lines.append(f"static MODULE_INPUT({mod_name})*   s_inPara;    // 输入参数实体在ADC， 这里只调用不修改")
    lines.append(f"static MODULE_OUTPUT({mod_name})  s_outPara;   // 输出参数缓冲区")
    lines.append("")

    # ---- 消费者 seq 比对变量 (每条输入管道一个) ----
    if in_pipes:
        lines.append("/* ---- 消费者 last_seq — seq 有效性比对 (空闲SLOT幂等) ---- */")
        for p in in_pipes:
            p_name = p["from"]
            lines.append(f"static uint8_t s_last_seq_{p_name};  /* {p_name}→{mod_name} */")
        lines.append("")

    # ---- 内部 OUTPUT_LINK + PARAMS 实例 ----
    if out_pipes:
        lines.append("/* ---- 内部 OUTPUT_LINK + PARAMS 实例 ---- */")
        for pipe in out_pipes:
            c_name = pipe["to"]
            lines.append(f"static MODULE_OUTPUT_PARAMS({mod_name}, {c_name})  s_{mod_name}To{c_name}Params;")
            lines.append(f"static MODULE_OUTPUT_LINK({mod_name}, {c_name})  s_{mod_name}To{c_name}Link;")
        lines.append("")

    lines.append("")

    # user_Process 默认空壳 — 用户在 END AI GENERATED 后重写即可 (dedup 会包掉旧版本)
    lines.append(f"/* 用户业务入口: flags.bits 指示哪些管道有新数据 (seq 比对通过)")
    lines.append(f" * 输出段请对产出数据的 LINK 执行 seq++: out->{'' if not out_pipes else out_pipes[0]['to']}_params->seq++; */")
    lines.append(f"static void user_Process(MODULE_INPUT({mod_name}) *in, MODULE_OUTPUT({mod_name}) *out, {mod_name}_PipeFlags_t flags)")
    lines.append(f"{{")
    lines.append(f"    (void)in; (void)out; (void)flags;")
    lines.append(f"}}")
    lines.append("")

    # ProcessInput
    lines.append("static void ProcessInput(void)")
    lines.append("{")
    lines.append(f"    MODULE_INPUT({mod_name}) *in  = (MODULE_INPUT({mod_name})*)g_input.para;")
    lines.append(f"    MODULE_OUTPUT({mod_name}) *out = (MODULE_OUTPUT({mod_name})*)g_output.para;")
    lines.append("")
    lines.append("    /* === 输入段: seq 有效性比对 === */")
    lines.append(f"    {mod_name}_PipeFlags_t flags = {{0}};")
    if in_pipes:
        for p in in_pipes:
            p_name = p["from"]
            lines.append(f"    {{")
            lines.append(f"        uint8_t cur_seq = in->{p_name}_params->seq;")
            lines.append(f"        if (cur_seq != s_last_seq_{p_name}) {{")
            lines.append(f"            flags.bits.{p_name.lower()} = 1;")
            lines.append(f"            s_last_seq_{p_name} = cur_seq;")
            lines.append(f"        }}")
            lines.append(f"    }}")
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
        after = content[end:].lstrip("\n")
        return content[:start] + new_block + ("\n" + after if after else "\n")
    else:
        return new_block + "\n" + content


# ========== 主接口 ==========

_FILE_HEADER_RE = re.compile(
    r'^\s*/\*\*?\s*\n'          
    r'(?:\s*\*\s*[^\n]*\n)*'   
    r'\s*\*/\s*\n',            
    re.MULTILINE
)


def _strip_file_header(content: str) -> str:
    """去除文件开头的 Doxygen 风格注释块"""
    return _FILE_HEADER_RE.sub("", content, count=1).lstrip("\n")


def generate_module_c(module: dict, pipes: list, project: dict, existing_file: str = "") -> str:
    """生成模块 .c 的纯 AI 块内容 (不含用户代码)

    原则:
      - 只生成 AI 块, 不含用户代码
      - 用户代码保留由 save_gen_file + replace_ai_block 负责
      - 无文件: save_gen_file 写入 AI 块 + 占位注释
    """
    mod_name = module["name"]
    out_pipes = [p for p in pipes if p["from"] == mod_name]
    in_pipes  = [p for p in pipes if p["to"] == mod_name]

    io_dir = project.get("paths", {}).get("io_dir", "include")

    return _generate_ai_block(mod_name, in_pipes, out_pipes, io_dir, module.get("source_file", ""))


def generate_module_c_refactored(
    module: dict,
    pipes: list,
    project: dict,
    original_path: str,
) -> str:
    """生成重构后的 .c 内容 — 与 generate_module_c 统一逻辑

    保留此函数签名以兼容 GUI 调用, 内部委托给 generate_module_c
    """
    return generate_module_c(module, pipes, project, existing_file=original_path)


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
