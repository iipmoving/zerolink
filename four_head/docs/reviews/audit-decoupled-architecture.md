# 四头电磁炉低耦合程序架构 — 审计报告

**审计日期**: 2026-05-21  
**基准**: CLAUDE.md + memory/MEMORY.md 全部约束条件  
**范围**: `four_head_ih_cooker/Claude/` 全部源文件

---

## 一、审计结果总表

| # | 类别 | 位置 | 描述 | 严重度 |
|---|------|------|------|--------|
| 1 | 分层泄漏 | drv_key.c:14,143-144 | DRV 绕过 HAL 直接调用 `Sys_Scan()` + `Bit_SYS_Read_Key`，`HAL_Key_Scan()` 成死代码 | **严重** |
| 2 | 分层泄漏 | drv_display.c:24 | DRV 直接 include `../lib/SMG_Disp_General_Lib.h`，应经 HAL 包装 | **中等** |
| 3 | 全局变量 | hal_buzzer.h:104-120 | 12 个 `extern` 全局变量暴露 HAL↔DRV 紧耦合，违反"无全局变量"原则 | **中等** |
| 4 | 时序 | drv_key.c:143 | `Sys_Scan()` 每 10ms 调用一次（原设计 1ms），21 通道全扫描约 210ms，触摸延迟可感知 | **中等** |
| 5 | 可调参数 | drv_buzzer.c:301-321,354 | 蜂鸣时序值隐藏在 switch 中（应 `#define` 至 .h） | **中等** |
| 6 | 可调参数 | drv_display.c:138,145 | 刷新区间 `50u`/`10u` 隐藏在 .c（应 `#define` 至 .h） | **中等** |
| 7 | HAL 耦合 | hal_timer.c:15 + 74 行 | `hal_timer.c` include `hal_key.h`；注释的 `HAL_Key_Poll()` 若启用则 ISR 违规 | **中等** |
| 8 | 文档 | hal_uart.h:6-7 | 声称"中断驱动 TX / 环缓冲"，实际为阻塞式 | **次要** |
| 9 | HAL 状态 | hal_display.c:78 | 静态 `s_prev_com` 造成 HAL 持久状态，建议改为参数传递 | **次要** |

**通过项**（无违规）：
- 业务模块零交叉 include（test_module_a/b 仅通过 Msg_Post 通信）
- HAL 层不 include 任何业务头文件
- DRV 层之间零交叉 include
- msg_def.h 零依赖（仅 stdint.h + stddef.h）
- ISR 合规（TIM0 只设标志，TIM1 只翻 GPIO，UART/DMA ISR 只清标志）
- COM 扫描序列正确（关旧→更新→开新，每 1ms）
- 系统时钟配置与 CLAUDE.md 一致（HIRC 48MHz, HCLK/APB Div1）
- 蜂鸣器引脚 PE1/PE4 与参考程序一致
- 显示 8SEG+11COM 引脚与 CLAUDE.md 一致
- 调试串口 UART3（APB2, 57600, PinRemap_A）正确
- MODBUS UART0（DMA0 RX + DMA1 TX, 20ms 帧检测）正确
- 可调参数 `drv_key.h` 中的 KEY_DEBOUNCE_CNT/KEY_LONG_THRESH/KEY_REPEAT_PERIOD 已在 .h

---

## 二、逐项详解

### 问题 1: DRV 绕过 HAL 直接调用触摸库（严重）

