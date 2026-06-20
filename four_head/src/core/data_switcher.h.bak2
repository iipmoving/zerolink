/**
 * @file    data_switcher.h
 * @brief   Data Switcher — 统一调度接口 (v2.3)
 * @layer   core
 *
 * v2.3 新增宏:
 *   - SLOT(mod): 槽位枚举宏
 *   - SLOT_GETIO(mod): 模块注册宏
 *   - INPUT_CALLBACK(consumer): InputCallback 函数声明宏（仅 consumer 名，一个 consumer 一个回调）
 *   - INPUT_GET_SLOT(producer, consumer): 指针直穿宏（零拷贝）
 *   - INPUT_LINK_PULL(producer, consumer, link): 带 ST_NEW 检查的指针直穿
 */
#ifndef DATA_SWITCHER_H
#define DATA_SWITCHER_H

#include "std_module.h"

/* ===== 槽位宏 ===== */
#define SLOT(mod)  SLOT_##mod

#define SLOT_GETIO(mod) \
    mod##_GetIO(&s_slot[SLOT(mod)].pIn, &s_slot[SLOT(mod)].pOut, &s_slot[SLOT(mod)].pDoWork)

/* ===== InputCallback 宏 ===== */

/* 函数壳 — 仅 consumer 名，一个 consumer 一个回调函数
 * 展开为: void consumer##_InputCallback(void) */
#define INPUT_CALLBACK(consumer) \
    void consumer##_InputCallback(void)

/* 指针直穿（推荐：检查放 ProcessInput） */
#define INPUT_GET_SLOT(producer, consumer) \
    MODULE_OUTPUT(producer) *out = \
        (MODULE_OUTPUT(producer) *)s_slot[SLOT(producer)].pOut->para; \
    MODULE_INPUT(consumer)   *in  = \
        (MODULE_INPUT(consumer)   *)s_slot[SLOT(consumer)].pIn->para; \
    in->producer##_params = (void*)&out->consumer##_params

/* 管道直穿 + 回调内检查 ST_NEW */
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
                __p_in->producer##_params = (void*)&__p_out->link_member; \
            } \
        } \
    } while (0)

void Switcher_Init(void);
void Switcher_Run(void);
void Switcher_Run_All(void);

#endif /* DATA_SWITCHER_H */