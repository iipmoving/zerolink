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


def _resolve_io_path(module_name: str, project: dict) -> str:
    """计算 io.h 在 data_switcher.c 中的 #include 路径"""
    io_dir = project.get("paths", {}).get("io_dir", "include")
    return f"{io_dir}/{module_name.lower()}_io.h"


def generate_switcher(modules: list, pipes: list, slot_order: list, project: dict) -> str:
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
    lines.append("/* ===== 模块槽位索引 (使用 SLOT 宏) ===== */")
    lines.append("typedef enum {")
    for i, s in enumerate(slot_order):
        lines.append(f"    SLOT({s}) = {i},")
    lines.append("    SLOT(COUNT)")
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
        ctype = c_pipes[0].get("callback_type", "pull")
        producers = [p["from"] for p in c_pipes]

        # 如果有多个 producer，使用第一个作为 INPUT_CALLBACK 名称
        # (一个 consumer 只有一个 InputCallback 函数)
        lines.append("")
        if len(producers) == 1:
            lines.append(f"INPUT_CALLBACK({producers[0]}, {consumer_name})")
        else:
            # 多 producer: 使用第一个作为主名
            lines.append(f"INPUT_CALLBACK({producers[0]}, {consumer_name})")
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
                        lines.append(f"        in->{from_name}_params.params[0].{fname} = out->{consumer_name}_params.params[0].{fname};")
                    lines.append(f"        in->{from_name}_params.status |= ST_NEW;")
                else:
                    lines.append("        /* TODO: 填写逐字段搬运 */")
                lines.append("    }")

        lines.append("}")
        lines.append("")

    # Switcher_Run_Slot[N] — 独立执行链
    lines.append("/* ================================================================")
    lines.append(" * Switcher_Run_Slot[N] — 独立执行链")
    lines.append(" *")
    lines.append(" * 每个函数对应一个模块的 DoWork，用户自由组合调用顺序：")
    lines.append(" *")
    lines.append(" *   // 示例: 自定义 1ms 控制流水线")
    lines.append(" *   void MyControlLoop(void) {")
    for i, s in enumerate(slot_order):
        lines.append(f" *       Switcher_Run_Slot{i}();  // {s}")
    lines.append(" *   }")
    lines.append(" *")
    lines.append(" * 或按需只执行部分链路：")
    lines.append(" *   if (adc_ready) Switcher_Run_Slot0();")
    lines.append(" *   if (power_on)  Switcher_Run_Slot3();")
    lines.append(" * ================================================================ */")
    lines.append("")
    for i, s in enumerate(slot_order):
        lines.append(f"void Switcher_Run_Slot{i}(void) {{ s_slot[SLOT_{s}].pDoWork(); }}")
    lines.append("")

    # Switcher_Run_All — 批量执行
    lines.append("/* ================================================================")
    lines.append(" * Switcher_Run_All — 一次执行全部模块 (批量模式)")
    lines.append(" *")
    lines.append(" * 用于控制周期长的场景 (如 20ms 周期), 一次跑完所有槽。")
    lines.append(" * 相当于按序调用全部 Switcher_Run_Slot[N]。")
    lines.append(" * ================================================================ */")
    lines.append("void Switcher_Run_All(void)")
    lines.append("{")
    for i in range(len(slot_order)):
        lines.append(f"    Switcher_Run_Slot{i}();")
    lines.append("}")
    lines.append("")

    return "\n".join(lines)
