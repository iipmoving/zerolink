# 专项审核：分层合理性与模块解耦

> 审核日期：2026-05-21
> 方法：逐文件追踪 `#include` 依赖链，绘制完整依赖图

---

## 一、当前依赖图（完整）

```
┌─────────────────────────────────────────────────────────────┐
│                         main.c                              │
│  (组装层: include所有模块, 注册回调, 主循环调度)               │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌──────────┐  ┌───────────┐  ┌───────────┐  ┌──────────┐ │
│  │drv_key   │  │drv_buzzer │  │drv_display│  │test_a/b  │ │
│  │  .c      │  │  .c       │  │  .c       │  │  .c      │ │
│  └────┬─────┘  └─────┬─────┘  └─────┬─────┘  └────┬─────┘ │
│       │              │              │              │        │
│  ╔════╪══════════════╪══════════════╪══════════════╪══════╗ │
│  ║    │     DRV层    │              │              │      ║ │
│  ╚════╪══════════════╪══════════════╪══════════════╪══════╝ │
│       │              │              │              │        │
│  ┌────┴─────┐  ┌─────┴─────┐  ┌─────┴─────┐  ┌───┴─────┐ │
│  │hal_key   │  │hal_buzzer │  │hal_display│  │hal_uart │ │
│  │  .c      │  │  .c       │  │  .c       │  │  .c     │ │
│  └────┬─────┘  └─────┬─────┘  └─────┬─────┘  └─────────┘ │
│       │              │              │                      │
│  ┌────┴─────┐  ┌─────┴─────┐  ┌─────┴─────┐  ┌─────────┐ │
│  │hal_timer │  │hal_comm   │  │hal_gpio   │  │hal_uart │ │
│  │  .c      │  │  .c       │  │  .c       │  │  .c     │ │
│  └──────────┘  └───────────┘  └───────────┘  └─────────┘ │
│                                                             │
│  ╔═══════════════════════════════════════════════════════╗   │
│  ║     HAL层: 每个.c只include自己的.h + 固件库            ║   │
│  ║     不include任何业务模块(除hal_buzzer.h外)            ║   │
│  ╚═══════════════════════════════════════════════════════╝   │
│                                                             │
│  ╔═══════════════════════════════════════════════════════╗   │
│  ║     CORE层(core/): 只依赖msg_def.h                     ║   │
│  ║     msg_scheduler.h → msg_def.h (无其他依赖)           ║   │
│  ╚═══════════════════════════════════════════════════════╝   │
└─────────────────────────────────────────────────────────────┘
```

---

## 二、分层完整性检查

### 2.1 正确的分层：HAL → DRV → APP

| 层级 | 文件职责 | 应该包含什么 | 应依赖什么 |
|------|---------|------------|-----------|
| **APP** | `src/main.c` | 初始化 + 主循环 + 回调函数 | 所有模块 |
| **TEST** | `test/*.c` | 测试模块 | CORE + HAL |
| **DRV** | `drv/*.c` | 业务逻辑封装（按键映射/音符序列/显示排版） | CORE(msg_scheduler) + HAL |
| **HAL** | `hal/*.c` | 硬件操作（寄存器/库函数） | 仅固件库（sc32f1xxx_*.h） |
| **CORE** | `core/*.c` | 消息调度器 | 仅 stdint/msg_def.h |

### 2.2 逐文件验证 ✅/❌

