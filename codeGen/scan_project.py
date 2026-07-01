#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
scan_project.py — 纯 Python 实现：扫描源码 → project.json

替代 Claude CLI gen-json skill，直接从源文件解析模块/管道/字段。
不需要 AI，不需要 TTY，确定性强。

用法:
    python scan_project.py <项目路径> [--keil <uvprojx路径>]
"""

import os
import re
import json
import glob


# ── 正则表达式 ──

RE_LAYER = re.compile(r'@layer\s+(\S+)')

# @brief 行（取整行内容）
RE_BRIEF = re.compile(r'@brief\s+(.*)', re.MULTILINE)

# MODULE_IO_H(Name)  — 模块声明
RE_IO_H_NAME = re.compile(r'MODULE_IO_H\s*\(\s*(\w+)\s*\)')

# MODULE_SKELETON(Name)  — 仅在 .c 中
RE_MODULE_SKELETON = re.compile(r'MODULE_SKELETON\s*\(\s*(\w+)\s*\)')

# 独立 typedef struct { ... } TypeName;
RE_TYPEDEF = re.compile(r'typedef\s+struct\s*\{([^}]+)\}\s*(\w+);', re.DOTALL)

# alias 声明: typedef MODULE_OUTPUT_PARAMS(From, To) MODULE_OUTPUT_PARAMS(From, Alias);
RE_ALIAS_OUTPUT = re.compile(r'typedef\s+MODULE_OUTPUT_PARAMS\s*\((\w+)\s*,\s*(\w+)\)\s+MODULE_OUTPUT_PARAMS\s*\((\w+)\s*,\s*(\w+)\)\s*;')
RE_ALIAS_OUTPUT_LINK = re.compile(r'typedef\s+MODULE_OUTPUT_LINK\s*\((\w+)\s*,\s*(\w+)\)\s+MODULE_OUTPUT_LINK\s*\((\w+)\s*,\s*(\w+)\)\s*;')

# 管道参数: struct { ... } MODULE_OUTPUT_PARAMS(From, To);
RE_OUTPUT_PARAMS = re.compile(r'struct\s*\{([^}]+)\}\s*MODULE_OUTPUT_PARAMS\s*\(\s*(\w+)\s*,\s*(\w+)\s*\)', re.DOTALL)
RE_INPUT_PARAMS  = re.compile(r'struct\s*\{([^}]+)\}\s*MODULE_INPUT_PARAMS\s*\(\s*(\w+)\s*,\s*(\w+)\s*\)', re.DOTALL)

# LINK 宏名匹配
RE_OUTPUT_LINK_MACRO = re.compile(r'MODULE_OUTPUT_LINK\s*\(\s*(\w+)\s*,\s*(\w+)\s*\)')
RE_INPUT_LINK_MACRO  = re.compile(r'MODULE_INPUT_LINK\s*\(\s*(\w+)\s*,\s*(\w+)\s*\)')

# 从 LINK 结构体中提取 params 声明: params[N] 或 *params 或 params[N][M]
# 类型可能是简单类型或宏调用: MODULE_OUTPUT_PARAMS(A, B) params[4][20];
RE_LINK_PARAMS = re.compile(r'(\w+(?:\s*\([^)]*\))?)\s*(\*?)\s*params\s*((?:\[\d+(?:\s*\*\s*\d+)?\])*)')

# SLOT(Name) 在 data_switcher.c 中 — 只在 enum 定义中匹配
RE_SLOT_ENUM = re.compile(r'^\s*SLOT\s*\(\s*(\w+)\s*\)\s*=', re.MULTILINE)

# 字段行: type name; /* comment */
RE_FIELD_LINE = re.compile(r'^\s*(\S+(?:\s+\S+)*?)\s+(\w+)\s*;\s*(?:(?://|/\*)\s*(.*?)(?:\*/)?\s*)?$', re.MULTILINE)

# #include "..._io.h"
RE_INCLUDE_IO = re.compile(r'#include\s+"([^"]*_io\.h)"')


def to_snake_id(name):
    """PascalCase → snake_case id，处理带下划线的名称"""
    name = re.sub(r'([A-Z]+)([A-Z][a-z])', r'\1_\2', name)
    name = re.sub(r'([a-z0-9])([A-Z])', r'\1_\2', name)
    name = re.sub(r'[^a-zA-Z0-9]', '_', name)
    name = re.sub(r'_+', '_', name)
    name = name.strip('_').lower()
    return name


def read_file(path):
    try:
        with open(path, 'r', encoding='utf-8', errors='replace') as f:
            return f.read()
    except:
        return None


def parse_fields(struct_body):
    """从 struct 体解析字段列表 (v2: 保留 res[n]、支持自定义指针类型)

    逐行解析，从行尾向前匹配：最后一个 \\w+ 为字段名，之前为类型。
    支持: uint8_t res[3], const TYPE *ptr, TYPE** dptr, float val
    """
    fields = []
    for line in struct_body.split('\n'):
        line = line.strip()
        if not line or line.startswith('//') or line.startswith('/*') or line.startswith('*'):
            continue
        if ';' not in line:
            continue

        # 提取注释
        comment = ''
        cmt = re.search(r'(?://|/\*\*?<?)\s*(.+?)(?:\*/)?\s*$', line)
        if cmt:
            comment = cmt.group(1).strip()
            line = line[:cmt.start()].strip()

        # 去掉末尾分号
        line = line.rstrip(';').strip()
        if not line:
            continue

        # 贪婪匹配 .+ 然后回溯找最后的 \\w+ 作为字段名
        m = re.match(r'^(.+)\s+(\*?\s*\w+)(\s*\[[^\]]*\])*\s*$', line)
        if not m:
            continue

        ftype = m.group(1).strip()
        name_part = m.group(2).strip()  # e.g. "*calc_copy" or "valid" or "**resonant_current"

        # 分离指针前缀
        ptr_prefix = ''
        rest = name_part
        while rest.startswith('*'):
            ptr_prefix += '*'
            rest = rest[1:].lstrip()
        fname = rest

        # 指针归入类型
        if ptr_prefix:
            ftype = ftype + ' ' + ptr_prefix

        # 检查数组后缀: name[3] 或 name[4][20]
        arr_suffix = ''
        arr_m = re.search(r'\b' + re.escape(fname) + r'((?:\s*\[[^\]]*\])+)', line)
        if arr_m:
            arr_suffix = arr_m.group(1).replace(' ', '')

        fields.append({
            "name": fname + arr_suffix,
            "type": ftype,
            "comment": comment,
        })
    return fields


def find_brief(content, default=""):
    """提取模块描述: @brief 行后的说明段（到 输入源/输出目标/空行结束）

    跳过 @file, @brief, @layer 等元数据行。
    """
    # 找到 @layer 行后到 */ 之前的所有内容
    m = re.search(r'@layer\s+\S+\s*\n(.*?)\*/', content, re.DOTALL)
    if not m:
        # 退回到 @brief 行后
        m = re.search(r'@brief\s+[^\n]*\n(.*?)\*/', content, re.DOTALL)
    if not m:
        return default
    block = m.group(1)
    # 提取第一段连续的 * 行作为描述（遇到"输入源"/"输出目标"/"输入:"/"输出:"停止）
    desc_lines = []
    for line in block.split('\n'):
        line = line.strip()
        if not line:
            if desc_lines:
                break
            continue
        # 去掉注释前缀
        if line.startswith('*'):
            line = line[1:].strip()
        if not line:
            continue
        if any(kw in line for kw in ['输入源', '输出目标', '输入:', '输出:', 'Inputs', 'Outputs']):
            break
        # 跳过其他 @ 标签
        if line.startswith('@'):
            continue
        desc_lines.append(line)
    return ' '.join(desc_lines).strip() or default


def find_pipe_comment(content, from_mod, to_mod):
    """从 /* --- ... (comment) --- */ 注释块提取管道 comment

    实际 io.h 中格式:
        /* ------------------------------------------------------------------
         * FromMod → ToMod  输出参数  (comment)
         * ------------------------------------------------------------------ */
    """
    pattern = rf'{from_mod}\s*→\s*{to_mod}\s+\S+\s+\(([^)]+)\)'
    m = re.search(pattern, content)
    if m:
        return m.group(1).strip()
    return ""


def find_layer(content):
    """提取 @layer"""
    m = RE_LAYER.search(content)
    return m.group(1).strip() if m else "app"


def extract_pipe_comment(content, from_mod, to_mod):
    """从 io.h 的 /* ---- From → To ... (comment) --- */ 注释块中提取管道描述

    匹配格式: * From → To  输出参数  (实际描述)
    或: * From → To  输入参数  (实际描述)
    描述本身可能包含括号，用贪婪匹配到最后一个 )
    """
    pattern = rf'\*+\s*{re.escape(from_mod)}\s*→\s*{re.escape(to_mod)}\s+\S+\s+\((.+)\)\s*(?:\*/|\s*$)'
    m = re.search(pattern, content, re.MULTILINE)
    if m:
        return m.group(1).strip()
    return ""


def extract_link_style(content, from_mod, to_mod, link_type):
    """从 LINK 结构中提取 style 和 array_dims

    Returns: (style, dims_string)
      style: "pointer" | "array"
      dims_string: "[4][20]" | "[4]" | "" — 完整维度声明（包含方括号）
    """
    # 搜索包含 "MODULE_PARAMS(from, to) params" 的行
    pattern = rf'MODULE_[A-Z_]+_PARAMS\({from_mod},\s*{to_mod}\)\s*params'
    m = re.search(pattern, content)
    if not m:
        return "pointer", ""

    # 从匹配位置向前找 struct 体开始，向后找 params 声明
    snippet = content[max(0, m.start()-500):m.end()+100]

    # 在 snippet 中找 params 声明
    for lm in RE_LINK_PARAMS.finditer(snippet):
        style = "pointer" if lm.group(2) == '*' else "array"
        raw_dims = lm.group(3)  # e.g. "[4][20]" or "[4]" or ""
        return style, raw_dims
    return "pointer", ""


def scan_io_file(path, proj_dir):
    """扫描单个 _io.h 文件，返回 (module_dict, pipe_list)"""
    content = read_file(path)
    if not content:
        return None

    rel_path = os.path.relpath(path, proj_dir).replace('\\', '/')

    # 模块名
    m = RE_IO_H_NAME.search(content)
    if not m:
        return None
    module_name = m.group(1)

    layer = find_layer(content)
    brief = find_brief(content)

    # 独立 typedef（不排除宏包裹的，因为 _OutputParams_t 这类命名也要扫）
    types = []
    for m in RE_TYPEDEF.finditer(content):
        tname = m.group(2).strip()
        # 仅排除 MODULE_OUTPUT_PARAMS / MODULE_INPUT_PARAMS / LINK 宏展开后的 typedef
        # （这些是宏调用不是 typedef struct）
        if tname.startswith('MODULE_'):
            continue
        tbody = m.group(1)
        tfields = parse_fields(tbody)
        types.append({"name": tname, "comment": "", "fields": tfields})

    # 检测 alias OUTPUT 管道: typedef MODULE_OUTPUT_PARAMS(A, B) MODULE_OUTPUT_PARAMS(A, C);
    # 意味着 (A, C) 是 (A, B) 的别名，管道复用 B 的 fields/link 配置
    alias_map = {}  # (from, to_alias) → (from, to_original)
    for m in RE_ALIAS_OUTPUT.finditer(content):
        orig_from = m.group(1)
        orig_to = m.group(2)
        alias_from = m.group(3)
        alias_to = m.group(4)
        if alias_from == orig_from and alias_to == orig_to:
            continue  # 不是 alias，跳过
        if alias_from == orig_from:
            alias_map[(alias_from, alias_to)] = (orig_from, orig_to)
    for m in RE_ALIAS_OUTPUT_LINK.finditer(content):
        orig_from = m.group(1)
        orig_to = m.group(2)
        alias_from = m.group(3)
        alias_to = m.group(4)
        if alias_from == orig_from and alias_to == orig_to:
            continue
        if alias_from == orig_from:
            alias_map[(alias_from, alias_to)] = (orig_from, orig_to)

    # 输出管道
    out_pipes = []
    for m in RE_OUTPUT_PARAMS.finditer(content):
        from_mod = m.group(2).strip()
        to_mod = m.group(3).strip()
        fields = parse_fields(m.group(1))
        out_link = extract_link_style(content, from_mod, to_mod, 'OUTPUT')
        in_link = extract_link_style(content, from_mod, to_mod, 'INPUT')
        pipe_comment = extract_pipe_comment(content, from_mod, to_mod)
        pipe_id = to_snake_id(f"pipe_{from_mod}_{to_mod}")
        pipe = {
            "id": pipe_id,
            "from": from_mod,
            "to": to_mod,
            "callback_type": "pull",
            "comment": pipe_comment,
            "out_link": {"style": out_link[0], "dims": out_link[1]},
            "in_link": {"style": in_link[0], "dims": in_link[1]},
            "fields": fields,
        }
        out_pipes.append(pipe)

    # 为 alias 管道创建虚拟条目（复用 original 管道的 fields/link）
    for (a_from, a_to), (o_from, o_to) in alias_map.items():
        orig_pipe = None
        for p in out_pipes:
            if p['from'] == o_from and p['to'] == o_to:
                orig_pipe = p
                break
        if orig_pipe:
            alias_id = to_snake_id(f"pipe_{a_from}_{a_to}")
            out_pipes.append({
                "id": alias_id,
                "from": a_from,
                "to": a_to,
                "callback_type": "pull",
                "comment": orig_pipe["comment"],
                "out_link": {"style": "pointer", "dims": ""},  # alias 转发用 pointer
                "in_link": dict(orig_pipe["in_link"]),
                "fields": orig_pipe["fields"],
                "alias_of": o_to,  # 存原始 to 名 (ElecParams / EKF_LKF)
            })

    # 输入管道
    in_pipes = []
    for m in RE_INPUT_PARAMS.finditer(content):
        from_mod = m.group(2).strip()
        to_mod = m.group(3).strip()
        fields = parse_fields(m.group(1))
        out_link = extract_link_style(content, from_mod, to_mod, 'OUTPUT')
        in_link = extract_link_style(content, from_mod, to_mod, 'INPUT')
        pipe_comment = extract_pipe_comment(content, from_mod, to_mod)
        pipe_id = to_snake_id(f"pipe_{from_mod}_{to_mod}")
        pipe = {
            "id": pipe_id,
            "from": from_mod,
            "to": to_mod,
            "callback_type": "pull",
            "comment": pipe_comment,
            "out_link": {"style": out_link[0], "dims": out_link[1]},
            "in_link": {"style": in_link[0], "dims": in_link[1]},
            "fields": fields,
        }
        in_pipes.append(pipe)

    # 合并：输出管道优先（有完整字段），输入管道补充 in_link 和 in_fields
    all_pipes = list(out_pipes)
    for ip in in_pipes:
        match = None
        for ep in all_pipes:
            if ep['from'] == ip['from'] and ep['to'] == ip['to']:
                match = ep
                break
        if match:
            # 用输入侧的 in_link 覆盖
            match['in_link'] = ip['in_link']
            # 保留输入侧字段类型（consumer 视角的自定义类型名）
            match['in_fields'] = ip['fields']
        else:
            # 仅在 consumer 端扫描到 (没有 producer 端 OUTPUT_PARAMS 定义)
            # 此时 fields 就是 consumer 视角的，同时设 in_fields 保持一致
            ip['in_fields'] = ip['fields']
            all_pipes.append(ip)

    module = {
        "id": to_snake_id(module_name),
        "name": module_name,
        "layer": layer,
        "source_file": rel_path if not rel_path.endswith('.h') else "",
        "comment": brief,
        "types": types,
    }

    return module, all_pipes


def scan_source_files(proj_dir, search_dirs=None):
    """扫描 .c 文件中的 MODULE_SKELETON，返回 {name: source_file}"""
    result = {}
    c_files = _glob_files(proj_dir, '**/*.c', search_dirs)
    for c_file in c_files:
        # 跳过 ProjectsOld
        if 'ProjectsOld' in c_file:
            continue
        content = read_file(c_file)
        if not content:
            continue
        for m in RE_MODULE_SKELETON.finditer(content):
            name = m.group(1)
            rel = os.path.relpath(c_file, proj_dir).replace('\\', '/')
            result[name] = rel
    return result


def _glob_files(proj_dir, pattern, search_dirs=None):
    """按 search_dirs 列表搜索文件，不传则全局递归

    Args:
        proj_dir: 项目根目录
        pattern: glob 模式 ('**/*.c' 等)
        search_dirs: 子目录列表如 ['src/RX32G410_FW_HAL_V1.3N/Projects', 'app']
                     不传则 proj_dir 全局递归

    Returns:
        绝对路径列表
    """
    results = []
    if not search_dirs:
        # 全局递归
        results = glob.glob(os.path.join(proj_dir, pattern), recursive=True)
    else:
        for sd in search_dirs:
            full = os.path.join(proj_dir, sd)
            if os.path.isdir(full):
                pat = os.path.join(full, pattern)
                results.extend(glob.glob(pat, recursive=True))
    return results


def scan_data_switcher(proj_dir, search_dirs=None):
    """从 data_switcher.c 提取 slot_order（只从 enum 定义中提取）"""
    sw_files = _glob_files(proj_dir, '**/data_switcher.c', search_dirs)
    sw_files = [f for f in sw_files if 'ProjectsOld' not in f]
    if not sw_files:
        return []
    content = read_file(sw_files[0])
    if not content:
        return []
    slots = []
    for m in RE_SLOT_ENUM.finditer(content):
        name = m.group(1)
        if name != 'COUNT':
            slots.append(name)
    return slots


def scan_all_modules(proj_dir, search_dirs=None):
    """扫描所有 _io.h 文件，收集模块和管道"""
    all_modules = {}
    all_pipes = []

    io_files = _glob_files(proj_dir, '**/*_io.h', search_dirs)
    io_files = [f for f in io_files if 'ProjectsOld' not in f]

    for io_path in sorted(io_files):
        result = scan_io_file(io_path, proj_dir)
        if result is None:
            continue
        module, pipes = result
        name = module['name']
        mod_id = module['id']

        # 去重：按 snake_case id 比较（处理 APP_POWER vs AppPower 差异）
        existing_key = None
        for k, m in all_modules.items():
            if m['id'] == mod_id:
                existing_key = k
                break

        if existing_key:
            # 同名碰撞 — 优先保留 PascalCase（替换 ALL_CAPS 版本）
            if existing_key == existing_key.upper() and name != name.upper():
                del all_modules[existing_key]
                all_modules[name] = module
            # 其他情况：保留第一个
            for p in pipes:
                _merge_pipe(all_pipes, p)
            continue

        all_modules[name] = module
        for p in pipes:
            _merge_pipe(all_pipes, p)

    return all_modules, all_pipes


def _merge_pipe(all_pipes, new_pipe):
    """管道去重 + 合并两端信息

    - 同 (from, to) 管道只保留一份
    - fields 来自 producer 端 (OUTPUT_PARAMS 扫描结果)
    - in_fields 来自 consumer 端 (INPUT_PARAMS 扫描结果，自定义类型用本模块命名)
    - out_link / in_link 各自从对应端获取
    """
    key = (new_pipe['from'], new_pipe['to'])
    for x in all_pipes:
        if x['from'] == key[0] and x['to'] == key[1]:
            if 'in_fields' in new_pipe and 'in_fields' not in x:
                x['in_fields'] = new_pipe['in_fields']
            if 'in_link' in new_pipe:
                # 合并 in_link: 优先保留新的 dims 字段，补缺失字段
                for k, v in new_pipe['in_link'].items():
                    if v != 0 and v != '':
                        x['in_link'][k] = v
                    elif k not in x['in_link']:
                        x['in_link'][k] = v
            if 'fields' in new_pipe and 'fields' not in x:
                x['fields'] = new_pipe['fields']
            if 'out_link' in new_pipe:
                for k, v in new_pipe['out_link'].items():
                    if v != 0 and v != '':
                        x['out_link'][k] = v
                    elif k not in x['out_link']:
                        x['out_link'][k] = v
            if 'alias_of' in new_pipe:
                x['alias_of'] = new_pipe['alias_of']
            return
    all_pipes.append(dict(new_pipe))


def scan_project(proj_dir, keil_path=None, proj_name=None,
                 search_dirs=None):
    """主入口：扫描项目并返回 project.json 数据结构

    Args:
        proj_dir: 项目根目录
        search_dirs: 搜索子目录列表，如
            ['src/RX32G410_FW_HAL_V1.3N/Projects', 'app', 'base_class']
            不传则递归 proj_dir 全部
    """
    proj_dir = os.path.abspath(proj_dir)

    # 1. 扫描 _io.h 获取模块和管道
    modules_dict, pipes = scan_all_modules(proj_dir, search_dirs)

    # 2. 扫描 MODULE_SKELETON 获取 source_file
    source_map = scan_source_files(proj_dir, search_dirs)

    # 3. 扫描 data_switcher.c 获取 slot_order
    slot_order = scan_data_switcher(proj_dir, search_dirs)

    # 4. 完善模块信息
    modules_list = []
    for name, mod in modules_dict.items():
        if name in source_map:
            mod['source_file'] = source_map[name]
        modules_list.append(mod)

    # 5. 按 slot_order 排序模块
    ordered = []
    remaining = list(modules_list)
    for slot_name in slot_order:
        for i, m in enumerate(remaining):
            if m['name'] == slot_name:
                ordered.append(m)
                remaining.pop(i)
                break
    ordered.extend(remaining)
    modules_list = ordered

    # 6. 管道排序：按文件出现顺序保留（与实际 io.h 顺序一致）
    # 不再按 slot_order 排序，保证生成的 io.h 与原文件顺序一致
    # pipes 已是文件出现顺序

    # 7. 构建输出
    if proj_name is None:
        proj_name = os.path.basename(proj_dir)

    project = {
        "schema_version": "2.3",
        "project": {
            "name": proj_name,
            "paths": {
                "io_dir": "include",
                "core_dir": "core",
                "app_dir": "app",
                "base_class_dir": "base_class",
                "proto_dir": "proto",
            },
            "std_module_path": "core/std_module.h",
            "pack": 4,
            "comment_guard": True,
            "output_root": proj_dir,
        },
        "modules": modules_list,
        "pipes": pipes,
        "slot_order": slot_order,
    }

    return project


def main():
    import sys
    if len(sys.argv) < 2:
        print("用法: python scan_project.py <项目路径> [--name <项目名称>] [--keil <uvprojx路径>]")
        print("")
        print("双向维护数据源:")
        print("  src/JSON/        — project.json (接口权威数据源)")
        print("  src/include_io/  — *_io.h (模块接口定义)")
        print("")
        print("扫描: 从 include_io/ 解析 → 写入 JSON/project.json")
        print("生成: 从 JSON/project.json 解析 → 重写 include_io/_io.h")
        sys.exit(1)

    proj_dir = sys.argv[1]
    proj_name = None
    keil_path = None
    if '--name' in sys.argv:
        idx = sys.argv.index('--name')
        proj_name = sys.argv[idx + 1]
    if '--keil' in sys.argv:
        idx = sys.argv.index('--keil')
        keil_path = sys.argv[idx + 1]

    data = scan_project(proj_dir, keil_path, proj_name)

    # 固定双向维护路径: src/JSON/project.json  ↔  src/include_io/*_io.h
    out_path = os.path.join(proj_dir, 'JSON', 'project.json')

    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    with open(out_path, 'w', encoding='utf-8') as f:
        json.dump(data, f, indent=2, ensure_ascii=False)
    print(f"[OK] project.json 已生成: {out_path}")
    print(f"     模块数: {len(data['modules'])}")
    print(f"     管道数: {len(data['pipes'])}")
    for m in data['modules']:
        print(f"       {m['name']:20s} layer={m['layer']:12s} src={m['source_file']}")
    for p in data['pipes']:
        n_fields = len(p.get('fields') or p.get('in_fields') or [])
        print(f"       {p['from']:12s} → {p['to']:12s}  fields={n_fields}  out_link={p['out_link']['style']}")


if __name__ == "__main__":
    main()
