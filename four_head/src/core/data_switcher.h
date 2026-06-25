/**
 * @file    data_switcher.h
 * @brief   v2.3 Data Switcher — PULL 路由调度器
 * @layer   core
 *
 * 调度入口 (覆盖 main.c 的 __weak 空壳):
 *   Slot_Init()        — 初始化: Switcher_Init() + Switcher_Run_All()
 *   Slot_every1ms()    — 每1ms ISR 后调用 (实时模块)
 *   Slot_loop1ms()     — 主循环每轮调用 (后台任务)
 *   Switcher_Run_Slot0~10 — 时间片轮转, 11ms 完整周期
 */

#ifndef DATA_SWITCHER_H
#define DATA_SWITCHER_H

void Switcher_Init(void);
void Switcher_Run_All(void);

void Slot_Init(void);
void Slot_every1ms(void);
void Slot_loop1ms(void);

void Switcher_Run_Slot0(void);
void Switcher_Run_Slot1(void);
void Switcher_Run_Slot2(void);
void Switcher_Run_Slot3(void);
void Switcher_Run_Slot4(void);
void Switcher_Run_Slot5(void);
void Switcher_Run_Slot6(void);
void Switcher_Run_Slot7(void);
void Switcher_Run_Slot8(void);
void Switcher_Run_Slot9(void);
void Switcher_Run_Slot10(void);

#endif /* DATA_SWITCHER_H */
