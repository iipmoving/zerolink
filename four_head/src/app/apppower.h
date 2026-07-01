/**
 * @file    apppower.h
 * @brief   AppPower 模块公共接口
 * @layer   app
 *
 * 功率控制 PID — 聚合 CommMgr/Protect/Cooking 输入，输出功率命令到 CommMgr 和状态到 Hmi
 */

#ifndef APPPOWER_H
//#define APPPOWER_H   /* L0 阻断 */

#include <stdint.h>

/* ---- v2.3 统一接口声明 ---- */
MODULE_IO_H(AppPower);

#endif /* guard */
