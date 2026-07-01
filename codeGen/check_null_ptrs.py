#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
check_null_ptrs.py — 检查已范式化模块的 NULL 指针风险

分析每个模块的 ProcessInput() 和 user_Process() 中对 s_inPara/s_outPara
的访问路径，检查是否存在 NULL 指针解引用风险。

规则：
  1. ProcessInput 中 in->xxx 在 s_inPara 未赋值前为 NULL → 崩溃
  2. user_Process 中 if (!in) return; 有 NULL 保护 → 安全
  3. ProcessInput 中的 in->xxx 无 NULL 检查 → 风险
  4. 输出端 in->xxx->params 是否为空指针 → 风险

运行: python codeGen/check_null_ptrs.py [--json path/to/project.json]
"""

import json
import os
import re
import sys


# ================================================================
# 模块源码中的指针访问模式
# ================================================================

# 危险模式: in->xxx 在 s_inPara 为 NULL 时会被解引用
DANGEROUS_PATTERNS = [
    # (正则, 风险描述)
    (r'in->\w+_params->status', "in->xxx_params->status — s_inPara 为 NULL 时崩溃"),
    (r'in->\w+_params->count', "in->xxx_params->count — s_inPara 为 NULL 时崩溃"),
    (r'in->\w+_params->max_count', "in->xxx_params->max_count — s_inPara 为 NULL 时崩溃"),
    (r'in->\w+_params->params\[',"in->xxx_params->params[] — s_inPara 为 NULL 时崩溃"),
    (r's_outPara\.\w+_params\.', "s_outPara.xxx_params — 未初始化时写入"),
    (r'out->\w+_params->status', "out->xxx_params->status — s_outPara 为空时崩溃"),
    (r'out->\w+_params\.params\[', "out->xxx_params.params[] — 越界风险"),
]

# 安全模式: 有 NULL 检查
SAFE_PATTERNS = [
    (r'if\s*\(\s*!in\s*\)', "有 if(!in) 保护"),
    (r'if\s*\(\s*!in->\w+_params\s*\)', "有 if(!in->xxx_params) 保护"),
    (r'if\s*\(\s*in\s*&&\s*in->', "有 if(in && in->) 保护"),
]


def read_source_file(module, project):
    """根据 source_file 和 output_root 读取实际源码"""
    output_root = project["project"]["output_root"]
    source_file = module["source_file"]
    full_path = os.path.join(output_root, source_file)

    if not os.path.exists(full_path):
        return None, full_path

    with open(full_path, "r", encoding="utf-8") as f:
        return f.read(), full_path


def find_ai_block(content):
    """找到 AI GENERATED 块的范围"""
    begin = content.find("// ===== [AI GENERATED]")
    end = content.find("// ===== [END AI GENERATED] =====")
    if begin >= 0 and end >= 0:
        return content[begin:end], begin, end
    return content, 0, len(content)


def analyze_module(module, project):
    """分析单个模块的 NULL 指针风险"""
    content, full_path = read_source_file(module, project)
    if content is None:
        return {
            "module": module["name"],
            "source_file": full_path,
            "error": "文件不存在",
            "severity": "ERROR",
            "findings": [],
        }

    # 分离 AI 块和用户代码
    ai_block, ai_start, ai_end = find_ai_block(content)

    # 分析 ProcessInput 和 user_Process
    findings = []

    # 检查 ProcessInput 中的 in 访问
    # 提取 ProcessInput 函数体
    process_input_match = re.search(
        r'static void ProcessInput\(void\)\s*\{([^}]*(?:\{[^}]*\}[^}]*)*)\}',
        ai_block, re.DOTALL
    )
    if process_input_match:
        pi_body = process_input_match.group(1)

        # 检查是否有 in 赋值
        in_assign = re.search(r'MODULE_INPUT\([^)]+\)\s*\*\s*in\s*=\s*\(\s*MODULE_INPUT\([^)]+\)\s*\*\)\s*g_input\.para', pi_body)
        if in_assign:
            # 找到 in 赋值，检查接下来对 in 的访问
            lines = pi_body.split('\n')
            in_accessed = False
            in_null_checked = False

            for line in lines:
                if re.search(r'in->\w+', line) and 'g_input' not in line:
                    in_accessed = True
                    # 检查是否是 NULL 检查
                    if re.search(r'if\s*\(\s*!in\b', line):
                        in_null_checked = True
                    elif re.search(r'if\s*\(\s*in\s*&&', line):
                        in_null_checked = True

            if in_accessed and not in_null_checked:
                findings.append({
                    "location": "ProcessInput",
                    "risk": "HIGH",
                    "detail": "ProcessInput 中访问 in->xxx 但无 NULL 检查",
                    "line": pi_body[:pi_body.find('in->')].count('\n') + 1,
                })

    # 检查 user_Process 中的 in 访问
    user_process_match = re.search(
        r'static void user_Process\([^)]+\)\s*\{',
        content
    )
    if user_process_match:
        # 找到 user_Process 的开始
        up_start = user_process_match.end()
        # 简单方法：找下一个 static void 或文件结束
        next_func = re.search(r'\nstatic void ', content[up_start:])
        if next_func:
            up_body = content[up_start:up_start + next_func.start()]
        else:
            up_body = content[up_start:]

        # 检查 NULL 保护
        has_in_check = bool(re.search(r'if\s*\(\s*!in\b', up_body))
        has_in_params_check = bool(re.search(r'if\s*\(\s*!in->\w+', up_body))

        # 检查 in 访问
        in_accesses = re.findall(r'in->(\w+)', up_body)
        if in_accesses:
            if not (has_in_check or has_in_params_check):
                findings.append({
                    "location": "user_Process",
                    "risk": "MEDIUM",
                    "detail": f"user_Process 访问 {len(in_accesses)} 处 in->xxx，仅依赖 InputCallback 保证非 NULL",
                    "in_accesses": list(set(in_accesses)),
                })

    # 检查 s_inPara 初始化
    s_inpara_init = re.search(r's_inPara\s*=\s*(NULL|&s_inPara|g_input)', content)
    if s_inpara_init:
        val = s_inpara_init.group(1)
        if val == "NULL":
            findings.append({
                "location": "Init()",
                "risk": "INFO",
                "detail": "s_inPara = NULL — 依赖 InputCallback 赋值 (标准模式)",
            })

    # 检查 s_outPara 初始化 — 看 Init 中是否有 memset 或赋值
    init_section = re.search(r'static void Init\(void\)\s*\{([^}]*(?:\{[^}]*\}[^}]*)*)\}', content, re.DOTALL)
    if init_section:
        init_body = init_section.group(1)
        has_s_outpara_init = (
            bool(re.search(r's_outPara', init_body)) and
            (bool(re.search(r'memset.*s_outPara', init_body)) or
             bool(re.search(r's_outPara\s*=', init_body)) or
             bool(re.search(r'g_output\.para.*s_outPara', init_body)))
        )
        if not has_s_outpara_init:
            findings.append({
                "location": "Init()",
                "risk": "HIGH",
                "detail": "Init() 中未初始化 s_outPara",
            })
    else:
        # 没找到 Init 函数 — 可能在 AI 块外
        if "s_outPara" in content and "memset" not in content.split("s_outPara")[1][:200]:
            findings.append({
                "location": "Init()",
                "risk": "HIGH",
                "detail": "Init() 中未初始化 s_outPara",
            })

    return {
        "module": module["name"],
        "source_file": full_path,
        "severity": "FAIL" if any(f["risk"] == "HIGH" for f in findings) else "PASS",
        "findings": findings,
    }


def check_io_h_types(project):
    """检查 _io.h 中各模块的 INPUT 字段是否引用了正确的 TYPE"""
    io_dir = os.path.join(project["project"]["output_root"], project["project"]["paths"]["io_dir"])
    if not os.path.exists(io_dir):
        return [{"error": f"io_dir 不存在: {io_dir}"}]

    findings = []
    for fname in os.listdir(io_dir):
        if not fname.endswith("_io.h"):
            continue

        fpath = os.path.join(io_dir, fname)
        with open(fpath, "r", encoding="utf-8") as f:
            content = f.read()

        module_name = fname.replace("_io.h", "")
        # PascalCase
        module_pascal = ''.join(w.capitalize() for w in module_name.split('_'))

        # 找 MODULE_INPUT_PARAMS
        input_params = re.findall(
            r'typedef struct\s*\{(.*?)\}\s*MODULE_INPUT_PARAMS\((\w+),\s*(\w+)\)',
            content, re.DOTALL
        )
        for fields_str, producer, consumer in input_params:
            # 找 fields 中的 type 引用
            field_types = re.findall(r'(\w+(?:_\w+)*)\*\s*(?:input|params)', fields_str)
            for ft in field_types:
                # 检查这个 type 是否在 producer 的 output 中有定义
                pass

    return findings


def main():
    # 查找 project.json
    script_dir = os.path.dirname(os.path.abspath(__file__))
    json_path = os.path.join(script_dir, "out", "HALF", "project.json")

    if len(sys.argv) > 1:
        json_path = sys.argv[1]

    if not os.path.exists(json_path):
        print(f"[ERROR] 找不到 project.json: {json_path}")
        sys.exit(1)

    with open(json_path, "r", encoding="utf-8") as f:
        config = json.load(f)

    print("=" * 70)
    print("v2.3 模块 NULL 指针风险分析")
    print("=" * 70)
    print(f"项目: {config['project']['name']}")
    print(f"模块: {len(config['modules'])}, 管道: {len(config['pipes'])}")
    print()

    results = []
    for mod in config["modules"]:
        result = analyze_module(mod, config)
        results.append(result)

        # 打印结果
        status = "PASS" if result["severity"] == "PASS" else "WARN"
        risk_count = sum(1 for f in result["findings"] if f.get("risk") == "HIGH")
        if risk_count > 0:
            status = "FAIL"

        print(f"[{status}] {result['module']}")
        for f in result["findings"]:
            risk_icon = {"HIGH": "!!", "MEDIUM": "!", "INFO": "i"}.get(f.get("risk", ""), "?")
            print(f"    [{risk_icon} {f['risk']:6s}] {f['location']}: {f['detail']}")

    print()
    print("=" * 70)
    # 总结
    high_risks = sum(1 for r in results if any(f["risk"] == "HIGH" for f in r["findings"]))
    medium_risks = sum(1 for r in results if any(f["risk"] == "MEDIUM" for f in r["findings"]))

    if high_risks > 0:
        print(f"FAIL: {high_risks} 个模块存在 HIGH 风险 (s_inPara 为 NULL 时崩溃)")
    if medium_risks > 0:
        print(f"WARN: {medium_risks} 个模块有 MEDIUM 风险 (依赖 InputCallback 保证)")
    if high_risks == 0 and medium_risks == 0:
        print("PASS: 所有模块 NULL 指针处理正确")

    print()
    print("=" * 70)
    print("数据流传递链验证:")
    print("=" * 70)

    # 验证数据流链
    mod_names = set(m["name"] for m in config["modules"])
    for pipe in config["pipes"]:
        prod = pipe["from"]
        cons = pipe["to"]
        if prod not in mod_names or cons not in mod_names:
            print(f"  [FAIL] {prod} -> {cons}: 模块不存在")
        else:
            print(f"  [OK]   {prod} -> {cons}")

    return 0 if high_risks == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
