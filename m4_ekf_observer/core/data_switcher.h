/**
 * @file    data_switcher.h
 * @brief   v2.0 Data Switcher — 结构化数据路由中间层
 * @layer   core
 *
 * 数据交换机在调度槽中按固定顺序:
 *   1. 调 producer Adc_DoWork() → 检查 status bit1 → 路由 → 清标志
 *   2. 调 consumer Power_DoWork()
 */

#ifndef DATA_SWITCHER_H
#define DATA_SWITCHER_H

void Switcher_Init(void);
void Switcher_Run_Slot1(void);

#endif /* DATA_SWITCHER_H */
