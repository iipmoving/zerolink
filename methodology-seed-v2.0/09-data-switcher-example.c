/* ===================================================================
 * 09 — 数据交换机完整范式：两模块 + 中间层 (v2.0)
 *
 * 核心:
 *   - 每个模块有 Input_t + Output_t, 嵌 status 字节 (bit1=新数据)
 *   - _io.h 保留 #define guard (公开接口, 两个合法 include 方)
 *   - 模块 DoWork: 先清自己 g_out bit1 → 检查 g_in bit1 → 消费 → 计算 → 置 g_out bit1
 *   - Switcher: 检测 producer g_out bit1 → 搬运字段到 consumer g_in → 清 producer bit1 → 设 consumer bit1
 *   - 模块只知道自己的 status 字节, Switcher 是唯一跨模块操作 status 的角色
 *
 * 本文件演示:
 *   Module ADC     — 纯生产者: 采数据 → 置 g_out bit1
 *   Module Power   — 纯消费者: 检查 g_in bit1 → 消费 → 置 g_out bit1
 *   Switcher       — 接线员: 搬运 Output_t 字段 → Input_t 字段, 管理所有 status 字节
 *   Task           — 只调 __weak, 不知道模块存在
 *
 * 实际项目中分布在独立文件里，这里合并便于阅读。
 * ================================================================= */

#include <stdint.h>
#include <string.h>


/* ===================================================================
 * §1 模块对外接口 (_io.h)
 *
 * 规则:
 *   - _io.h 保留 #define guard (公开接口, 本模块 .c + Switcher.c 两个合法 include 方)
 *   - _io.h 不加入编译器 -I path → 所有人都用全路径 include
 *   - 模块自身 .h (如有) 注释 //#define (私有, L0 编译阻断)
 * ================================================================= */

/* --------------------- include/adc_io.h ----------------------------- */
#ifndef ADC_IO_H
#define ADC_IO_H       /* ← 保留: _io.h 是公开接口 */

#include <stdint.h>

#pragma pack(4)

/* ADC 是纯生产者 — 只有 Output, 没有 Input */
typedef struct {
    uint8_t  status;        /* bit0=保留, bit1=新数据就绪 (本模块设, Switcher 清) */
    uint8_t  res[3];
    uint16_t voltage;       /* mV */
    uint16_t current;       /* mA */
} Adc_Output_t;
/* sizeof=8, 32位对齐 */

#pragma pack()

void Adc_GetIO(Adc_Output_t **ppOut);
void Adc_DoWork(void);

#endif /* ADC_IO_H */


/* --------------------- include/power_io.h --------------------------- */
#ifndef POWER_IO_H
#define POWER_IO_H       /* ← 保留: _io.h 是公开接口 */

#include <stdint.h>

#pragma pack(4)

/* Power 从 ADC 取电压/电流, 从外部取模式 */
typedef struct {
    uint8_t  status;        /* bit0=构造, bit1=新输入到达 (Switcher 设, 本模块消费后清) */
    uint8_t  res[3];
    uint16_t voltage;       /* mV — 来自 Adc_Output_t.voltage */
    uint16_t current;       /* mA — 来自 Adc_Output_t.current */
    uint8_t  op_mode;       /* 运行模式 — 来自外部 (示例: 硬编码) */
} Power_Input_t;
/* sizeof=12 */

typedef struct {
    uint8_t  status;        /* bit0=保留, bit1=输出就绪 (本模块设, Switcher 清) */
    uint8_t  res[3];
    uint16_t power;         /* mW */
    uint8_t  overcurrent;
    uint8_t  mode_active;
} Power_Output_t;
/* sizeof=8 */

#pragma pack()

void Power_GetIO(Power_Input_t **ppIn, Power_Output_t **ppOut);
void Power_DoWork(void);

#endif /* POWER_IO_H */


/* ===================================================================
 * §2 模块实现
 *
 * 模块只操作自己的 status:
 *   - DoWork 进入时: g_out.status &= ~0x02 (每帧先清)
 *   - 消费 g_in 后:   g_in.status  &= ~0x02 (消费完毕)
 *   - 有新产出时:     g_out.status |=  0x02
 *
 * 关键: 模块不知道 Switcher 存在, 不知道谁在写 g_in、谁在读 g_out.
 * ================================================================= */

