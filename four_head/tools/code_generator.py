#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
four_head v2.3 代码自动生成器

输入: 模块清单 + 数据流向表 (JSON/YAML)
输出:
  1. include/xxx_io.h - INPUT/OUTPUT 结构定义
  2. core/data_switcher.c - 槽位枚举、Init、InputCallback
  3. app/xxx.c - 模块骨架（填空题模式）

Usage:
    python code_generator.py --config flow_config.json
"""

import argparse
import json
import os

# ========== 模板定义 ==========

IO_H_TEMPLATE = """/**
 * {io_file} —— {module_name} 输入输出接口定义 (v2.3)
 *
 * 数据流:
{data_flow_doc}
 *
 * Include 权限:
 *   - 仅 {module_lower}.c 和 data_switcher.c 可 include
 *   - 使用全路径: #include "include/{io_file}"
 *
 * 本文件包含 std_module.h，调用者无需再包含
 */
#ifndef {guard_macro}
#define {guard_macro}

#include "../core/std_module.h"
#include <stdint.h>

{output_section}

{input_section}

#endif /* {guard_macro} */
"""

OUTPUT_LINK_TEMPLATE = """
/* ========== OUTPUT ({module_name} 给别的模块提供的数据) ========== */

{output_params}

{output_links}

/* {module_name}_Output — 输出聚合 (v2.3 对称命名: 成员名 = {{Consumer}}_params) */
typedef struct {{
{output_members}
}} MODULE_OUTPUT({module_name});

/* ========== Consumer INPUT_LINK 定义在 Consumer 的 io.h ========== */
{consumer_includes}
"""

INPUT_LINK_TEMPLATE = """
/* ========== INPUT ({module_name} 从别的模块得到的数据) ========== */

{input_params}

{input_links}

/* {module_name}_Input — 输入聚合 (v2.3 对称命名: 成员名 = {{Producer}}_params) */
typedef struct {{
{input_members}
}} MODULE_INPUT({module_name});
"""

DATA_SWITCHER_TEMPLATE = """/**
 * @file    data_switcher.c
 * @brief   Data Switcher — PULL 路由调度器 (v2.3)
 * @layer   core
 *
 * 自动生成，请勿手动修改！
 */
#include "core/std_module.h"
#include "data_switcher.h"

/* IO 接口文件 */
{io_includes}

/* 模块槽位索引 (使用 SLOT 宏) */
typedef enum {{
{slot_enum}
    SLOT(COUNT)
}} SwitcherSlot_t;

static ModuleSlotDef s_slots[SLOT(COUNT)];

void Switcher_Init(void)
{{
{slot_getio}
}}

/* ===== Consumer InputCallback 强符号实现 — 指针直穿 (v2.3) ===== */
{input_callbacks}

/* ===== @OUTPUT_CALLBACK 即时路由 ===== */
{output_callbacks}

void Switcher_Run(void)
{{
{module_calls}
}}
"""

MODULE_C_TEMPLATE = """/**
 * {module_lower}.c —— {module_name} 模块实现 (v2.3)
 *
 * 依赖: include/{module_lower}_io.h
 * 层级: {layer}
 *
 * v2.3 三层结构: PARAMS → LINK → OUTPUT
 * 指针直穿: Consumer 回调不做 memcpy，直接赋指针
 *
 * TODO: 填写以下内容:
 *   1. Init() - 初始化逻辑
 *   2. ProcessInput() - 三段式处理逻辑
 *   3. 业务逻辑函数
 */
#include "include/{module_lower}_io.h"  /* 包含 std_module.h + 三层结构定义 */
#include "{module_lower}.h"
#include <stddef.h>

/* ---- v2.3 三层数据结构 ---- */
/* PARAMS 实例 (纯数据) - TODO: 添加字段初始化 */
{params_instances}

/* LINK 实例 (status + *params) */
{link_instances}

/* OUTPUT 实例 (多LINK指针聚合) - Producer 专用 */
{output_declaration}

