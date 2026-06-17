/**
 * appprotect.c —— AppProtect 模块实现 (v2.3)
 *
 * 依赖: include/appprotect_io.h
 * 层级: app
 *
 * v2.3 三层结构: PARAMS → LINK → OUTPUT
 * 指针直穿: Consumer 回调不做 memcpy，直接赋指针
 *
 * TODO: 填写以下内容:
 *   1. Init() - 初始化逻辑
 *   2. ProcessInput() - 三段式处理逻辑
 *   3. 业务逻辑函数
 */
#include "include/appprotect_io.h"  /* 包含 std_module.h + 三层结构定义 */
#include "appprotect.h"
#include <stddef.h>

/* ---- v2.3 三层数据结构 ---- */
/* PARAMS 实例 (纯数据) - TODO: 添加字段初始化 */
static MODULE_OUTPUT_PARAMS(AppProtect, AppPower) s_params_apppower;


/* LINK 实例 (status + *params) */
static MODULE_OUTPUT_LINK(AppProtect, AppPower) s_link_apppower;


/* OUTPUT 实例 (多LINK指针聚合) - Producer 专用 */
static MODULE_OUTPUT(AppProtect) s_out;

/* INPUT 实例 — Consumer 的输入由 Switcher 通过 InputCallback 赋值 */
static MODULE_INPUT(AppProtect) s_in;

MODULE_SKELETON(AppProtect);

/* ========== 业务逻辑 ========== */

static void ProcessInput(void)
{
    /* TODO: 三段式处理逻辑 */
    /* 1. 输入段: 检查 status & ST_NEW → 消费 → 清除标志 */
    /* 2. 计算段: 纯逻辑处理 */
    /* 3. 输出段: 写 g_output.para */
}

/* ========== 初始化 ========== */
static void Init(void)
{
    /* TODO: 添加初始化逻辑 */
    
    /* v2.3: 初始化三层结构 */
        memset(&s_params_apppower, 0, sizeof(s_params_apppower));

    
        s_link_apppower.status = 0u;
    s_link_apppower.max_count = 4u;
    s_link_apppower.count = 0u;
    s_link_apppower.params = &s_params_apppower;

    
    /* OUTPUT 初始化 — LINK 指针绑定 */
        s_out.AppPower_params = &s_link_apppower;

    
    /* g_input/g_output 绑定 */
    g_input.para  = &s_in;
    g_output.para = &s_out;
}

void appprotect_Init(void) { Constructor(); }
MODULE_EXPORT(AppProtect);

/* ========== 业务函数 ========== */
/* TODO: 添加业务函数 */
