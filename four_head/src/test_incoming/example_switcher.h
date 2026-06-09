/**
 * @file    example_switcher.h
 * @brief   Switcher — 数据交换机中间层 (范例)
 *
 * 全项目唯一有权 include 所有 _io.h 的文件。
 * 模块之间互不知道对方存在，路由集中在 Switcher 完成。
 */
#ifndef EXAMPLE_SWITCHER_H
#define EXAMPLE_SWITCHER_H

#include <stdint.h>

/* ---- 调试接口（仅 demo 用 — 生产代码不需要） ---- */
typedef struct {
    uint16_t a_counter;     /* Producer A 的当前计数器值 */
    uint8_t  a_status;      /* Producer A 的 status 字节 */
    uint16_t b_consumed;    /* Consumer B 消费后的结果 */
} SwitcherDebug_t;

void ExampleSwitcher_Init(void);
void ExampleSwitcher_Run(void);
void ExampleSwitcher_GetDebug(SwitcherDebug_t *pDbg);

#endif /* EXAMPLE_SWITCHER_H */
