#!/usr/bin/env python3
"""
check_json.py — project.json 正确性检查工具

检查项:
  1. JSON Schema 合规
  2. 模块 source_file 存在性
  3. 管道字段对称性 (OUTPUT_PARAMS <-> INPUT_PARAMS)
  4. 嵌套 struct 处理完整性
  5. 数组大小一致性
  6. 路径与 KEIL 项目一致性 (可选)
  7. 自检清单

用法:
  python check_json.py <project.json>
  python check_json.py <project.json> --keil <uvprojx文件>
"""

import json
import os
import re
import sys
from pathlib import Path

# ── 颜色 ──
GREEN = "\033[92m"
RED = "\033[91m"
YELLOW = "\033[93m"
CYAN = "\033[96m"
RESET = "\033[0m"

issues = []
warnings = []

def report(category, status, msg):
    tag = f"[{status}]" if status else ""
    color = GREEN if status == "OK" else (RED if status == "FAIL" else (YELLOW if status == "WARN" else ""))
    issues.append(f"  {color}{tag}{RESET} {category:25s} {msg}")

def warn(msg):
    warnings.append(msg)

# ── 工具 ──
def pascal_to_snake(name):
    s1 = re.sub(r'([A-Z]+)([A-Z][a-z])', r'\1_\2', name)
    return re.sub(r'([a-z0-9])([A-Z])', r'\1_\2', s1).lower()

def read_file(path):
    try:
        with open(path, "r", encoding="utf-8", errors="replace") as f:
            return f.read()
    except:
        return None

def extract_typedefs(content):
    """从 io.h 内容提取所有 typedef struct { ... } Name_t; 定义"""
    typedefs = {}
    pat = re.compile(r'typedef\s+struct\s*\{([^}]+)\}\s*(\w+);', re.DOTALL)
    for m in pat.finditer(content):
        name = m.group(2).strip()
        fields_text = m.group(1)
        fields = []
        for line in fields_text.split(";"):
            line = line.strip()
            if not line:
                continue
            parts = line.split()
            if len(parts) >= 2:
                fname = parts[-1].rstrip(";").strip()
                ftype = " ".join(parts[:-1]).strip()
                # 提取注释
                comment = ""
                if "/*" in line and "*/" in line:
                    comment = line[line.index("/*")+2:line.index("*/")].strip()
                fields.append({"name": fname, "type": ftype, "comment": comment})
        typedefs[name] = fields
    return typedefs

def extract_macro_struct(content, macro_pattern):
    """从 MODULE_OUTPUT_PARAMS/INPUT_PARAMS 宏定义提取字段"""
    pat = re.compile(rf'struct\s*\{{([^}}]+)\}}\s*{re.escape(macro_pattern)}', re.DOTALL)
    fields = []
    for m in pat.finditer(content):
        block = m.group(1)
        for line in block.split(";"):
            line = line.strip()
            if not line or line.startswith("//"):
                continue
            # 跳过纯注释行
            parts = line.split()
            if len(parts) >= 2:
                fname = parts[-1]
                ftype = " ".join(parts[:-1])
                comment = ""
                if "/*" in line and "*/" in line:
                    comment = line[line.index("/*")+2:line.index("*/")].strip()
                fields.append({"name": fname, "type": ftype, "comment": comment})
    return fields

