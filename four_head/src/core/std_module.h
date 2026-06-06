/**
 * @file    std_module.h
 * @brief   统一模块框架
 *
 * 所有模块共享同一套输入输出路径，用 route 字段分流：
 *
 *   输入: 别人写 g_input.para + g_input.info.route + ST_NEW
 *   处理: ProcessInput() 读 route 分流
 *   输出: 写 g_output.para + g_output.info.route(=输入route) + ST_OUT
 *   回调: 骨架自动调 module_OnOutput(&g_output)
 *         接收方强符号覆盖，读 pOut->info.route 分支处理
 *
 * ISR 事件（中断服务）:
 *   紧急: 走独立回调函数，不碰 g_input/g_output
 *   不紧急: ISR 设标志，Switcher 检测后从 g_input 正常路径注入
 *
 * 一对多输出:
 *   OutData_t 一个结构体容纳所有下游需要的数据
 *   强符号内按需 memcpy 分发给各模块
 *   不依赖回调链，一个强符号覆盖即可
 *
 * 使用:
 *   1. MODULE_SKELETON()        — 文件顶部
 *   2. 实现 Init() + ProcessInput()
 *   3. ProcessInput 内 g_output.info.route = g_input.info.route
 *   4. MODULE_EXPORT(模块名)    — 文件底部
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
    ST_OUT   = 0x04,   /* bit2: 立即输出回调 */
};

/* ========== 2. 信息头 ========== */
typedef struct {
    uint8_t status;   /* ST_INIT, ST_NEW, ST_OUT */
    uint8_t res1;
    uint8_t route;    /* 路由标识 */
    uint8_t res2;
} Info_Header;

/* ========== 2. 统一参数组 ========== */
typedef struct {
    Info_Header info;
    void       *para;
} Para_Grp_t;

/* ========== 3. 骨架宏 ========== */
#define MODULE_SKELETON()                                                \
    static Para_Grp_t g_input;                                          \
    static Para_Grp_t g_output;                                         \
    static uint8_t    g_init_done = 0;                                  \
    static void       (*_onOutput)(Para_Grp_t *) = NULL;                \
                                                                         \
    static void Init(void);                                              \
    static void ProcessInput(void);                                      \
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
        if ((g_output.info.status & ST_OUT) && _onOutput) {             \
            _onOutput(&g_output);                                       \
            g_output.info.status &= ~ST_OUT;                             \
        }                                                                \
    }

/* ========== 4. 导出宏 ========== */
#define MODULE_EXPORT(module_name)                                       \
    void module_name##_GetIO(Para_Grp_t **ppIn,                          \
                             Para_Grp_t **ppOut,                         \
                             void      (**ppDoWork)(void)) {             \
        *ppIn     = &g_input;                                            \
        *ppOut    = &g_output;                                           \
        *ppDoWork = DoWork;                                              \
    }                                                                    \
                                                                         \
    __attribute__((weak)) void module_name##_OnOutput(Para_Grp_t *pOut)  \
    { (void)pOut; }                                                      \
    static void _Reg__##module_name(void) __attribute__((constructor));  \
    static void _Reg__##module_name(void) {                              \
        _onOutput = module_name##_OnOutput;                              \
    }

#endif /* STD_MODULE_H */
