/**
 * @file    main_example.c
 * @brief   Switcher v2.0 范例 — 主入口
 *
 * 模拟 5 帧调度周期，演示完整数据流：
 *
 * 时序:
 *   T=0  Switcher_Init()      — 绑定指针，清零 status
 *   T=1  Switcher_Run()
 *         路由: (A 无产出) → 跳过
 *         A: 构造 → counter=43
 *         B: (无输入) → 跳过
 *         结果: A=43, B=跳过
 *   T=2  Switcher_Run()
 *         路由: A 有产出 (43) → 搬运到 B
 *         A: counter=44
 *         B: 消费 counter=43 → result=86
 *         结果: A=44, B=86
 *   T=3  Switcher_Run()
 *         路由: A 有产出 (44) → 搬运到 B
 *         A: counter=45
 *         B: 消费 counter=44 → result=88
 *         结果: A=45, B=88
 *
 * 核心特性:
 *   - 模块间零互知：A 不知道 B 存在，B 不知道 A 存在
 *   - 数据路由由 Switcher 集中管理
 *   - status bit0=构造, bit1=数据就绪
 *   - 一帧延迟：B 消费的是上一帧 A 的产出
 */
#include <stdio.h>
#include "test_incoming/example_switcher.h"

int main(void)
{
    printf("========================================\n");
    printf("  Switcher v2.0  —  A → B  范例\n");
    printf("========================================\n\n");

    /* ---- 初始化 ---- */
    ExampleSwitcher_Init();
    printf("[INIT] Switcher 初始化完成\n");
    printf("       A: Producer, B: Consumer\n");
    printf("       路由: A.Output_t → B.Input_t (一帧延迟)\n\n");

    /* ---- 5 帧调度 ---- */
    for (int frame = 1; frame <= 5; frame++) {
        ExampleSwitcher_Run();

        SwitcherDebug_t dbg;
        ExampleSwitcher_GetDebug(&dbg);

        printf("Frame %d | A.counter=%-4u (status=0x%02x) | B.consumed=%-4u",
               frame, dbg.a_counter, dbg.a_status, dbg.b_consumed);

        /* 简单校验 */
        if (frame >= 2) {
            unsigned expected = (41u + (unsigned)frame) * 2; /* A初始42, 每帧+1, B乘2 */
            printf(" %s", (dbg.b_consumed == expected) ? "[OK]" : "[MISMATCH]");
        } else {
            printf(" [B skipped — no data yet]");
        }
        printf("\n");
    }

    printf("\n========================================\n");
    printf("  Demo Complete\n");
    printf("========================================\n");
    return 0;
}
