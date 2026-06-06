/**
 * @file    data_switcher.h
 * @brief   Data Switcher — 统一调度接口
 * @layer   core
 */
#ifndef DATA_SWITCHER_H
#define DATA_SWITCHER_H

#include <stdint.h>

void Switcher_Init(void);
void Switcher_Register(void (*pDoWork)(void));
void Switcher_Run(void);

#endif /* DATA_SWITCHER_H */
