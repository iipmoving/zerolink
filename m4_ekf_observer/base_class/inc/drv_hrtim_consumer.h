/**
 * @file    drv_hrtim_consumer.h
 * @brief   DrvHrtimConsumer — 单入口多 Route HRTIM 驱动消费模块
 * @layer   base_class
 *
 * 输入: PowerHw_Command_t (来自 APP_Power)
 * 输出: PowerHw_Status_t  (反馈给 APP_Power)
 *
 * 内部 Route:
 *   - pan_detect:  检锅 + 起振序列
 *   - output_apply: 累加 ppg_delta → ISR 缓冲
 *   - protect:     BK/限流/故障读取
 *   - sync:        Master 同步周期管理
 */
#ifndef DRV_HRTIM_CONSUMER_H
#define DRV_HRTIM_CONSUMER_H

#include "core/std_module.h"

/* ---- v2.2 注册入口 ---- */
MODULE_IO_H(DrvHrtim);

#endif /* DRV_HRTIM_CONSUMER_H */