/* ======================== adc.c ================================== */
#include "../include/adc_io.h"       /* 全路径 — 自己的 _io.h */

static Adc_Output_t g_out;
static uint8_t      _tick = 0;       /* 模拟 100ms 采一次 */

void Adc_GetIO(Adc_Output_t **ppOut)
{
    *ppOut = &g_out;
}

void Adc_DoWork(void)
{
    /* ---- 每帧先清输出标志 ---- */
    g_out.status &= ~0x02;

    /* ---- 计算段 ---- */
    _tick++;
    if (_tick < 10) return;          /* 没到 100ms, 本帧无产出 */
    _tick = 0;

    g_out.voltage = 12000;           /* 模拟采样: 12V */
    g_out.current = 2500;            /* 模拟采样: 2.5A */

    /* ---- 输出段: 只设自己的标志 ---- */
    g_out.status |= 0x02;            /* "我有新数据" */
}


/* ======================== power.c ================================ */
#include "../include/power_io.h"      /* 全路径 — 自己的 _io.h */
/* 不 include adc_io.h — 不知道 ADC 的存在 */

static Power_Input_t  g_in;
static Power_Output_t g_out;

static void _Constructor(void)
{
    memset(&g_in,  0, sizeof(g_in));
    memset(&g_out, 0, sizeof(g_out));
}

void Power_GetIO(Power_Input_t **ppIn, Power_Output_t **ppOut)
{
    *ppIn  = &g_in;
    *ppOut = &g_out;
}

void Power_DoWork(void)
{
    /* ---- 构造: lazy init ---- */
    if (!(g_in.status & 0x01)) {
        _Constructor();
        g_in.status |= 0x01;
    }

    /* ---- 每帧先清输出标志 ---- */
    g_out.status &= ~0x02;

    /* ---- 输入段: 检查 Switcher 填入的输入 ---- */
    if (!(g_in.status & 0x02)) return;  /* 无新输入, 本帧跳过 */

    /* ---- 计算段: 消费 g_in ---- */
    g_out.power       = (uint32_t)g_in.voltage * g_in.current / 1000;
    g_out.overcurrent = (g_in.current > 5000) ? 1 : 0;
    g_out.mode_active = g_in.op_mode;

    /* ---- 消费完毕, 清输入标志 ---- */
    g_in.status &= ~0x02;

    /* ---- 输出段 ---- */
    g_out.status |= 0x02;              /* "我有新输出" */
}


/* ===================================================================
 * §3 中间层 Switcher
 *
 * 全项目唯一有权 include 所有 _io.h 的文件.
 * 职责:
 *   - Init: 收所有模块的 GetIO 指针, 上电清零
 *   - Run:  检查 producer g_out bit1 → 搬运字段到 consumer g_in →
 *           清 producer bit1 → 设 consumer bit1 → 调各模块 DoWork
 *
 * 标志流转全程:
 *   producer DoWork → 设自己 g_out bit1
 *   Switcher 检测 producer g_out bit1 → 搬运 → 清 producer bit1 → 设 consumer g_in bit1
 *   consumer DoWork → 检查自己 g_in bit1 → 消费 → 清自己 g_in bit1
 *
 * 没有一个模块操作另一个模块的 status 字节.
 * ================================================================= */

/* ==================== data_switcher.c ============================ */
#include "../include/adc_io.h"        /* 特权: 全路径 include 所有 _io.h */
#include "../include/power_io.h"

static Adc_Output_t   *pAdc_Out;
static Power_Input_t  *pPower_In;
static Power_Output_t *pPower_Out;

void Switcher_Init(void)
{
    Adc_GetIO(&pAdc_Out);
    Power_GetIO(&pPower_In, &pPower_Out);

    /* 上电清所有模块输入/输出 status, 模块首次 DoWork 自检 bit0=0 → _Constructor() */
    pAdc_Out->status   = 0;
    pPower_In->status  = 0;
    pPower_Out->status = 0;
}

/* ---- 接线: 从 producer 输出槽搬运字段到 consumer 输入槽 ---- */
static void _Route_Adc_to_Power(void)
{
    if (pAdc_Out->status & 0x02) {               /* producer 有新数据? */
        /* 按接线规则搬运字段 (可包含转换、聚合) */
        pPower_In->voltage   = pAdc_Out->voltage;
        pPower_In->current   = pAdc_Out->current;
        pPower_In->op_mode   = 1;                 /* 示例: 硬编码模式 */
        pAdc_Out->status    &= ~0x02;             /* 清 producer — 数据已取走 */
        pPower_In->status   |=  0x02;             /* 通知 consumer — 新输入到达 */
    }
}

