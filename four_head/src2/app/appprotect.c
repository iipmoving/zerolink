/**
 * @file    appprotect.c
 * @brief   AppProtect 模块实现 (v2.3 LINK+PARAMS)
 * @layer   app
 *
 * 故障保护逻辑 — 从 CommMgr 获取寄存器，输出故障状态到 Power
 */

// ===== [AI GENERATED] — 每次重生成 ===== 
#include "include/appprotect_io.h"
#include "appprotect.h"

MODULE_SKELETON(AppProtect);

/* 管道就绪标志: 每 BIT 代表一个管道的 ST_NEW 状态 */
typedef union {
    uint8_t all;           /* 全零 = 无数据就绪，跳过本次 */
    struct {
        uint8_t appcommmgr   : 1;  /* AppCommMgr 数据就绪 */
    } bits;
} AppProtect_PipeFlags_t;

// ===== [USER CODE] =====
// >>> 用户常量、变量、业务函数
// 此区域由用户维护，AI 重生成时保留

// >>> 用户常量、变量、业务函数
// 此区域由用户维护，AI 重生成时保留

// 用户业务函数 — by ProcessInput 调用
void AppProtect_Process(MODULE_INPUT(AppProtect) *in, MODULE_OUTPUT(AppProtect) *out, AppProtect_PipeFlags_t flags)
{
    if (flags.bits.appcommmgr) { /* 处理 AppCommMgr 输入 */ }
    /* TODO: 写入 AppPower 输出 */
}

// >>> 用户常量
// #define APPPROTECT_THRESHOLD  100

// >>> 用户变量
// static uint32_t s_appprotect_counter;

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
    MODULE_INPUT(AppProtect) *in = (MODULE_INPUT(AppProtect)*)g_input.para;
    MODULE_OUTPUT(AppProtect) *out = (MODULE_OUTPUT(AppProtect)*)g_output.para;

    /* === 输入段: 数据有效检查 === */
    if (!in) return;
    AppProtect_PipeFlags_t flags = {0};
    flags.bits.appcommmgr = (in->AppCommMgr_params.status & ST_NEW) ? 1 : 0;
    if (!flags.all) return;

    /* === 计算段: 用户业务 === */
    AppProtect_Process(in, out, flags);

    /* === 输出段: 状态管理 === */
    in->AppCommMgr_params.status  &= ~ST_NEW;  /* 消费完成 */
    out->AppPower_params.status |= ST_OUT;  /* 输出就绪 */
}

MODULE_EXPORT(AppProtect);
