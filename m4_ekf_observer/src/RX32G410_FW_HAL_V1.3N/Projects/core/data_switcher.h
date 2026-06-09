/**
 * @file    data_switcher.h
 * @brief   Data Switcher — PULL 路由调度器 (v2.2)
 * @layer   core
 *
 * 数据交换机在调度槽中按顺序调用各模块 DoWork.
 * 模块通过 3-参数 GetIO (MODULE_EXPORT 风格) 注册函数指针和输出指针。
 */

#ifndef DATA_SWITCHER_H
#define DATA_SWITCHER_H

#include "std_module.h"  /* Para_Grp_t */

void Switcher_Init(void);
void Switcher_Run_Slot1(void);
void Switcher_Register(void (*pDoWork)(void), Para_Grp_t *pOut);

#endif /* DATA_SWITCHER_H */