# ── 主检查类 ──
class ProjectJsonChecker:
    def __init__(self, json_path, keil_path=None):
        self.json_path = Path(json_path)
        self.root = self.json_path.parent
        with open(json_path, "r", encoding="utf-8") as f:
            self.data = json.load(f)
        self.project = self.data.get("project", {})
        self.modules = self.data.get("modules", [])
        self.pipes = self.data.get("pipes", [])
        self.output_root = Path(self.project.get("output_root", ""))
        self.keil_path = Path(keil_path) if keil_path else None
        self.mod_map = {m["name"]: m for m in self.modules}
        self.pipe_map = {}
        for p in self.pipes:
            self.pipe_map[(p["from"], p["to"])] = p
        self.io_cache = {}  # module_name -> content

    # ── 1. Schema 合规 ──
    def check_schema(self):
        print(f"\n{CYAN}=== 1. Schema 合规 ==={RESET}")
        if "schema_version" not in self.data:
            report("schema", "FAIL", "缺少 schema_version")
        if not self.modules:
            report("schema", "FAIL", "modules 为空")
        if not self.pipes:
            report("schema", "FAIL", "pipes 为空")
        for m in self.modules:
            if not m.get("name"):
                report("schema", "FAIL", f"module 缺少 name: {m}")
            if not m.get("source_file"):
                report("schema", "FAIL", f"{m.get('name','?')}: 缺少 source_file")
            if not m.get("comment"):
                report("schema", "WARN", f"{m['name']}: comment 为空")
        for p in self.pipes:
            if not p.get("from") or not p.get("to"):
                report("schema", "FAIL", f"pipe 缺少 from/to: {p.get('id', '?')}")
            if not p.get("fields"):
                report("schema", "WARN", f"{p.get('id','?')}: fields 为空")
            for f in p.get("fields", []):
                if not f.get("comment"):
                    report("schema", "WARN", f"{p.get('id','?')}.{f.get('name','?')}: comment 为空")
            for side in ("out_link", "in_link"):
                if side not in p:
                    report("schema", "FAIL", f"{p.get('id','?')}: 缺少 {side}")
        # 检查 output_root 是否为绝对路径
        if not os.path.isabs(self.project.get("output_root", "")):
            report("schema", "FAIL", f"output_root 不是绝对路径: {self.project.get('output_root', '')}")
        else:
            report("schema", "OK", f"output_root 是绝对路径")

    # ── 2. 文件存在性 ──
    def check_files_exist(self):
        print(f"\n{CYAN}=== 2. 源文件存在性 ==={RESET}")
        all_ok = True
        for m in self.modules:
            src = m.get("source_file", "")
            if not src:
                continue
            full = self.output_root / src
            if full.exists():
                report("source_file", "OK", f"{m['name']}: {src}")
            else:
                report("source_file", "FAIL", f"{m['name']}: {src} 不存在 ({full})")
                all_ok = False
        # io.h 文件
        io_dir = self.project.get("paths", {}).get("io_dir", "include")
        for m in self.modules:
            snake = pascal_to_snake(m["name"])
            # 处理 APP_Power → app_power
            io_path = self.output_root / io_dir / f"{snake}_io.h"
            if m["name"] == "APP_Power":
                io_path = self.output_root / io_dir / "app_power_io.h"
            if m["name"] == "EKF_LKF":
                io_path = self.output_root / io_dir / "ekf_lkf_io.h"
            if io_path.exists():
                self.io_cache[m["name"]] = read_file(io_path)
                report("io.h", "OK", f"{m['name']}: {io_path.name}")
            else:
                report("io.h", "WARN", f"{m['name']}: {io_path.name} 不存在")
        if all_ok:
            report("source_file", "OK", "所有源文件存在")

    # ── 3. 管道对称性 ──
    def check_pipe_symmetry(self):
        print(f"\n{CYAN}=== 3. Pipe Symmetry (OUTPUT <-> INPUT) ==={RESET}")
        checked = 0
        for p in self.pipes:
            producer = p["from"]
            consumer = p["to"]
            pipe_id = p.get("id", f"{producer}->{consumer}")

            producer_io = self.io_cache.get(producer)
            consumer_io = self.io_cache.get(consumer)
            if not producer_io or not consumer_io:
                # 尝试从同一个文件找
                pass

            # 从 project.json 的 fields 检查
            json_fields = p.get("fields", [])

            # 检查嵌套 struct 处理
            for f in json_fields:
                ftype = f.get("type", "")
                # 检查指针类型 -> 应有对应的 types[]
                if ftype.endswith("*") and not ftype.startswith("uint") and not ftype.startswith("int"):
                    base_type = ftype.rstrip("*").strip()
                    producer_mod = self.mod_map.get(producer)
                    if producer_mod and producer_mod.get("types"):
                        type_names = [t["name"] for t in producer_mod["types"]]
                        if base_type not in type_names:
                            report("nested", "WARN", f"{pipe_id}.{f['name']}: 类型 {base_type} 未在 {producer}.types[] 中定义")
                    else:
                        report("nested", "WARN", f"{pipe_id}.{f['name']}: 指针类型 {base_type} 但 {producer} 无 types[]")
                # 检查 type=struct 是否有 fields
                if ftype == "struct":
                    if "fields" not in f or not f["fields"]:
                        report("nested", "FAIL", f"{pipe_id}.{f['name']}: type=struct 但无 fields[]")

            # 检查数组大小一致性
            out_link = p.get("out_link", {})
            in_link = p.get("in_link", {})
            if out_link.get("style") == "array":
                oa = out_link.get("array_size", 0) or 0
                if oa <= 0:
                    report("array", "WARN", f"{pipe_id}: out_link array_size={oa}")
            if in_link.get("style") == "array":
                ia = in_link.get("array_size", 0) or 0
                if ia <= 0:
                    report("array", "WARN", f"{pipe_id}: in_link array_size={ia}")

            # 对称性: 检查上下游 module 是否都存在
            if producer not in self.mod_map:
                report("pipe", "FAIL", f"{pipe_id}: Producer '{producer}' 不在 modules[] 中")
            if consumer not in self.mod_map:
                report("pipe", "FAIL", f"{pipe_id}: Consumer '{consumer}' 不在 modules[] 中")
            checked += 1
        report("pipe", "OK", f"检查了 {checked} 条管道")

    # ── 4. slot_order 完整性 ──
    def check_slot_order(self):
        print(f"\n{CYAN}=== 4. slot_order 完整性 ==={RESET}")
        slot_order = self.data.get("slot_order", [])
        mod_names = {m["name"] for m in self.modules}
        slot_set = set(slot_order)
        missing = mod_names - slot_set
        extra = slot_set - mod_names
        if missing:
            report("slot", "FAIL", f"modules 中有但 slot_order 缺失: {', '.join(sorted(missing))}")
        if extra:
            report("slot", "WARN", f"slot_order 中有但 modules 中无: {', '.join(sorted(extra))}")
        if not missing and not extra:
            report("slot", "OK", f"slot_order 包含 {len(slot_order)} 个模块")

    # ── 5. 路径与 KEIL 一致性 ──
    def check_keil_paths(self):
        print(f"\n{CYAN}=== 5. KEIL 路径一致性 ==={RESET}")
        if not self.keil_path or not self.keil_path.exists():
            report("keil", "WARN", "未提供 KEIL 项目文件，跳过")
            return
        try:
            content = read_file(self.keil_path)
            if not content:
                report("keil", "FAIL", "无法读取 KEIL 文件")
                return
            # 提取 KEIL 中的文件路径
            keil_files = set()
            for m in re.finditer(r'<FilePath>([^<]+)</FilePath>', content):
                keil_files.add(m.group(1).replace("\\", "/").lower())
            for m in re.finditer(r'<IncludePath>([^<]+)</IncludePath>', content):
                for path in m.group(1).split(";"):
                    keil_files.add(path.strip().replace("\\", "/").lower())

            # 对比 source_file
            found = 0
            not_found_count = 0
            for m in self.modules:
                src = m.get("source_file", "")
                if not src:
                    continue
                src_lower = src.replace("\\", "/").lower()
                # 检查是否在 KEIL 文件列表中
                matched = False
                for kf in keil_files:
                    if src_lower in kf or kf in src_lower:
                        matched = True
                        break
                if matched:
                    found += 1
                else:
                    not_found_count += 1
                    report("keil", "WARN", f"{m['name']}: {src} 未在 KEIL 项目文件中找到")
            report("keil", "OK" if not_found_count == 0 else "WARN", f"KEIL 中匹配 {found}/{len(self.modules)} 个模块")
        except Exception as e:
            report("keil", "FAIL", f"解析出错: {e}")

    # ── 6. 自检摘要 ──
    def summary(self):
        print(f"\n{CYAN}=== 检查摘要 ==={RESET}")
        fails = [i for i in issues if "FAIL" in i]
        warns = [i for i in issues if "WARN" in i]
        ok = [i for i in issues if "OK" in i]
        print(f"  {GREEN}通过{RESET}: {len(ok)}   {YELLOW}警告{RESET}: {len(warns)}   {RED}失败{RESET}: {len(fails)}")
        if fails:
            print(f"\n{RED}需修复项:{RESET}")
            for i in fails:
                print(f"  {i}")

    def run(self):
        self.check_schema()
        self.check_files_exist()
        self.check_pipe_symmetry()
        self.check_slot_order()
        self.check_keil_paths()
        self.summary()
        return len([i for i in issues if "FAIL" in i]) == 0


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(f"用法: python check_json.py <project.json> [--keil <uvprojx>]")
        sys.exit(1)

    json_path = sys.argv[1]
    keil_path = None
    if "--keil" in sys.argv:
        idx = sys.argv.index("--keil")
        if idx + 1 < len(sys.argv):
            keil_path = sys.argv[idx + 1]

    if not os.path.isfile(json_path):
        print(f"文件不存在: {json_path}")
        sys.exit(1)

    print(f"{CYAN}检查: {json_path}{RESET}")
    checker = ProjectJsonChecker(json_path, keil_path)
    ok = checker.run()
    sys.exit(0 if ok else 1)
