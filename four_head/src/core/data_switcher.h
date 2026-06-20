/**
 * @file    data_switcher.h
 * @brief   Data Switcher — PULL 路由调度器 (v2.3)
 * @layer   core
 *
 * v2.3 新增宏:
 *   - SLOT(mod): 槽位枚举宏
 *   - SLOT_GETIO(mod): 模块注册宏
 *   - INPUT_CALLBACK(consumer): InputCallback 函数声明宏（仅 consumer 名，一个 consumer 一个回调）
 *   - INPUT_GET_SLOT(producer, consumer): do{}while(0) 包裹, 指针直穿宏（零拷贝）
 *   - INPUT_LINK_PULL(producer, consumer, link_member): 带 ST_NEW 检查的指针直穿
 *   - INPUT_EDGE_PULL(producer, consumer, link_member): ST_OUT 边沿触发
 */

#ifndef DATA_SWITCHER_H
#define DATA_SWITCHER_H

#include "std_module.h"

/* ===== 槽位宏 ===== */
#define SLOT(mod)  SLOT_##mod

#define SLOT_GETIO(mod)                                                      \
    mod##_GetIO(&s_slot[SLOT(mod)].pIn,                                      \
                &s_slot[SLOT(mod)].pOut,                                      \
                &s_slot[SLOT(mod)].pDoWork)

/* ===== InputCallback 宏 ===== */

/* 函数壳 — 仅 consumer 名，一个 consumer 一个回调函数
 * 展开为: void consumer##_InputCallback(void) */
#define INPUT_CALLBACK(consumer) \
    void consumer##_InputCallback(void)

/* ===== OutputCallback 宏 ===== */
#define OUTPUT_CALLBACK(consumer) \
    void consumer##_OutputCallback(Para_Grp_t *pOut)

/* 取 slot 指针 + 直穿: 成员名 = {Producer}_params / {Consumer}_params
 * OUTPUT_LINK 已指针化, 直接用 __out->xxx_params (不再取 &)
 * do{}while(0) 包裹: 允许在 INPUT_CALLBACK 内多次调用, 变量名 __out/__in 不冲突 */
#define INPUT_GET_SLOT(producer, consumer) \
    do { \
        MODULE_OUTPUT(producer) *__out = \
            (MODULE_OUTPUT(producer) *)s_slot[SLOT(producer)].pOut->para; \
        MODULE_INPUT(consumer)   *__in  = \
            (MODULE_INPUT(consumer)   *)s_slot[SLOT(consumer)].pIn->para; \
        __in->producer##_params = (void*)__out->consumer##_params; \
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
                __p_in->producer##_params = (void*)__p_out->link_member; \
                s_slot[SLOT(consumer)].pIn->info.status |= ST_NEW; \
            } \
        } \
    } while (0)

/* ST_OUT 边沿触发 */
#define INPUT_EDGE_PULL(producer, consumer, link_member) \
    do { \
        MODULE_OUTPUT(producer) *__p_out = \
            (MODULE_OUTPUT(producer) *)s_slot[SLOT(producer)].pOut->para; \
        if (__p_out && (__p_out->link_member.status & ST_OUT)) { \
            __p_out->link_member.status &= ~ST_OUT; \
            MODULE_INPUT(consumer) *__p_in = \
                (MODULE_INPUT(consumer) *)s_slot[SLOT(consumer)].pIn->para; \
            if (__p_in) { \
                __p_in->producer##_params = (void*)__p_out->link_member; \
            } \
        } \
    } while (0)

void Switcher_Init(void);
void Switcher_Run(void);
void Switcher_Run_All(void);

extern const uint8_t SWITCHER_SLOT_COUNT;


#endif /* DATA_SWITCHER_H */
