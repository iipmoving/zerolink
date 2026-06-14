#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
keil_parser.py — KEIL .uvprojx 项目解析器

功能:
  - 解析 .uvprojx XML → 提取文件列表 (Group + FilePath)
  - 按文件夹白名单过滤 (只保留在指定目录下的文件)
  - 自动排除 HAL/库/启动文件
  - 扫描 .c 文件生成 AI 分析辅助文档 (函数签名/结构体/头文件依赖)
"""

import xml.etree.ElementTree as ET
import os
import re


# ================================================================
# 默认排除的 Group 名 (KEIL 工程分组)
# ================================================================
EXCLUDED_GROUPS = {
    "FWLib", "Startup", "mcu_driver", "lib", "CMSIS",
    "Device", "StdLib", "HAL_Driver", "BSP",
    "Drivers\\HAL", "Middlewares",
}

# 默认排除的文件名/路径模式
EXCLUDED_PATTERNS = [
    r"startup_.*\.s$",
    r"system_.*\.c$",
    r".*\.lib$",
    r".*\.a$",
]

# 默认排除的路径片段 (大小写不敏感, 按路径组件匹配, 不是子串)
EXCLUDED_PATH_PARTS = [
    "vendor", "FWLib", "CMSIS", "StdLib",
]


def parse_uvprojx(uvprojx_path: str) -> list[dict]:
    """
    解析 .uvprojx 文件，返回文件列表。

    XML 结构:
      Targets > Target > Groups > Group
        - GroupName
        - Files > File > FileName, FileType, FilePath
    """
    tree = ET.parse(uvprojx_path)
    root = tree.getroot()
    proj_dir = os.path.dirname(os.path.abspath(uvprojx_path))

    files = []

    for group_elem in root.iter("Group"):
        # 提取 GroupName
        gn = group_elem.find("GroupName")
        group_name = gn.text.strip() if gn is not None and gn.text else ""

        # 提取 Files > File
        files_elem = group_elem.find("Files")
        if files_elem is None:
            continue

        for file_elem in files_elem.iter("File"):
            fn = file_elem.find("FileName")
            fp = file_elem.find("FilePath")

            file_name = fn.text.strip() if fn is not None and fn.text else ""
            file_path = fp.text.strip() if fp is not None and fp.text else ""

            if not file_name or not file_path:
                continue

            # 只收 .c / .h / .cpp (排除 .lib)
            ext = os.path.splitext(file_name)[1].lower()
            if ext not in (".c", ".h", ".cpp"):
                continue

            # 转为绝对路径
            abs_path = os.path.normpath(os.path.join(proj_dir, file_path))

            rel_path = os.path.relpath(abs_path, proj_dir).replace("\\", "/")

            files.append({
                "group": group_name,
                "file_name": file_name,
                "file_path": file_path,
                "abs_path": abs_path,
                "rel_path": rel_path,
            })

    return files


def filter_files(
    files: list[dict],
    folder_whitelist: list[str] = None,
    exclude_groups: set = None,
) -> list[dict]:
    """
    过滤文件列表:
      - 排除指定 Group (如 Drivers\\HAL, CMSIS, Middlewares)
      - 排除 startup_*.s / system_*.c / *.lib
      - 排除路径含 vendor/FWLib/CMSIS/StdLib 的
      - 如果传了 folder_whitelist, 只保留路径匹配白名单的文件 (GUI 重构工作流使用)
    """
    if exclude_groups is None:
        exclude_groups = EXCLUDED_GROUPS

    result = []
    for f in files:
        # 按 Group 排除 (大小写不敏感)
        gn = f["group"].lower().replace("\\", "/")
        if any(g.lower().replace("\\", "/") == gn for g in exclude_groups):
            continue

        # 按文件名/路径模式排除
        fname = f["file_name"]
        if any(re.match(p, fname, re.IGNORECASE) for p in EXCLUDED_PATTERNS):
            continue

        # 按路径组件排除 (vendor, FWLib, CMSIS, StdLib — 按组件匹配, 不是子串)
        if _path_contains_excluded(f["abs_path"], EXCLUDED_PATH_PARTS):
            continue

        # 按文件夹白名单筛选 — 只在传了 whitelist 时才启用
        if folder_whitelist:
            if not _path_matches_whitelist(f["rel_path"], folder_whitelist, f["group"]):
                continue

        result.append(f)

    return result


def _path_contains_excluded(abs_path: str, excluded_parts: list) -> bool:
    """检查绝对路径的**组件**是否匹配排除关键词 (不是子串匹配, 避免项目根目录误伤)"""
    parts = abs_path.replace("\\", "/").lower().split("/")
    ex_lower = [e.lower() for e in excluded_parts]
    for part in parts:
        for ex in ex_lower:
            if ex == part:
                return True
    return False


def _path_matches_whitelist(rel_path: str, whitelist: list[str], group_name: str = "") -> bool:
    """检查文件是否匹配白名单 (大小写不敏感, 匹配路径组件 或 Group 名 或 文件前缀)"""
    wl_lower = [w.lower() for w in whitelist]

    # 1. 匹配 KEIL Group 名 (大小写不敏感, 子串匹配)
    if group_name:
        gn_lower = group_name.lower().replace("\\", "/")
        for wl in wl_lower:
            if wl in gn_lower or gn_lower in wl:
                return True

    # 2. 匹配 rel_path 的路径组件 (大小写不敏感)
    parts = rel_path.replace("\\", "/").lower().split("/")
    for part in parts:
        if part in wl_lower:
            return True

    # 3. 匹配文件名下划线前缀 (如 app_power.c → app)
    fname = os.path.basename(rel_path)
    base = os.path.splitext(fname)[0].lower()
    if "_" in base:
        prefix = base.split("_")[0]
        if prefix in wl_lower:
            return True
    return False


def scan_source_files(files: list[dict]) -> dict:
    """
    扫描选中的 .c 文件，提取函数签名/结构体定义/头文件依赖，
    生成 AI 分析辅助文档。
    """
    scan_data = {"files": []}

    for f in files:
        if not f["file_name"].endswith(".c"):
            continue
        if not os.path.isfile(f["abs_path"]):
            continue

        try:
            with open(f["abs_path"], "r", encoding="utf-8") as fh:
                content = fh.read()
        except (UnicodeDecodeError, IOError):
            continue

        info = _extract_file_info(f, content)
        scan_data["files"].append(info)

    return scan_data


def _extract_file_info(file_entry: dict, content: str) -> dict:
    """提取单个 .c 文件的结构化信息"""
    lines = content.split("\n")

    includes = []
    functions = []
    structs = []
    defines = []

    for line in lines:
        stripped = line.strip()
        if not stripped:
            continue

        # #include
        if stripped.startswith("#include"):
            includes.append(stripped)

        # 函数声明/定义 (粗略匹配)
        m = re.match(
            r'^(?:static\s+)?(?:void|int|uint\d+_t|float|char|bool|uint8_t|uint16_t|uint32_t|'
            r'uint64_t|int\d+_t|int8_t|int16_t|int32_t|int64_t)'
            r'\s+\*?\s*([A-Za-z_]\w*)\s*\(', stripped)
        if m:
            func_name = m.group(1)
            has_body = "{" in stripped
            functions.append({
                "name": func_name,
                "decl": stripped[:100],
                "has_body": has_body,
            })

        # typedef struct / struct (仅含 { 的行)
        if "typedef struct" in stripped or ("struct" in stripped and "{" in stripped and "}" in stripped):
            structs.append(stripped[:100])

        # #define (非 __ 开头)
        if stripped.startswith("#define") and not stripped.startswith("#define __"):
            parts = stripped.split(None, 2)
            if len(parts) >= 2:
                defines.append({
                    "name": parts[1],
                    "value": parts[2] if len(parts) > 2 else "",
                })

    # 提取文件头注释 @brief
    brief = ""
    for line in lines[:10]:
        m = re.search(r'@brief\s+(.*)', line)
        if m:
            brief = m.group(1).strip()
            break

    return {
        "rel_path": file_entry["rel_path"],
        "group": file_entry["group"],
        "brief": brief,
        "includes": includes[:30],
        "functions": functions[:40],
        "structs": structs[:20],
        "defines": defines[:20],
        "total_lines": len(lines),
    }


def generate_ai_scan_doc(scan_data: dict, output_path: str = None) -> str:
    """
    生成 AI 分析辅助文档 (Markdown)。
    包含每个文件的结构信息，供 AI 推断数据流向。
    """
    lines = []
    lines.append("# 项目源码扫描 — AI 数据流分析辅助文档")
    lines.append("")
    lines.append(f"共扫描 {len(scan_data['files'])} 个 .c 文件")
    lines.append("")
    lines.append("---")
    lines.append("")

    for fi in scan_data["files"]:
        lines.append(f"## {fi['rel_path']}")
        lines.append(f"")
        lines.append(f"- **Group**: {fi['group']}")
        lines.append(f"- **行数**: {fi['total_lines']}")
        if fi["brief"]:
            lines.append(f"- **说明**: {fi['brief']}")
        lines.append("")

        if fi["includes"]:
            lines.append("### #include 依赖")
            lines.append("")
            for inc in fi["includes"]:
                lines.append(f"  - `{inc}`")
            lines.append("")

        if fi["functions"]:
            lines.append("### 函数")
            lines.append("")
            lines.append("| 函数名 | 声明 | 有函数体 |")
            lines.append("|--------|------|----------|")
            for fn in fi["functions"]:
                body = "✓" if fn["has_body"] else " "
                lines.append(f"| {fn['name']} | `{fn['decl']}` | {body} |")
            lines.append("")

        if fi["defines"]:
            lines.append("### #define 常量")
            lines.append("")
            for d in fi["defines"]:
                lines.append(f"  - `{d['name']}` = {d['value']}")
            lines.append("")

        lines.append("---")
        lines.append("")

    doc = "\n".join(lines)

    if output_path:
        os.makedirs(os.path.dirname(output_path) or ".", exist_ok=True)
        with open(output_path, "w", encoding="utf-8") as f:
            f.write(doc)

    return doc


# ================================================================
# CLI 测试入口
# ================================================================
if __name__ == "__main__":
    import sys
    if len(sys.argv) < 2:
        print("Usage: python keil_parser.py <path.uvprojx> [白名单文件夹...]")
        sys.exit(1)

    proj = sys.argv[1]
    whitelist = sys.argv[2:] or ["src"]

    print(f"Parse KEIL project: {proj}")
    all_files = parse_uvprojx(proj)
    print(f"  Total: {len(all_files)} source files")

    filtered = filter_files(all_files, folder_whitelist=whitelist)
    print(f"  After filter (whitelist={whitelist}): {len(filtered)} files")
    for f in filtered:
        print(f"    [{f['group']:12s}] {f['rel_path']}")

    print("\nScanning source code...")
    scan = scan_source_files(filtered)
    print(f"  Scanned {len(scan['files'])} .c files")

    doc = generate_ai_scan_doc(scan)
    print(f"  Generated AI doc: {len(doc)} chars")
    print("\n--- First 30 lines ---")
    for line in doc.split("\n")[:30]:
        print(line)
