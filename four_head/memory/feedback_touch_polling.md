---
name: touch-polling-10ms-not-125us
description: "触摸库自带中断, HAL_Key_Poll 不需要 125us 轮询, 10ms 读一次即可"
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 8468b018-0dc2-4dea-925a-f289e7ef9c0b
---

触摸库 (SC_TK_Scan) 自带硬件中断完成通道扫描, `Sys_Scan()` 只需每 10ms 调用一次读取结果。

**Why:** 用户明确指出触摸库"有它自己中断，你只需要10ms读一次"。`HAL_Key_Poll()` 在 TIM0 ISR (125us) 中应保持注释状态, 不要取消注释。

**How to apply:** 修改按键相关代码时, 不要动 `hal_timer.c` 中的 `HAL_Key_Poll()` 注释。触摸时序问题应检查 `drv_key.c` 的去抖/长按/松手逻辑, 而非调整 ISR 轮询频率。
