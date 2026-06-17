/**
 * @file    apphmi.c
 * @brief   AppHmi 模块实现 (v2.3 LINK+PARAMS)
 * @layer   app
 *
 * 人机交互逻辑 — 从 Key/Power 输入，输出显示到 Display 和蜂鸣到 Buzzer
 */

// ===== [AI GENERATED] — 每次重生成 ===== 
#include "include/apphmi_io.h"
#include "apphmi.h"

MODULE_SKELETON(AppHmi);

/* 管道就绪标志: 每 BIT 代表一个管道的 ST_NEW 状态 */
typedef union {
    uint8_t all;           /* 全零 = 无数据就绪，跳过本次 */
    struct {
        uint8_t drvkey   : 1;  /* DrvKey 数据就绪 */
        uint8_t apppower   : 1;  /* AppPower 数据就绪 */
    } bits;
} AppHmi_PipeFlags_t;

// ===== [USER CODE] =====
// >>> 用户常量、变量、业务函数
// 此区域由用户维护，AI 重生成时保留

// 用户业务函数 — by ProcessInput 调用
void AppHmi_Process(MODULE_INPUT(AppHmi) *in, MODULE_OUTPUT(AppHmi) *out, AppHmi_PipeFlags_t flags)
{
    if (flags.bits.drvkey) { /* 处理 DrvKey 输入 */ }
    if (flags.bits.apppower) { /* 处理 AppPower 输入 */ }
    /* TODO: 写入 DrvDisplay, DrvBuzzer 输出 */
}

// >>> 用户常量
// #define APPHMI_THRESHOLD  100

// >>> 用户变量
// static uint32_t s_apphmi_counter;

// ===== [END USER CODE] =====

// ===== [AI GENERATED] — 每次重生成 ===== 

static void Init(void)
{
    g_input.para  = &s_in;
    g_output.para = &s_out;

    // >>> [USER: 添加初始化代码]
}

static void ProcessInput(void)
{
    MODULE_INPUT(AppHmi) *in = (MODULE_INPUT(AppHmi)*)g_input.para;
    MODULE_OUTPUT(AppHmi) *out = (MODULE_OUTPUT(AppHmi)*)g_output.para;

    /* === 输入段: 数据有效检查 === */
    if (!in) return;
    AppHmi_PipeFlags_t flags = {0};
    flags.bits.drvkey = (in->DrvKey_params.status & ST_NEW) ? 1 : 0;
    flags.bits.apppower = (in->AppPower_params.status & ST_NEW) ? 1 : 0;
    if (!flags.all) return;

    /* === 计算段: 用户业务 === */
    AppHmi_Process(in, out, flags);

    /* === 输出段: 状态管理 === */
    in->DrvKey_params.status  &= ~ST_NEW;  /* 消费完成 */
    in->AppPower_params.status  &= ~ST_NEW;  /* 消费完成 */
    out->DrvDisplay_params.status |= ST_OUT;  /* 输出就绪 */
    out->DrvBuzzer_params.status |= ST_OUT;  /* 输出就绪 */
}

MODULE_EXPORT(AppHmi);
