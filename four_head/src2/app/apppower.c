/**
 * @file    apppower.c
 * @brief   AppPower 模块实现 (v2.3 LINK+PARAMS)
 * @layer   app
 *
 * 功率控制 PID — 聚合 CommMgr/Protect/Cooking 输入，输出功率命令到 CommMgr 和状态到 Hmi
 */

// ===== [AI GENERATED] — 每次重生成 ===== 
#include "include/apppower_io.h"
#include "apppower.h"

MODULE_SKELETON(AppPower);

/* 管道就绪标志: 每 BIT 代表一个管道的 ST_NEW 状态 */
typedef union {
    uint8_t all;           /* 全零 = 无数据就绪，跳过本次 */
    struct {
        uint8_t appcommmgr   : 1;  /* AppCommMgr 数据就绪 */
        uint8_t appprotect   : 1;  /* AppProtect 数据就绪 */
        uint8_t appcooking   : 1;  /* AppCooking 数据就绪 */
    } bits;
} AppPower_PipeFlags_t;

// ===== [USER CODE] =====
// >>> 用户常量、变量、业务函数
// 此区域由用户维护，AI 重生成时保留

// >>> 用户常量、变量、业务函数
// 此区域由用户维护，AI 重生成时保留

// 用户业务函数 — by ProcessInput 调用
void AppPower_Process(MODULE_INPUT(AppPower) *in, MODULE_OUTPUT(AppPower) *out, AppPower_PipeFlags_t flags)
{
    if (flags.bits.appcommmgr) { /* 处理 AppCommMgr 输入 */ }
    if (flags.bits.appprotect) { /* 处理 AppProtect 输入 */ }
    if (flags.bits.appcooking) { /* 处理 AppCooking 输入 */ }
    /* TODO: 写入 DrvCommMgr, AppHmi 输出 */
}

// >>> 用户常量
// #define APPPOWER_THRESHOLD  100

// >>> 用户变量
// static uint32_t s_apppower_counter;

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
    MODULE_INPUT(AppPower) *in = (MODULE_INPUT(AppPower)*)g_input.para;
    MODULE_OUTPUT(AppPower) *out = (MODULE_OUTPUT(AppPower)*)g_output.para;

    /* === 输入段: 数据有效检查 === */
    if (!in) return;
    AppPower_PipeFlags_t flags = {0};
    flags.bits.appcommmgr = (in->AppCommMgr_params.status & ST_NEW) ? 1 : 0;
    flags.bits.appprotect = (in->AppProtect_params.status & ST_NEW) ? 1 : 0;
    flags.bits.appcooking = (in->AppCooking_params.status & ST_NEW) ? 1 : 0;
    if (!flags.all) return;

    /* === 计算段: 用户业务 === */
    AppPower_Process(in, out, flags);

    /* === 输出段: 状态管理 === */
    in->AppCommMgr_params.status  &= ~ST_NEW;  /* 消费完成 */
    in->AppProtect_params.status  &= ~ST_NEW;  /* 消费完成 */
    in->AppCooking_params.status  &= ~ST_NEW;  /* 消费完成 */
    out->DrvCommMgr_params.status |= ST_OUT;  /* 输出就绪 */
    out->AppHmi_params.status |= ST_OUT;  /* 输出就绪 */
}

MODULE_EXPORT(AppPower);
