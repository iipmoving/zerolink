/**
 * @file    app_power.h
 * @brief   AppPower 模块公共接口
 * @layer   app
 *
 * 功率控制 PID — 从 AppAdc 拉取数据，输出功率增量到 DrvHrtim
 */

#ifndef APPPOWER_H
//#define APPPOWER_H   /* L0 阻断 */

#include <stdint.h>

/* ---- v2.3 统一接口声明 ---- */
MODULE_IO_H(AppPower);

#endif /* guard */