**文件**: [drv_key.c](four_head_ih_cooker/Claude/drv/drv_key.c#L14)

```c
#include "../lib/SC_TK_Scan.h"    /* ← DRV 直接 include 库头文件 */
...
raw_mask = Sys_Scan();             /* ← 绕过 HAL 调用库函数 */
if (!Bit_SYS_Read_Key) return;    /* ← 库全局变量直接访问 */
```

**违反规则**: `DRV层只include msg_scheduler.h + HAL接口`

**后果**: `hal_key.c` 中的 `HAL_Key_Scan()` 成死代码，HAL 触摸抽象形同虚设。任何触摸库升级都需同时修改 DRV 和 HAL。

**修复方向**: `drv_key.c` → `HAL_Key_Scan()` → 内部调 `Sys_Scan()`/`Bit_SYS_Read_Key`，DRV 不感知库符号。

---

### 问题 2: DRV 直接 include SMG 显示库（中等）

**文件**: [drv_display.c](four_head_ih_cooker/Claude/drv/drv_display.c#L24)

```c
#include "../lib/SMG_Disp_General_Lib.h"
```

**违反规则**: 同问题 1，SMG 库应经 HAL 包装。

**修复方向**: 创建 `hal_smg.h/.c` 封装 SMG 库的 `SMG_Init()` / `SMG_GetSegData()` 等函数，DRV 通过 HAL 调用。

---

### 问题 3: hal_buzzer.h 公开 12 个全局变量（中等）

**文件**: [hal_buzzer.h](four_head_ih_cooker/Claude/hal/hal_buzzer.h#L104-L120)

```c
extern uint8_t  g_buzz_my_active;
extern uint16_t g_buzz_my_time_on;
extern uint16_t g_buzz_my_time_off;
extern uint8_t  g_buzz_my_step;
extern uint8_t  g_buzz_my_step_init;
extern uint8_t  g_buzz_my_mode;
extern uint8_t  g_buzz_count;
extern uint8_t  g_buzz_hz_timer;
extern uint8_t  g_buzz_jiange;
extern uint8_t  g_buzz_hz_timer_hc;
extern uint8_t  g_buzz_jiange_hc;
extern uint8_t  g_buzz_on_delay;
extern uint8_t  g_buzz_off_flag;
```

**违反规则**: 项目"无全局变量"原则

**分析**: 这些变量是 HAL 和 DRV 之间的共享状态。DRV 直接读写 `extern` 变量形成紧耦合——变量名或类型任何改动都破坏 DRV。虽然 ISR 路径确需零开销访问（GPIO toggle 在 ~100us 级），但当前所有 12 个变量均无条件暴露。

**修复方向**: 保留 ISR 关键变量（`g_buzz_my_active`, `g_buzz_hz_timer`）为 extern，其余改为 HAL 内 `static` + DRV 通过 setter/getter 函数访问。消隐逻辑（`g_buzz_on_delay`/`g_buzz_off_flag`）应封装在 DRV 内部。

---

### 问题 4: 触摸扫描时序（中等）

**文件**: [drv_key.c](four_head_ih_cooker/Claude/drv/drv_key.c#L143)

**现状**: `Sys_Scan()` 每 10ms 调用一次（在槽位 3 `Drv_Key_Scan()` 中）

**分析**: 21 通道 × 10ms = 210ms 全扫描周期。触摸库 `Sys_Scan()` 每调用一次推进一个通道的状态机，按参考程序设计应在 TIM0 ISR（125us）中每 125us 驱动一次（全扫描 2.6ms）。用户明确要求"ISR 中不用调 SYS_SCAN，只需要 10ms 去执行一次"——当前设计符合用户意图。但若后续发现触摸响应偏慢，这是优化方向。

---

### 问题 5/6: 可调参数未在 .h 中定义（中等）

**蜂鸣器** — [drv_buzzer.c:301-321](four_head_ih_cooker/Claude/drv/drv_buzzer.c#L301)

每个音效的 `(count, on_time, interval)` 三元组直接硬编码在 `switch` 中：
```c
case DRV_BUZZ_KEY:  Buzz_Dispose_Set(1u,  20u,   0u); break;
case DRV_BUZZ_SD:   Buzz_Dispose_Set(1u,  30u,  10u); break;
```
消隐窗口 `g_buzz_on_delay = 150u` 同样隐藏在第 354 行。

**显示** — [drv_display.c:138,145](four_head_ih_cooker/Claude/drv/drv_display.c#L138)

```c
if (s_flash_cnt >= 50u)   /* 500ms/10ms —— 应 #define FLASH_PERIOD_10MS 50u */
if (s_update_cnt >= 10u)  /* 100ms/10ms —— 应 #define UPDATE_PERIOD_10MS 10u */
```

**违反规则**: `memory/feedback_tunable-params-in-h.md`: 所有模块的可调常数必须在 .h 定义

---

### 问题 7: hal_timer.c → hal_key.h 跨 HAL 耦合（中等）

**文件**: [hal_timer.c](four_head_ih_cooker/Claude/hal/hal_timer.c#L15)

```c
#include "hal_key.h"
```

时基定时器 HAL 不应依赖特定外设。若 `HAL_Key_Poll()` 恢复启用，TIM0 ISR 将直接调触摸状态机，违反"ISR 只设时基标志"规则。

**修复方向**: 移除该 include。若需调 `HAL_Key_Poll()`，从主循环 1ms 路径调用，而非从 ISR。

---

## 三、分层依赖关系图（现状）

```
main.c (组合根)
 ├── hal/  (HAL 层 —— 无业务依赖 ✓)
 │   ├── hal_timer.c  → hal_key.h  ← ⚠ C6: 跨 HAL 耦合
 │   ├── hal_buzzer.h → extern ×12  ← ⚠ 问题 3: 全局变量暴露
 │   └── hal_key.c    → HAL_Key_Poll() 空函数  ← ⚠ 问题 1
 │
 ├── drv/  (DRV 层)
 │   ├── drv_key.c     → ../lib/SC_TK_Scan.h  ← ⚠ 问题 1: 绕过 HAL
 │   ├── drv_display.c → ../lib/SMG_...Lib.h  ← ⚠ 问题 2: 绕过 HAL
 │   └── drv_buzzer.c  → hal_buzzer externs   ← ⚠ 问题 3: 全局变量
 │
 ├── core/ (消息调度器 —— 零依赖 ✓)
 │   └── msg_def.h  → stdint.h + stddef.h only ✓
 │
 └── test/ (测试模块 —— 纯消息通信 ✓)
     ├── test_module_a ←→ test_module_b  (via MSG_TEST_CHAT_A/B only) ✓
     └── 零交叉 include ✓
```

---

## 四、已确认合规项

| 规则 | 状态 |
|------|------|
| 业务模块零交叉 include | ✓ 通过 |
| HAL 不 include 业务头文件 | ✓ 通过 |
| DRV 之间零交叉 include | ✓ 通过 |
| msg_def.h 零依赖 | ✓ 通过 |
| ISR 只设时基标志 | ✓ 通过 (TIM0/TIM1/UART) |
| 10 槽 1ms 轮转调度 | ✓ 通过 |
| COM 每 1ms 扫描（关旧→更新→开新） | ✓ 通过 |
| 模块自计数计时（不依赖 hal_timer） | ✓ 通过 |
| 硬件配置与参考程序一致 | ✓ 通过 |
| 可调参数 drv_key.h 已在 .h | ✓ 通过 |
| 系统时钟 HIRC 48MHz, HCLK/APB Div1 | ✓ 通过 |

---

## 五、待完成模块

| 模块 | 描述 | 状态 |
|------|------|------|
| proto | MODBUS 协议解析 | 未开始 |
| json_ui | JSON 引擎（UI 数据绑定） | 未开始 |
| cook | 烹饪逻辑（加热控制/功率管理） | 未开始 |
| actuator | 执行器（加热 IO/风机/保护） | 未开始 |

---

## 六、修复优先级

1. **问题 1** (严重) — 触摸 HAL 重建：`Sys_Scan()` 封装入 `hal_key.c`，DRV 只调 `HAL_Key_Scan()`
2. **问题 3** (中等) — 全局变量收敛：仅 ISR 关键变量保留 extern，其余通过函数访问
3. **问题 5+6** (中等) — 可调参数提取到 .h：蜂鸣时序 + 刷新周期
4. **问题 2** (中等) — SMG 库 HAL 包装
5. **问题 7** (中等) — 移除 hal_timer.c → hal_key.h 耦合
6. **问题 4** (中等) — 触摸扫描时序：若实际响应延迟，调整调用频率
7. **问题 8+9** (次要) — 文档修正 + HAL 状态无状态化
