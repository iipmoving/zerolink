/**
 * hal_key.h —— 触摸按键HAL接口
 *
 * 依赖: 无（仅 stdint.h）
 * 层级: HAL —— 封装赛元触摸库
 *
 * 触摸库使用: TK_Init → Sys_Scan(轮询驱动) → KeyFlag(读取键值)
 * 21键触摸面板: 通道7-24, 28-30
 *
 * 时序: HAL_Key_Poll() 需每 1ms 从 TIM0 ISR 调用，驱动触摸状态机
 *       HAL_Key_Scan() 在 10ms 槽位调用，读取去抖后的键值
 */
#ifndef HAL_KEY_H
#define HAL_KEY_H

#include <stdint.h>

// HAL_Key_Scan 返回此值表示触摸扫描周期未完成，无有效数据
#define KEY_SCAN_NOT_READY  0xFFFFFFFFu

void HAL_Key_Init(void);

/* ISR中调用(1ms) —— 驱动触摸库状态机，不阻塞 */
void HAL_Key_Poll(void);

/* 主循环中调用 —— 返回去抖后的按键位掩码，无键返回0，扫描未完成返回KEY_SCAN_NOT_READY */
uint32_t HAL_Key_Scan(void);

#endif /* HAL_KEY_H */