/* INPUT 实例 — Consumer 的输入由 Switcher 通过 InputCallback 赋值 */
{input_instance}

MODULE_SKELETON({module_name});

/* ========== 业务逻辑 ========== */

static void ProcessInput(void)
{{
    /* TODO: 三段式处理逻辑 */
    /* 1. 输入段: 检查 status & ST_NEW → 消费 → 清除标志 */
    /* 2. 计算段: 纯逻辑处理 */
    /* 3. 输出段: 写 g_output.para */
}}

/* ========== 初始化 ========== */
static void Init(void)
{{
    /* TODO: 添加初始化逻辑 */
    
    /* v2.3: 初始化三层结构 */
    {params_init}
    
    {links_init}
    
    /* OUTPUT 初始化 — LINK 指针绑定 */
    {output_init}
    
    /* g_input/g_output 绑定 */
{input_bind}
    g_output.para = &s_out;
}}

void {module_lower}_Init(void) {{ Constructor(); }}
MODULE_EXPORT({module_name});

/* ========== 业务函数 ========== */
/* TODO: 添加业务函数 */
"""

# ========== 生成器核心函数 ==========

def generate_io_h(module, config):
    """生成 io.h 文件"""
    module_name = module['name']
    module_lower = module_name.lower()
    io_file = f"{module_lower}_io.h"
    guard_macro = f"{module_lower.upper()}_IO_H"
    
    # 构建数据流文档
    data_flow_doc = ""
    for producer in config['producers'].get(module_name, []):
        data_flow_doc += f" *   INPUT: {producer} → {module_name}\n"
    for consumer in config['consumers'].get(module_name, []):
        data_flow_doc += f" *   OUTPUT: {module_name} → {consumer}\n"
    
    # 生成 OUTPUT 部分
    output_params = ""
    output_links = ""
    output_members = ""
    consumer_includes = ""
    
    for consumer in config['consumers'].get(module_name, []):
        params_name = f"{module_name}_to_{consumer}_Params"
        link_name = f"{module_name}_to_{consumer}_Output_Link"
        
        output_params += f"""/* {params_name} — 输出给 {consumer} 的数据 */
typedef struct {{
    // TODO: 添加输出字段
    uint8_t  res[4];
}} MODULE_OUTPUT_PARAMS({module_name}, {consumer});
"""
        
        output_links += f"""/* {link_name} — 输出管道 */
typedef struct {{
    uint8_t  status;        /* ST_NEW/ST_OUT */
    uint8_t  max_count;     /* 最大数量 */
    uint8_t  count;         /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS({module_name}, {consumer}) *params;
}} MODULE_OUTPUT_LINK({module_name}, {consumer});
"""
        
        output_members += f"    MODULE_OUTPUT_LINK({module_name}, {consumer}) *{consumer}_params;  /* → {consumer} */\n"
        consumer_includes += f"/* {consumer}_INPUT_LINK({module_name}, {consumer}) 定义在 {consumer.lower()}_io.h */\n"
    
    output_section = OUTPUT_LINK_TEMPLATE.format(
        module_name=module_name,
        output_params=output_params,
        output_links=output_links,
        output_members=output_members,
        consumer_includes=consumer_includes
    ) if config['consumers'].get(module_name) else "/* ========== OUTPUT (无) ========== */\n/* 本模块没有输出 */"
    
    # 生成 INPUT 部分
    input_params = ""
    input_links = ""
    input_members = ""
    
    for producer in config['producers'].get(module_name, []):
        params_name = f"{producer}_to_{module_name}_Input_Params"
        link_name = f"{producer}_to_{module_name}_Input_Link"
        
        input_params += f"""/* {params_name} — 从 {producer} 得到的数据 */
typedef struct {{
    // TODO: 添加输入字段（与 {producer} OUTPUT 布局一致）
    uint8_t  res[4];
}} MODULE_INPUT_PARAMS({producer}, {module_name});
"""
        
        input_links += f"""/* {link_name} — 输入管道 (与 {producer} OUTPUT 配对) */
