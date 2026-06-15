/**
 * @file    isrwork.c
 * @brief   ISRWORK 模块 — ISR 实时处理入口 (v2.3 示例)
 * @layer   app
 *
 * 注意: 此文件是 v2.3 架构示例，需要根据实际模块重新设计。
 * v2.3 不提供 DECLARE_MODULE_OUTPUT 宏 (破坏封装)，
 * 跨模块 ISR 数据应通过 GetISR_IO 或中间层管理。
 *
 * 使用:
 *   1. #define STD_MODULE_ENABLE_ISR 1
 *   2. #include "std_module.h"
 *   3. MODULE_SKELETON(ISRWORK)
 *   4. 实现 Init() + ProcessInput() + ISR_ProcessInput()
 *   5. MODULE_EXPORT(ISRWORK)
 *   6. 在 Switcher_PendSV_Init() 后注册 Switcher_RegisterISRModule(ISRWORK_GetISR_IO)
 */

#define STD_MODULE_ENABLE_ISR  1
#include "std_module.h"
#include "pendsv_switcher.h"
#include <string.h>

/* ========== 1. 输入数据结构 ========== */
#pragma pack(4)
typedef struct {
    uint8_t  a_valid;
    uint8_t  b_valid;
    uint8_t  c_valid;
    uint8_t  isr_source;    /* ISR 来源标识 */
    uint8_t  res[4];        /* 对齐填充 */

    uint16_t a_value;       /* ModuleA 数据 */
    uint16_t b_value;       /* ModuleB 数据 */
    uint16_t c_value;       /* ModuleC 数据 */
} ISRWORK_InData_t;
#pragma pack()

/* ========== 2. 输出数据结构 ========== */
#pragma pack(4)
typedef struct {
    uint8_t  result_valid;
    uint8_t  res[3];
    uint16_t fused_value;   /* 融合结果 */
    uint16_t fault_code;    /* 故障码 */
} ISRWORK_OutData_t;
#pragma pack()

static ISRWORK_InData_t  s_in;
static ISRWORK_OutData_t s_out;

/* ========== 3. 骨架宏 ========== */
MODULE_SKELETON(ISRWORK);

/* ========== 4. 辅助: 收集输入数据 ========== */
static void CollectInput(void)
{
    ISRWORK_InData_t *in = (ISRWORK_InData_t *)g_input.para;
    /* TODO: 通过 GetISR_IO 或中间层从其他模块获取数据 */

    /* 记录 ISR 来源 */
    in->isr_source = g_isr_source;

    g_input.info.status |= ST_NEW;
}

/* ========== 5. ISR_ProcessInput: PendSV 通路三段式处理 ========== */
static void ISR_ProcessInput(void)
{
    ISRWORK_InData_t  *in  = (ISRWORK_InData_t *)g_isr_input.para;
    ISRWORK_OutData_t *out = (ISRWORK_OutData_t *)g_isr_output.para;

    /* ① 输入段: 收集数据 */
    CollectInput();

    if (!(g_isr_input.info.status & ST_NEW)) {
        return;
    }

    /* ② 计算段: 根据 ISR 来源选择处理策略 */
    switch (in->isr_source) {
        case ISR_SOURCE_ADC:
            /* ADC ISR: 快速处理 */
            if (in->a_valid && in->b_valid) {
                out->fused_value = (in->a_value + in->b_value) / 2;
            }
            break;

        case ISR_SOURCE_TIMER:
            if (in->a_valid && in->b_valid && in->c_valid) {
                out->fused_value = (in->a_value + in->b_value + in->c_value) / 3;
            }
            break;

        case ISR_SOURCE_FAULT:
            out->fault_code = 0x1234;
            break;

        default:
            break;
    }

    /* ③ 输出段 */
    out->result_valid = 1;
    g_isr_output.info.status |= ST_OUT;
    g_isr_input.info.status &= ~ST_NEW;
}

/* ========== 6. ProcessInput: 主循环通路三段式处理 ========== */
static void ProcessInput(void)
{
    /* 主循环中 ISRWORK 的处理逻辑 (如有需要) */
}

/* ========== 7. Init: 初始化 ========== */
static void Init(void)
{
    memset(&s_in,  0, sizeof(s_in));
    memset(&s_out, 0, sizeof(s_out));

    g_input.para       = &s_in;
    g_output.para      = &s_out;
    g_isr_input.para   = &s_in;   /* ISR 和主循环共用输入缓冲 (示例) */
    g_isr_output.para  = &s_out;  /* ISR 和主循环共用输出缓冲 (示例) */
}

/* ========== 8. 导出模块 ========== */
MODULE_EXPORT(ISRWORK);