/* ---- 多源汇聚示例 (从多个 producer 填一个 consumer Input_t) ----
static void _Route_All_to_Power(void)
{
    // ADC 数据
    if (pAdc_Out->status & 0x02) {
        pPower_In->voltage = pAdc_Out->voltage;
        pPower_In->current = pAdc_Out->current;
        pAdc_Out->status &= ~0x02;
        pPower_In->status |= 0x02;     // 设一次就够了
    }
    // 温度模块数据 → 填充 pPower_In 的其他字段
    // ...
}
------------------------------------------------------------------ */


/* ===================================================================
 * §4 调度层
 *
 * Task 只通过 __weak 调 Switcher, 不知道任何模块的存在.
 * ================================================================= */

/* ==================== app_task.c ================================== */

__attribute__((weak)) void Switcher_Run_Slot2(void) {}

void Task_TimeChip2(void)
{
    Switcher_Run_Slot2();   /* __weak — 链接器接线, task 不知道接收方 */
}

/* ==================== data_switcher.c (续) ======================= */

/* STRONG — 覆盖 task 的 __weak */
void Switcher_Run_Slot2(void)
{
    /* 1. producer: 有产出 → 置 g_out bit1 */
    Adc_DoWork();

    /* 2. 路由: 检测 producer bit1 → 搬运 → 清 producer bit1 → 设 consumer bit1 */
    _Route_Adc_to_Power();

    /* 3. consumer: 检查 g_in bit1 → 消费 → 清 bit1 → 计算 → 置 g_out bit1 */
    Power_DoWork();
}


/* ===================================================================
 * §5 完整时序
 *
 * 标志流转 (一个生产-消费周期):
 *
 *   帧 N:
 *     Adc_DoWork:        tick=10 → 采数据 → g_out.status |= 0x02
 *     _Route_Adc:        pAdc_Out->status bit1=1 → 搬运字段到 pPower_In
 *                        → 清 pAdc_Out->status bit1
 *                        → 设 pPower_In->status bit1=1
 *     Power_DoWork:      g_in.status bit1=1 → 消费 → g_in.status &= ~0x02
 *                        → 计算 → g_out.status |= 0x02
 *
 *   帧 N+1..N+9 (ADC 无产出):
 *     Adc_DoWork:        tick < 10 → return (g_out.status bit1=0)
 *     _Route_Adc:        pAdc_Out->status bit1=0 → 跳过
 *     Power_DoWork:      g_in.status bit1=0 → return (无新输入)
 *
 * 时序保证: consumer 消费的是上一帧 producer 产出 (自然延迟一帧).
 * 10ms 轮转确保每轮数据都是新的.
 *
 * ================================================================= */


/* ===================================================================
 * §6 与 __weak 模式的对比
 *
 * __weak 模式 (v1.0):
 *   - 发送方声明 __weak void Receiver_OnData(type *p) {}
 *   - 发送方调 Receiver_OnData(&data)
 *   - 接收方 STRONG Receiver_OnData → 搬运到本地
 *   - consumer 需要 _LINK_t 副本 (双向维护)
 *   - void* 解耦 (调用方不知道接收方类型)
 *
 * 数据交换机 (v2.0):
 *   - 每个模块定义 Input_t / Output_t 在 _io.h
 *   - Switcher 是唯一跨模块节点, 全路径 include 所有 _io.h
 *   - Switcher 直接搬运 Output_t 字段 → Input_t 字段 (有类型安全)
 *   - 模块只操作自己的 status 字节
 *   - consumer 不需要 _LINK_t 副本
 *   - 不需要 void* — Switcher 持有所有类型定义
 *
 * 共存:
 *   - ISR 回调 (延迟敏感)        → 仍走 __weak
 *   - 同槽同步计算 (调用即执行)   → 仍走 __weak
 *   - 周期性结构化数据路由        → 数据交换机
 * ================================================================= */


int main(void)
{
    Switcher_Init();

    while (1) {
        Task_TimeChip2();
    }
    return 0;
}