| 文件 | include了什么（除自身.h） | 层次是否正确 | 说明 |
|------|--------------------------|-------------|------|
| `core/msg_def.h` | `<stdint.h> <stddef.h>` | ✅ | 零项目依赖 |
| `core/msg_scheduler.h` | `"msg_def.h"` | ✅ | 只依赖core层 |
| `core/msg_scheduler.c` | (仅自己的.h) | ✅ | 无额外依赖 |
| `hal/hal_timer.c` | `sc32f1xxx_tim.h` `rcc.h` `sc32L14xx.h` | ✅ | 正确 |
| `hal/hal_uart.c` | `sc32f1xxx_uart.h` `rcc.h` `gpio.h` `sc32L14xx.h` | ✅ | 正确 |
| `hal/hal_comm.c` | `sc32f1xxx_uart.h` `dma.h` `rcc.h` `gpio.h` `sc32L14xx.h` | ✅ | 正确 |
| `hal/hal_gpio.c` | `sc32f1xxx_gpio.h` | ✅ | 正确 |
| `hal/hal_key.c` | `../lib/SC_TK_Scan.h` `../lib/MCU_Drivers/TKDriverNew/TKDriver.h` | ✅ | HAL访问硬件库 |
| `hal/hal_buzzer.c` | (仅自己的.h) | ✅ | 正确 |
| `hal/hal_display.c` | `sc32f1xxx_gpio.h` `rcc.h` | ✅ | 正确 |
| `drv/drv_key.c` | `../core/msg_scheduler.h` `../hal/hal_key.h` **`../lib/SC_TK_Scan.h`** | ❌ | 多include了一个 |
| `drv/drv_buzzer.c` | `../hal/hal_buzzer.h` `<stddef.h>` | ✅ | 正确 |
| `drv/drv_display.c` | `../lib/SMG_Disp_General_Lib.h` `../hal/hal_display.h` `<string.h>` | ⚠️ | 见说明 |
| `test/test_module_a.c` | `core/msg_scheduler.h` `hal/hal_uart.h` | ⚠️ | 见说明 |
| `test/test_module_b.c` | `core/msg_scheduler.h` `hal/hal_uart.h` | ⚠️ | 同上 |
| `src/main.c` | 所有hal + 所有drv + core + test | ✅ | APP组装层 |

---

## 三、发现的问题（按严重度排序）

### 🔴 P0 — 分层违规

#### 1. drv_key.c 直接include触摸库，绕过HAL

**位置**：`drv/drv_key.c` 第14行
```c
#include "../lib/SC_TK_Scan.h"
```

**分析**：
- `hal_key.h` 已经声明了 `uint32_t HAL_Key_Scan(void)` 封装了触摸库
- `hal_key.c` 中 `HAL_Key_Scan()` 调用了 `Sys_Scan()`
- drv_key.c 中**没有任何代码**直接使用 SC_TK_Scan.h 中的符号
- 这个 include 是完全多余的

**影响**：
- DRV层绕过了HAL层的封装
- 如果未来更换触摸库，SC_TK_Scan.h 路径变了，drv_key.c 也要改
- HAL 层的封装失去了意义

**修复**：直接删除第14行 `#include "../lib/SC_TK_Scan.h"`

#### 2. hal_buzzer.h 公开12个extern全局变量，DRV层直接读写

**位置**：`hal/hal_buzzer.h` 第101-120行
```c
extern uint8_t  g_buzz_my_active;
extern uint16_t g_buzz_my_time_on;
...
extern uint8_t  g_buzz_count;
extern uint8_t  g_buzz_hz_timer;
...
```

**分析**：
- `hal_buzzer.c` 中定义（第18-32行）
- `drv_buzzer.c` 中直接读写（如 `g_buzz_count = 0u;` 第221行）

**影响**：
- HAL 层和 DRV 层通过**extern全局变量耦合**，不是通过函数接口
- 违反"模块不暴露全局变量"的架构原则
- 任何 include 了 hal_buzzer.h 的文件都能读写这些变量，无法控制访问权限
- 如果未来要替换蜂鸣器硬件，需要同时改 HAL 和 DRV

**修复建议**：
- 选项A（推荐）：合并 `hal_buzzer` 和 `drv_buzzer` 为一个模块
- 选项B：在 hal_buzzer.h 中定义 setter/getter 函数，移除外extern声明
- 选项C：drv_buzzer通过消息（MSG_BUZZER_CTRL）控制蜂鸣器

---

### 🟠 P1 — 分层界限模糊

#### 3. HAL层的.h文件直接include固件库

**位置**：
- `hal/hal_buzzer.h` 第19-21行
```c
#include "sc32f1xxx_gpio.h"
#include "sc32f1xxx_tim.h"
#include "sc32f1xxx_rcc.h"
```
- `hal/hal_gpio.h` 第12行
```c
#include "sc32f1xxx_gpio.h"
```

**分析**：
- 通常 `.h` 文件只放接口声明，固件库头文件应由 `.c` 文件引入
- 当前设计是因为宏定义（如 `HAL_BUZZ_PWM_SET`）用到了固件库类型
- 但后果：任何 include 了 `hal_buzzer.h` 的文件（如 drv_buzzer.c 和 main.c）都间接依赖了固件库

