/**
 * @file    drv_hrtim_consumer.c
 * @brief   DrvHrtimConsumer — 单入口多 Route HRTIM 驱动消费模块
 * @layer   base_class
 *
 * 全部 Route 在同一文件, 共享 file-scope static 变量。
 * 零 extern, 零跨文件耦合。
 *
 * 输入: PowerHw_Command_t (来自 APP_Power)
 * 输出: PowerHw_Status_t  (反馈给 APP_Power)
 *
 * Route 执行顺序:
 *   1. Route_pan_detect    检锅 + 起振
 *   2. Route_protect       保护/BK 读取
 *   3. Route_sync          同步管理
 *   4. Route_output_apply  累加 ppg_delta → ISR 缓冲
 *   5. 收集影子变量 → g_output 反馈
 */
#include "core/std_module.h"
#include "drv_hrtim_consumer.h"
#include "app_power_hw_io.h"
#include <string.h>

/* ===================================================================
 * 影子变量 (file-scope static — 仅本文件可见)
 * =================================================================== */

/* 输出影子 (由 Route_output_apply 维护) */
static uint16_t s_shadow_duty[HW_POTMAX];
static uint16_t s_shadow_period[HW_POTMAX];
static uint8_t  s_shadow_on_off[HW_POTMAX];

/* 检锅状态 (由 Route_pan_detect 维护) */
static uint8_t  s_pan_in_progress;

/* 同步状态 (由 Route_sync 维护) */
static uint16_t s_master_period;
static uint8_t  s_sync_active;

/* ===================================================================
 * I/O 缓冲区
 * =================================================================== */
static PowerHw_Command_t s_in;
static PowerHw_Status_t  s_out;

/* ---- 骨架 ---- */
MODULE_SKELETON(DrvHrtim);

/* ===================================================================
 * Route_pan_detect — 检锅 + 起振
 *
 * TODO: Phase 3.2 从 app_power_claude.c 迁入检锅状态机全部代码:
 *   - PanStatusCheck()      检锅状态机
 *   - check_pot_pluse()     起振脉冲产生
 *   - s_pan_check_fun()     移锅检测
 *   - API_DMA_PAN_IRQHandlerCallBack  脉冲计数
 *   - API_POWER_PanCheckPluse()       脉冲检测
 *   - RealTimePulseDetector           脉冲检测器
 * =================================================================== */
static void Route_pan_detect(const PowerHw_Command_t *cmd, uint8_t isNewData)
{
    (void)cmd;
    (void)isNewData;

    /* ---- 检查 APP 检锅请求 ---- */
    if (cmd->pan_request) {
        s_pan_in_progress = 1;
    }

    /* TODO: 检锅状态机主循环 */
}

/* ===================================================================
 * Route_protect — 保护/BK 读取
 *
 * 读取 HRTIM BK 标志、故障状态, 写入 out 反馈给 APP。
 * TODO: Phase 3.2 迁入 FunTimBkFlag() 等 HW 读取 + 限幅逻辑
 * =================================================================== */
static void Route_protect(const PowerHw_Command_t *cmd, PowerHw_Status_t *out, uint8_t isNewData)
{
    (void)cmd;
    (void)isNewData;

    /* TODO: 从 HW 读取 BK 标志 */
    for (uint8_t i = 0; i < HW_POTMAX; i++) {
        out->bk_flag[i] = 0;
        out->fault[i]   = 0;
    }
    out->surge_flag = 0;
}

/* ===================================================================
 * Route_sync — 同步管理
 *
 * 响应 APP 的 sync_request, 设置 master 同步周期。
 * TODO: Phase 3.2 迁入同步管理逻辑
 * =================================================================== */
static void Route_sync(const PowerHw_Command_t *cmd, uint8_t isNewData)
{
    (void)isNewData;

    if (cmd->sync_request) {
        s_sync_active = 1;
        if (cmd->target_freq > 0) {
            s_master_period = cmd->target_freq;
        }
    }
}

/* ===================================================================
 * Route_output_apply — 输出累积 + ISR 缓冲
 *
 * 核心路径:
 *   1. 累加 ppg_delta → s_shadow_duty
 *   2. 处理 power_on 开关
 *   3. 准备 ISR 缓冲数据, 供中断服务程序写入 HRTIM
 *
 * TODO: Phase 3.2 迁入:
 *   - FunPPGsetDuty / FunPPGonOff / FunDeadTimeSetValue 替换
 * =================================================================== */
static void Route_output_apply(const PowerHw_Command_t *cmd, uint8_t isNewData)
{
    (void)isNewData;

    for (uint8_t i = 0; i < HW_POTMAX; i++) {
        /* 增量累积 */
        if (cmd->delta_valid[i]) {
            int32_t new_duty = (int32_t)s_shadow_duty[i] + cmd->ppg_delta[i];
            if (new_duty < 0)    new_duty = 0;
            if (new_duty > 65535) new_duty = 65535;
            s_shadow_duty[i] = (uint16_t)new_duty;
        }

        /* 开关 */
        s_shadow_on_off[i] = cmd->power_on[i];
    }

    /* TODO: 准备 ISR 缓冲 — 写共享变量供中断读取 */
}

/* ---- 初始化 ---- */
static void Init(void)
{
    memset(&s_in,  0, sizeof(s_in));
    memset(&s_out, 0, sizeof(s_out));

    memset(s_shadow_duty,   0, sizeof(s_shadow_duty));
    memset(s_shadow_period, 0, sizeof(s_shadow_period));
    memset(s_shadow_on_off, 0, sizeof(s_shadow_on_off));
    s_master_period   = 0;
    s_sync_active     = 0;
    s_pan_in_progress = 0;

    g_input.para  = &s_in;
    g_output.para = &s_out;
}

/* ---- 处理逻辑 ---- */
static void ProcessInput(void)
{
    uint8_t isNewData = (g_input.info.status & ST_NEW);

    if (isNewData) {
        g_input.info.status &= ~ST_NEW;
    }

    PowerHw_Command_t *cmd = (PowerHw_Command_t *)g_input.para;
    PowerHw_Status_t  *out = (PowerHw_Status_t  *)g_output.para;

    memset(out, 0, sizeof(*out));

    Route_pan_detect(cmd, isNewData);
    Route_protect(cmd, out, isNewData);
    Route_sync(cmd, isNewData);
    Route_output_apply(cmd, isNewData);

    /* 反馈: 影子变量 → Status */
    for (uint8_t i = 0; i < HW_POTMAX; i++) {
        out->current_duty[i]   = s_shadow_duty[i];
        out->current_period[i] = s_shadow_period[i];
        out->current_on_off[i] = s_shadow_on_off[i];
    }
    out->valid = 1;

    g_output.info.status |= ST_OUT;
}

/* ---- 导出 ---- */
MODULE_EXPORT(DrvHrtim);
