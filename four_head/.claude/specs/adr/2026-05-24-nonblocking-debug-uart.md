# ADR: 非阻塞调试打印 (Non-Blocking Debug Printf)

**日期**: 2026-05-24
**状态**: 已实施
**决策者**: 技术负责人

---

## 背景

UART3 调试串口使用阻塞式 `HAL_UART_Debug_Print()` —— 每字符 `while(!TX_FLAG){}` 轮询。57600bps 下每字符约 174us，40 字符日志卡主循环约 7ms，破坏 1ms 分时调度。

## 决策

**新增 `HAL_UART_Debug_Printf(fmt, ...)`，通过条件编译切换阻塞/非阻塞实现。**

### 方案

- **阻塞模式** (`DEBUG_PRINT_MODE_BLOCKING`): `vsnprintf` → `PutChar` 逐字节轮询。与原 API 行为一致。
- **非阻塞模式** (`DEBUG_PRINT_MODE_NONBLOCKING`): `vsnprintf` → 256 字节环形缓冲 → UART3 TX 中断逐字节发送。

### 条件编译开关

```c
#define DEBUG_PRINT_MODE_BLOCKING     0u
#define DEBUG_PRINT_MODE_NONBLOCKING  1u
#ifndef DEBUG_PRINT_MODE
#define DEBUG_PRINT_MODE  DEBUG_PRINT_MODE_NONBLOCKING  /* 默认非阻塞 */
#endif
```

### 为什么不用 DMA

SC32L14xx DMA 控制器只支持 UART0/UART1 的 DMA 请求，UART3 无 DMA 支持。TX 中断是唯一可用的非阻塞方案。

### 为什么复用现有文件而不是新建 `hal_debug.c`

`hal_uart.c` 已有 `Flush()` 和 `ISR()` 空桩函数，主循环已调用 `Flush()`。复用这些 hook 点零侵入。

## 后果

- 新增 API 3 个: `Printf` / `Print_NB` / `TxBusy`
- 现有 API 完全不变
- 调用方代码无需修改（只需 `#include "hal_uart.h"` 后使用 `HAL_UART_Debug_Printf()`）
- 环形缓冲 256 字节常驻 SRAM（非阻塞模式）
- ISR 每字节触发一次（无硬 FIFO），但执行时间极短（~2us 取字节+发送）
