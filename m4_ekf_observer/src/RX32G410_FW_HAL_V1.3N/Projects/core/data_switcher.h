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

#include "std_module.h"  /* Para_Grp_t, ModuleSlotDef */

/* ===== 槽位宏 — 强制 Slot 枚举与 GetIO 函数名对齐 ===== */
#define SLOT(mod)  SLOT_##mod

#define SLOT_GETIO(mod)                                                      \
    mod##_GetIO(&s_slot[SLOT(mod)].pIn,                                      \
                &s_slot[SLOT(mod)].pOut,                                      \
                &s_slot[SLOT(mod)].pDoWork)

/* ===== InputCallback 宏 — 管道配对指针直穿 (v2.3) ===== */

/* 函数壳: INPUT_CALLBACK(AppPower) { ... }
 * 展开: void AppPower_InputCallback(void) */
#define INPUT_CALLBACK(consumer) \
    void consumer##_InputCallback(void)

/* 取 slot 指针 + 直穿: 成员名 = {Producer}_params / {Consumer}_params
 * 检查放 ProcessInput */
#define INPUT_GET_SLOT(producer, consumer) \
    do { \
        MODULE_OUTPUT(producer) *__out = \
            (MODULE_OUTPUT(producer) *)s_slot[SLOT(producer)].pOut->para; \
        MODULE_INPUT(consumer)   *__in  = \
            (MODULE_INPUT(consumer)   *)s_slot[SLOT(consumer)].pIn->para; \
        __in->producer##_params = (void*)&__out->consumer##_params; \
    } while (0)

/* 单管道直穿+回调内检查 (ST_NEW 触发)
 * 可在 INPUT_CALLBACK 内多次调用, 独立判断 */
#define INPUT_LINK_PULL(producer, consumer, link_member) \
    do { \
        MODULE_OUTPUT(producer) *__p_out = \
            (MODULE_OUTPUT(producer) *)s_slot[SLOT(producer)].pOut->para; \
        if (__p_out && (__p_out->link_member.status & ST_NEW)) { \
            MODULE_INPUT(consumer) *__p_in = \
                (MODULE_INPUT(consumer) *)s_slot[SLOT(consumer)].pIn->para; \
            if (__p_in) { \
                __p_in->producer##_params = (void*)&__p_out->link_member; \
                s_slot[SLOT(consumer)].pIn->info.status |= ST_NEW; \
            } \
        } \
    } while (0)

/* 单管道边沿触发 (ST_OUT, 自动清除实现 0→1 边沿检测) */
#define INPUT_EDGE_PULL(producer, consumer, link_member) \
    do { \
        MODULE_OUTPUT(producer) *__p_out = \
            (MODULE_OUTPUT(producer) *)s_slot[SLOT(producer)].pOut->para; \
        if (__p_out && (__p_out->link_member.status & ST_OUT)) { \
            __p_out->link_member.status &= ~ST_OUT; \
            MODULE_INPUT(consumer) *__p_in = \
                (MODULE_INPUT(consumer) *)s_slot[SLOT(consumer)].pIn->para; \
            if (__p_in) { \
                __p_in->producer##_params = (void*)&__p_out->link_member; \
                s_slot[SLOT(consumer)].pIn->info.status |= ST_NEW; \
            } \
        } \
    } while (0)

void Switcher_Init(void);
void Switcher_Run_Slot1(void);
void Switcher_Run_TK(void);
void Switcher_Run_All(void);

void Switcher_Slot_AppAdc(void);
void Switcher_Slot_AppPower(void);
void Switcher_Slot_Calculator(void);
void Switcher_Slot_ElecParams(void);
void Switcher_Slot_EKF_LKF(void);
void Switcher_Slot_Telemetry(void);

#endif /* DATA_SWITCHER_H */
