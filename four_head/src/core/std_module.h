/**
 * @file    std_module.h
 * @brief   统一模块框架 — PULL 路由范式
 *
 * 模块写 g_output.para，不主动推送。Switcher 在 DoWork 调用之间显式路由数据。
 *
 *   输入: consumer STRONG 回调写 g_input.para + 置 ST_NEW
 *   处理: ProcessInput() 消费 g_input.para → 计算 → 写 g_output.para
 *   输出: Switcher 显式调用 consumer 回调 (PULL 模式)
 *
 * ISR 事件（中断服务）:
 *   紧急: 走独立回调函数，不碰 g_input/g_output
 *   不紧急: ISR 设标志，Switcher 检测后从 g_input 正常路径注入
 *
 * 使用:
 *   1. MODULE_SKELETON()        — 文件顶部
 *   2. 实现 Init() + ProcessInput()
 *   3. MODULE_EXPORT(模块名)    — 文件底部
 *
 * 引用: #include "core/std_module.h"
 */
#ifndef STD_MODULE_H
#define STD_MODULE_H

#include <stdint.h>
#include <string.h>

/* ========== 1. 状态位常量 ========== */
enum {
    ST_INIT  = 0x01,   /* bit0: 已初始化 */
    ST_NEW   = 0x02,   /* bit1: 新输入到达 */
    ST_OUT   = 0x04,   /* bit2: 输出就绪（设 ST_OUT 触发 _onOutput） */
};

/* ========== 2. 信息头 ========== */
typedef struct {
    uint8_t status;   /* ST_INIT, ST_NEW */
    uint8_t res1;
    uint8_t route;    /* 路由标识 */
    uint8_t res2;
} Info_Header;

/* ========== 3. 统一参数组 ========== */
typedef struct {
    Info_Header info;
    void       *para;
} Para_Grp_t;

/* ========== 4. 骨架宏 ========== */
#define MODULE_SKELETON()                                                \
    static Para_Grp_t g_input;                                          \
    static Para_Grp_t g_output;                                         \
    static uint8_t    g_init_done = 0;                                  \
                                                                         \
    static void Init(void);                                              \
    static void ProcessInput(void);                                      \
                                                                         \
    __attribute__((weak)) void _onOutput(Para_Grp_t *pOut)               \
    { (void)pOut; }                                                      \
                                                                         \
    static void Constructor(void) {                                      \
        memset(&g_input,  0, sizeof(g_input));                           \
        memset(&g_output, 0, sizeof(g_output));                         \
        g_init_done = 0;                                                \
        Init();                                                         \
        g_output.info.status |= ST_INIT;                                 \
    }                                                                    \
                                                                         \
    static void DoWork(void) {                                           \
        if (!g_init_done) { Constructor(); g_init_done = 1; }           \
        ProcessInput();                                                  \
        if ((g_output.info.status & ST_OUT) && _onOutput) {              \
            _onOutput(&g_output);                                        \
            g_output.info.status &= ~ST_OUT;                              \
        }                                                                \
    }

/* ========== 5. 导出宏 ========== */
#define MODULE_EXPORT(module_name)                                       \
    void module_name##_GetIO(Para_Grp_t **ppIn,                          \
                             Para_Grp_t **ppOut,                         \
                             void      (**ppDoWork)(void)) {             \
        *ppIn     = &g_input;                                            \
        *ppOut    = &g_output;                                           \
        *ppDoWork = DoWork;                                              \
    }

#endif /* STD_MODULE_H */