typedef struct {{
    uint8_t  status;
    uint8_t  max_count;
    uint8_t  count;
    uint8_t  res[1];
    MODULE_INPUT_PARAMS({producer}, {module_name}) *params;
}} MODULE_INPUT_LINK({producer}, {module_name});
"""
        
        input_members += f"    MODULE_INPUT_LINK({producer}, {module_name}) *{producer}_params;  /* 从 {producer} 得到 */\n"
    
    input_section = INPUT_LINK_TEMPLATE.format(
        module_name=module_name,
        input_params=input_params,
        input_links=input_links,
        input_members=input_members
    ) if config['producers'].get(module_name) else "/* ========== INPUT (无) ========== */\n/* 本模块没有输入 */"
    
    # 组合完整文件
    content = IO_H_TEMPLATE.format(
        io_file=io_file,
        module_name=module_name,
        module_lower=module_lower,
        data_flow_doc=data_flow_doc,
        guard_macro=guard_macro,
        output_section=output_section,
        input_section=input_section
    )
    
    return content

def generate_data_switcher(config):
    """生成 data_switcher.c 文件"""
    modules = config['modules']
    
    # IO includes
    io_includes = ""
    for module in modules:
        io_includes += f'#include "include/{module["name"].lower()}_io.h"\n'
    
    # Slot enum
    slot_enum = ""
    for i, module in enumerate(modules):
        slot_enum += f"    SLOT({module['name']}) = {i},\n"
    
    # Slot GETIO
    slot_getio = ""
    for module in modules:
        slot_getio += f"    SLOT_GETIO({module['name']});\n"
    
    # Input callbacks
    input_callbacks = ""
    consumers = {}
    for consumer in modules:
        producers = config['producers'].get(consumer['name'], [])
        if producers:
            callbacks = []
            for producer in producers:
                callbacks.append(f"    INPUT_GET_SLOT({producer}, {consumer['name']});")
            
            input_callbacks += f"""
INPUT_CALLBACK({producers[0]}, {consumer['name']})
{{
{chr(10).join(callbacks)}
}}
"""
    
    # Output callbacks
    output_callbacks = ""
    for module in modules:
        consumers_list = config['consumers'].get(module['name'], [])
        if "DrvBuzzer" in consumers_list:
            output_callbacks += f"""
void {module['name']}_OutputCallback(Para_Grp_t *pOut)
{{
    if (!pOut || !pOut->para) return;
    
    {module['name']}_Output *out = ({module['name']}_Output *)pOut->para;
    if (out->DrvBuzzer_params && (out->DrvBuzzer_params->status & ST_NEW)) {{
        // TODO: 蜂鸣器即时路由逻辑
        out->DrvBuzzer_params->status &= ~ST_NEW;
    }}
}}
"""
    
    # Module calls in Run
    module_calls = ""
    for module in modules:
        module_calls += f"    if (s_slots[SLOT({module['name']})].pDoWork) s_slots[SLOT({module['name']})].pDoWork();\n"
    
    content = DATA_SWITCHER_TEMPLATE.format(
        io_includes=io_includes,
        slot_enum=slot_enum,
        slot_getio=slot_getio,
        input_callbacks=input_callbacks,
        output_callbacks=output_callbacks,
        module_calls=module_calls
    )
    
    return content

def generate_module_c(module, config):
    """生成模块 .c 文件"""
    module_name = module['name']
    module_lower = module_name.lower()
    layer = module['layer']
    
    consumers = config['consumers'].get(module_name, [])
    producers = config['producers'].get(module_name, [])
    
    # ========== Producer 部分: 有下游模块才生成 PARAMS/LINK/OUTPUT ==========
    if consumers:
        # OUTPUT 聚合实例
        output_declaration = f"static MODULE_OUTPUT({module_name}) s_out;"
        
        params_instances = ""
        for consumer in consumers:
            params_instances += f"static MODULE_OUTPUT_PARAMS({module_name}, {consumer}) s_params_{consumer.lower()};\n"
        
        link_instances = ""
        for consumer in consumers:
            link_instances += f"static MODULE_OUTPUT_LINK({module_name}, {consumer}) s_link_{consumer.lower()};\n"
        
        params_init = ""
        for consumer in consumers:
            params_init += f"    memset(&s_params_{consumer.lower()}, 0, sizeof(s_params_{consumer.lower()}));\n"
        
        links_init = ""
        for consumer in consumers:
            links_init += f"""    s_link_{consumer.lower()}.status = 0u;
    s_link_{consumer.lower()}.max_count = 4u;
    s_link_{consumer.lower()}.count = 0u;
    s_link_{consumer.lower()}.params = &s_params_{consumer.lower()};
