/**
 * @file    std_module.h
 * @brief   统一模块框架 — PULL 路由范式 (v2.2)
 *
 * ================================================================
 * 核心原则：单向调用 — 下游拉 (InputCallback), 上游不推
 * ================================================================
 *
 * 数据流:
 *   上游 DoWork() → 写 g_output.para + 置 ST_OUT
 *   中间层 name_InputCallback (强符号) → 从上游 s_slot[].pOut 拉数据
 *                                     → 写下游 s_slot[].pIn + 置 ST_NEW
 *   下游 DoWork() → ProcessInput() → 消费 g_input.para
 *
 * DoWork 调用顺序:
 *   1. name##_InputCallback()  — weak 空壳, 中间层覆盖强符号注入数据
 *   2. ProcessInput()          — 三段式: 输入段 → 计算段 → 输出段
 *   3. name##_OutputCallback() — weak 空壳, 仅 ST_OUT 时调用, 一般不用
 *
 * InputCallback — 常规路由方式:
 *   - DoWork 第一件事, 在 ProcessInput 之前执行
 *   - 中间层覆盖强符号: 从上游 g_output 读 → 写本模块 g_input
 *   - 不需要参数: 数据源和目标都在 s_slot[] 里, 规划阶段定好
 *   - 单向调用: 上游只写, 下游只拉, 互不知对方存在
 *
 * OutputCallback — 仅特殊场景:
 *   - ProcessInput 中间需要即时调用别的模块小功能
 *   - 需要跨模块即时同步状态 (紧急输出)
 *   - 一般不用, 默认空壳
 *
 * ISR 事件（中断服务）:
 *   紧急: 走独立回调函数，不碰 g_input/g_output
 *   不紧急: ISR 设标志，Switcher 检测后从 g_input 正常路径注入
 *
 * ================================================================
 * 使用步骤:
 *   1. MODULE_SKELETON(模块名)        — 文件顶部 (注册 callback)
 *   2. 实现 Init()      — g_input.para = &s_in
 *                              g_output.para = &s_out
 *   3. 实现 ProcessInput() — 三段式处理
 *   4. MODULE_EXPORT(模块名)          — 文件底部 (生成 GetIO)
 *   5. .h 中用 MODULE_IO_H(模块名)    — 声明 GetIO 签名
 * ================================================================
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
    ST_NEW   = 0x02,   /* bit1: 新输入到达 (中间层在 InputCallback 中置位) */
    ST_OUT   = 0x04,   /* bit2: 输出就绪 (触发 OutputCallback, 一般不用) */
};

/* ========== 2. 信息头 ========== */
typedef struct {
    uint8_t status;   /* 组合 ST_INIT | ST_NEW | ST_OUT */
    uint8_t res1;
    uint8_t route;    /* 路由标识 — 多炉头/多模式时区分数据路径 */
    uint8_t res2;
} Info_Header;

/* ========== 3. 统一参数组 ========== */
typedef struct {
    Info_Header info;     /* 状态 + 路由 */
    void       *para;     /* 指向模块自定义 I/O 结构体 */
} Para_Grp_t;

/* ================================================================
 * 4. 骨架宏 — 每个模块 .c 文件调用一次
 *
 *   参数: name = 模块名 (PascalCase, 与 MODULE_EXPORT 一致)
 *
 *   生成内容:
 *     - static Para_Grp_t g_input / g_output   (统一起点)
 *     - static void Init(void)                  (模块实现)
 *     - static void ProcessInput(void)          (模块实现)
 *     - weak void name_InputCallback(void)      (中间层覆盖)
 *     - weak void name_OutputCallback(pOut)     (按需覆盖)
 *     - static void DoWork(void)                (Switcher 调度入口)
 *
 *   注意: 一个 .c 文件只能调用一次 (g_input/g_output 是 static 全局)
 * ================================================================ */
#define MODULE_SKELETON(name)                                            \
    static Para_Grp_t g_input;                                          \
    static Para_Grp_t g_output;                                         \
    static uint8_t    g_init_done = 0;                                  \
                                                                         \
    static void Init(void);                                              \
    static void ProcessInput(void);                                      \
                                                                         \
    __attribute__((weak)) void name##_InputCallback(void)                \
    { }                                                                  \
                                                                         \
    __attribute__((weak)) void name##_OutputCallback(Para_Grp_t *pOut)   \
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
        name##_InputCallback();                 /* ① 输入拉取 */        \
        ProcessInput();                         /* ② 消费→计算→产出 */ \
        if ((g_output.info.status & ST_OUT)) {                          \
            name##_OutputCallback(&g_output);   /* ③ 紧急输出 */        \
            g_output.info.status &= ~ST_OUT;                            \
        }                                                                \
    }

/* ================================================================
 * 5. 导出宏 — 生成 GetIO 注册入口
 *
 *   用法: MODULE_EXPORT(Power) → 生成 Power_GetIO(...)
 *   Switcher 通过此函数获取模块的 g_input/g_output/DoWork 指针
 * ================================================================ */
#define MODULE_EXPORT(module_name)                                       \
    void module_name##_GetIO(Para_Grp_t **ppIn,                          \
                             Para_Grp_t **ppOut,                         \
                             void      (**ppDoWork)(void)) {             \
        *ppIn     = &g_input;                                            \
        *ppOut    = &g_output;                                           \
        *ppDoWork = DoWork;                                              \
    }

/* ================================================================
 * 6. IO 头声明辅助宏 — 在 .h 中声明 GetIO 签名
 *
 *   用法: MODULE_IO_H(Power) → 声明 void Power_GetIO(...)
 *   替代手写声明, 避免模块名拼写不一致
 * ================================================================ */
#define MODULE_IO_H(module_name)                                         \
    void module_name##_GetIO(Para_Grp_t **ppIn,                          \
                             Para_Grp_t **ppOut,                         \
                             void      (**ppDoWork)(void))

#endif /* STD_MODULE_H */