**影响**：
- DRV层通过HAL的头文件间接依赖了MCU固件库
- 如果要移植到其他MCU，HAL层.h文件也要改
- 但这是嵌入式开发常见做法，控制在一个合理的可接受范围内

**建议**：非紧急，后续可将宏改为内联函数或放到.c文件中

#### 4. test模块直接include HAL层的调试串口

**位置**：`test/test_module_a.c` 第14行 / `test/test_module_b.c` 第12行
```c
#include "hal/hal_uart.h"
```

**分析**：
- 测试模块通过 `HAL_UART_Debug_Print()` 直接输出到串口
- 没有通过消息调度器

**影响**：测试模块是顶层模块，测试期间直接调用HAL做调试输出是可以接受的。正式发布时这些测试模块不会被编译进固件。

**建议**：保留，测试模块的特性就是直接访问资源。

#### 5. drv_display.c 直接include SMG算法库

**位置**：`drv/drv_display.c` 第24行
```c
#include "../lib/SMG_Disp_General_Lib.h"
```

**分析**：
- SMG_Disp_General_Lib.h 是段码转换算法库，不是硬件驱动
- 放在 lib/ 目录下，由 DRV 层直接调用是合理的
- 但跟 HAL 的关系需要明确：HAL 管 GPIO 操作，SMG 库管段码算法，DRV 管排版布局
- 目前的分工是清晰的

**结论**：✅ 可接受。算法库和硬件库不同，DRV直接调用算法库是嵌入式项目常态。

---

### 🔵 P2 — 模块间未通过消息通信

#### 6. main.c 的 Key_OnEvent 直接调用 drv 函数

**位置**：`src/main.c` 第26-65行
```c
static void Key_OnEvent(...) {
    Drv_Display_ShowKey(key_code, key_state);  // 直接调用
    ...
    Drv_Buzzer_Select(...);  // 直接调用
}
```

**分析**：
- 按键事件通过消息调度器分发 ✅
- 但处理函数中直接调用了 drv_display 和 drv_buzzer ❌
- 根据架构，display 和 buzzer 应该响应消息（如 MSG_DISPLAY_REFRESH、MSG_BUZZER_CTRL）

**建议**：后续引入 json_ui.c 后，将 Key_OnEvent 中的显示和蜂鸣器调用改为消息投递。

---

## 四、依赖违规汇总

| 违规 | 文件 | 多余/缺失的依赖 | 类型 |
|------|------|----------------|------|
| 1 | `drv/drv_key.c` | 多余 `#include "../lib/SC_TK_Scan.h"` | **必须删** |
| 2 | `hal/hal_buzzer.h` | 12个 extern 全局变量，DRV绕过接口直接读写 | **必须改** |
| 3 | `hal/hal_gpio.h` | 在.h中include固件库（.c再include一次） | 可改可不改 |
| 4 | `hal/hal_buzzer.h` | 在.h中include 3个固件库头文件 | 可改可不改 |
| 5 | `test/*.c` | include hal/hal_uart.h（测试模块特例） | 接受 |
| 6 | `src/main.c` | 直接调drv函数（应改为消息） | 后续修复 |

---

## 五、关于解耦的评估

### 好消息 ✅
- **DRV模块之间无相互include** — `drv_key.c` 不 include `drv_display.h`，`drv_buzzer.c` 不 include `drv_key.h`，正确
- **HAL模块之间无相互include** — 所有hal_*.c仅include自己的.h和固件库，正确
- **CORE层不依赖任何业务模块** — msg_scheduler.c 不知道 buzzer/display/key 的存在
- **依赖方向正确** — APP → DRV → HAL → 固件库，永远是单向

### 不好的消息 ❌
- **存在全局变量跨层耦合** — `g_buzz_*` 在 hal_buzzer 定义、drv_buzzer 读写
- **存在多余的include** — drv_key.c 多了一行 SC_TK_Scan.h，应该由HAL层独享
- **消息通信不完整** — 按键处理函数中直接调用了 buzzer 和 display，没有走消息通道

---

## 六、结论

**总体评分：中等。** 架构骨架正确（单向依赖、模块分离、消息调度），但有两处具体代码违背了设计原则。

**必须立即修复：**
1. drv_key.c 删除多余的 `#include "../lib/SC_TK_Scan.h"`
2. hal_buzzer.h 的12个 extern 全局变量改为函数接口

**建议后续修复：**
3. 待 json_ui.c 引入后，将 buzzer/display 的控制改为消息投递