"""
        
        output_init = ""
        for consumer in consumers:
            output_init += f"    s_out.{consumer}_params = &s_link_{consumer.lower()};\n"
    else:
        output_declaration = "/* OUTPUT (无) — 本模块没有下游输出 */"
        params_instances = "/* PARAMS (无) — 本模块没有下游输出 */"
        link_instances = "/* LINK (无) — 本模块没有下游输出 */"
        params_init = ""
        links_init = ""
        output_init = ""
    
    # ========== Consumer 部分: 有上游模块才生成 s_in ==========
    if producers:
        input_instance = f"static MODULE_INPUT({module_name}) s_in;"
        input_bind = "    g_input.para  = &s_in;"
    else:
        input_instance = "/* INPUT (无) — 本模块没有上游输入 */"
        input_bind = "    /* g_input.para 无上游输入 */"
    
    content = MODULE_C_TEMPLATE.format(
        module_lower=module_lower,
        module_name=module_name,
        layer=layer,
        params_instances=params_instances,
        link_instances=link_instances,
        output_declaration=output_declaration,
        params_init=params_init,
        links_init=links_init,
        output_init=output_init,
        input_instance=input_instance,
        input_bind=input_bind
    )
    
    return content

def main():
    parser = argparse.ArgumentParser(description='four_head v2.3 代码自动生成器')
    parser.add_argument('--config', required=True, help='配置文件路径 (JSON)')
    parser.add_argument('--output', default='.', help='输出根目录 (默认: 当前目录)')
    parser.add_argument('--src-root', default='.', help='源码根目录 (默认: 当前目录)')
    args = parser.parse_args()
    
    # 读取配置
    with open(args.config, 'r', encoding='utf-8') as f:
        config = json.load(f)
    
    # 创建输出目录
    output_root = args.output
    src_root = args.src_root
    
    os.makedirs(f'{output_root}/include', exist_ok=True)
    os.makedirs(f'{output_root}/core', exist_ok=True)
    
    # 生成 io.h 文件
    for module in config['modules']:
        content = generate_io_h(module, config)
        with open(f"{output_root}/include/{module['name'].lower()}_io.h", 'w', encoding='utf-8') as f:
            f.write(content)
        print(f"生成: {output_root}/include/{module['name'].lower()}_io.h")
    
    # 生成 data_switcher.c
    content = generate_data_switcher(config)
    with open(f"{output_root}/core/data_switcher.c", 'w', encoding='utf-8') as f:
        f.write(content)
    print(f"生成: {output_root}/core/data_switcher.c")
    
    # 生成模块 .c 文件
    for module in config['modules']:
        content = generate_module_c(module, config)
        os.makedirs(f"{output_root}/{module['layer']}", exist_ok=True)
        with open(f"{output_root}/{module['layer']}/{module['name'].lower()}.c", 'w', encoding='utf-8') as f:
            f.write(content)
        print(f"生成: {output_root}/{module['layer']}/{module['name'].lower()}.c")
    
    print(f"\n✅ 代码生成完成！输出目录: {output_root}")

if __name__ == '__main__':
    main()