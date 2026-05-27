# Claude代码二次审核报告

> 审核日期：2026-05-21
> 审核范围：Claude项目全部应用层代码
> 对比基准：4份MD规格文档 + AI提示词
> 本次新增审核：`drv_buzzer`/`hal_buzzer`/`drv_display`/`hal_display`

---

## 一、总体进度

| 模块 | 状态 | 文件数 | 完成度变化 |
|------|------|--------|-----------|
| `core/msg_def.h` | ✅ 完成 | 1 | ← |
| `core/msg_scheduler.c/.h` | ✅ 完成 | 2 | ← |
| `hal/hal_timer.c/.h` | ✅ 完成 | 2 | ← |
| `hal/hal_uart.c/.h` (UART3 DEBUG) | ✅ 完成 | 2 | ← |
| `hal/hal_comm.c/.h` (UART0 DMA) | ✅ 完成 | 2 | ← |
| `hal/hal_gpio.c/.h` | ✅ 完成 | 2 | ← |
| `hal/hal_key.c/.h` | ✅ 完成 | 2 | ← |
| `hal/hal_buzzer.c/.h` | ✅ **新增** | 2 | 🆕 TIM1 PWM + 双模式驱动 |
| `hal/hal_display.c/.h` | ✅ **新增** | 2 | 🆕 8 SEG + 11 COM动态扫描 |
| `drv/drv_key.c/.h` | ✅ 完成 | 2 | ← |
| `drv/drv_buzzer.c/.h` | ✅ **新增** | 2 | 🆕 普通+美声双模式 |
| `drv/drv_display.c/.h` | ✅ **新增** | 2 | 🆕 SMG库封装+双缓冲 |
| `src/main.c` | ⚠️ 更新 | 1 | ← 集成display+buzzer |
| `doc/hw_ref.md` | ✅ 完成 | 1 | ← |
| `test/test_module_a/b` | ✅ 完成 | 4 | ← 测试对话Demo |

| 未实现模块 | 预期文件 | 状态 |
|-----------|---------|------|
| `proto.c` (MODBUS协议) | `inc/proto.h` + `src/proto.c` | ❌ 未实现 |
| `json_ui.c` (JSON驱动UI) | `inc/json_ui.h` + `src/json_ui.c` | ❌ 未实现 |
| `cook.c` (烹饪逻辑) | `inc/cook.h` + `src/cook.c` | ❌ 未实现 |
| `actuator.c` (执行器) | `inc/actuator.h` + `src/actuator.c` | ❌ 未实现 |

---

## 二、上次P0/P1问题的修复情况

| 上次问题 | 优先级 | 修复状态 | 当前情况 |
|---------|--------|---------|---------|
| 1. 回调签名变更 | P0 | ❌ **未修复** | 仍为 `(MsgId_t, uint16_t, void*)` |
| 2. MsgScheduler_UpdateCommData() 被移除 | P0 | ❌ **未修复** | 仍未恢复 |
| 3. 按键映射与规格不匹配 | P0 | ❌ **未修复** | 仍然是规格书外的键码 |
| 4. 1ms ISR架构差异 | P1 | ⚠️ **维持** | 旗帜式ISR，可接受但有延迟风险 |
| 5. 10槽分时调度 | P1 | ⚠️ **维持** | 每个模块调用间隔10ms |
| 6. TIM0时钟配置 | P1 | ⚠️ **维持** | 未验证硬件是否支持 |
| 7. 蜂鸣器未实现 | P1 | ✅ **已解决** | buzzer.c/h 已完成 |
| 8. 按键映射不匹配 | P1 | ❌ **未修复** | 同P0问题3 |
| 9. CommData_t未定义 | P1 | ❌ **未修复** | 仍在msg_def.h的注释里写着但未定义 |
| 10. 缺失模块 | P2 | ⚠️ **部分解决** | buzzer/display已完成，proto/cook/ui/actuator仍缺失 |
| 11. data_ptr悬挂指针风险 | P2 | ⚠️ **维持** | 目前未使用data_ptr传栈指针 |
| 12. 无App_Init() | P2 | ❌ **未修复** | 初始化仍然散落在main中 |

---

## 三、新增模块审核

### 3.1 hal_buzzer.c/h — 蜂鸣器HAL层

**优点**：
- TIM1 PWM中断驱动，频率可调（通过 `HAL_BUZZ_PWM_SET` 宏）
- PE1(信号) + PE4(电源) 引脚映射与 hw_ref.md 完全一致
- 实现了尾音效果：鸣叫结束后PE1切高阻输入，利用电容放电产生RC衰减
- 中断开关由 `HAL_Buzzer_IntSync()` 动态控制，有声音时开中断，无声音时关中断，降低功耗
- ISR优先级高，频率驱动稳定

**问题**：

1. **🔴 全局变量暴露** — `hal_buzzer.h` 公开了12个 `extern` 全局变量：
   ```c
   extern uint8_t  g_buzz_my_active;
   extern uint16_t g_buzz_my_time_on;
   extern uint16_t g_buzz_my_time_off;
   // ... 共12个
   ```
   **影响**：架构要求"禁止全局变量暴露"，这些变量被 `drv_buzzer.c` 直接读写，绕过了消息调度器。虽然这是嵌入式项目中常见的HAL↔DRV协作模式，但与"无全局变量"的原则冲突。
   **建议**：要么将 `hal_buzzer.c` 和 `drv_buzzer.c` 合并为一个模块，要么定义setter/getter接口。

2. **🟡 TIM1中断向量处理** — `TIMER1_IRQHandler` 直接在 `hal_buzzer.c` 中实现。如果其他模块也需要TIM1中断，会有冲突。参考程序显示 TIM1 同时用于蜂鸣器和 I2C 驱动。

3. **🟡 `RCC_APB0Cmd(ENABLE)` 调用** — 第72行调用了 `RCC_APB0Cmd`，这个函数名不是标准STM32/SC32库函数，可能是自定义函数或笔误。参考程序用的是 `RCC_APB0PeriphClockCmd`。

### 3.2 drv_buzzer.c/h — 蜂鸣器驱动层

**优点**：
- 双模式支持：普通蜂鸣器（固定频率方波）+ 美声/和弦（音符序列）
- 美声模式有多达13种音效，包括上电音、按键音、小星星等
- 普通模式有16种预设音效参数
- 150ms消隐防抖（`g_buzz_on_delay`）
- 音符频率表来自参考程序 `Buzz_Drive.c` + `Buzz_Drive_MY.c`，已验证可工作

**问题**：

4. **🟡 美声模式与普通模式的互斥逻辑** — `Buzz_Mode()` 中调用 `Buzz_Mode_MY(0)` 关闭美声，但 `Buzz_Mode_MY()` 中只是清零美声状态，没有调用 `Buzz_Mode(0)` 关闭普通模式。如果先调普通再调美声，普通模式的 `g_buzz_count` 残留可能导致 `Buzz_Dispose()` 中的 `if (g_buzz_count == 0u) return;` 条件在美声模式下意外触发。

5. **🟡 普通模式鸣叫时序单位** — `Buzz_Dispose_Set()` 的参数单位为10ms（`hz_timer × 10ms`），而 `Drv_Buzzer_Timer_1ms()` 每1ms调用一次，每10ms才执行 `Buzz_Dispose()`。这意味着：
   - `DRV_BUZZ_KEY`: 鸣叫20 × 10ms = 200ms，间隔0ms → 实际蜂鸣器响200ms
   - 这与规格书要求的"按键音1次20ms"不一致
   
   **确认**：如果 `hz_timer = 20` 且每10ms递减一次，总时长 = 20 × 10ms = 200ms。但参考程序可能就是这个值。

### 3.3 hal_display.c/h — 显示HAL层

**优点**：
- 8 SEG + 11 COM的引脚映射与 hw_ref.md 完全一致
- 动态扫描实现正确：关上一COM → 清SEG消隐 → 输段码 → 选通新COM
- 11ms完成一轮完整扫描，每1ms切换一个COM
- IO驱动调用正确，COM低有效、SEG高有效

**问题**：

6. **🟡 无扫描中断** — 当前 `Drv_Display_Scan()` 由主循环每1ms调用。在极端情况下，如果某次1ms循环被阻塞（如UART3阻塞式TX），显示扫描会延迟，导致显示闪烁。
   **建议**：短期内问题不大，长期建议将扫描搬入TIM0中断或使用专用定时器。

7. **🟢 COM映射与语义** — `hal_display.c` 注释说 IO[0..3] 对应炉头0+1（左数码管），IO[4..7] 对应炉头2+3（右数码管）。这与规格书一致：
   - 左数码管左两位=炉头1，右两位=炉头2
   - 右数码管左两位=炉头3，右两位=炉头4
   **注意**：这里的炉头编号比规格书少1（C语言是0基 vs 规格书1基），后续UI代码要转换。

### 3.4 drv_display.c/h — 显示驱动层

**优点**：
- 双缓冲设计（`s_io_work` / `s_io_buff`），Update写入、Scan读取，通过dirty标志同步
- SMG库集成正确，`Disp_General_Init` 初始化2组显示
- 500ms闪烁同步
- 调试模式下直接显示按键码（hex上4位 + 按键类型字符串下4位）

**问题**：

8. **🟡 刷新频率** — `Drv_Display_Update()` 每10ms调用（槽位1），内部100ms才做一次内容刷新。从按键按下到显示更新，最大延迟 = 10ms(槽位等待) + 100ms(内部计时) + 11ms(扫描) ≈ 121ms，人眼看不出但偏慢。

9. **🟡 `Disp_Hex_H_L()` 函数调用** — 这个函数在 `SMG_Disp_General_Lib.h` 中是否存在？需要确认库函数的签名是否匹配。

### 3.5 main.c 更新

**优点**：
- 原来蜂鸣器上做Heartbeat的问题已修复（`#if 0` 禁用）
- 主循环顺序合理：先buzzer → 显示扫描 → 消息调度 → 槽位执行
- 新增了 `HAL_UART_Debug_Flush()` 调用

**问题**：

10. **🟡 单次主循环执行内容过多** — 每1ms tick执行：
    - Drv_Buzzer_Timer_1ms()：状态机处理
    - Drv_Display_Scan()：11 GPIO写入
    - MsgScheduler_Run1ms()：消息分发
    - ExecSlot_Run()：1个槽位
    - UART Debug Flush
    
    总执行时间可能在50-100μs，考虑到48MHz的SC32F1xxx，1ms tick内还有约900μs空闲，应该OK。

11. **🟢 初始化顺序** — `HAL_Buzzer_Init()` 在 `HAL_GPIO_Init()` 之前调用。理论上 `hal_buzzer.h` 的宏 `HAL_BUZZ_SIG_OUT` 等会自己做GPIO初始化，但和后面的 `HAL_GPIO_Init()` 对同一引脚做了两次初始化。不会出错，但不够干净。

---

## 四、整体符合度更新

| 要求 | 符合度 | 变化 |
|------|--------|------|
| 消息调度器 | ✅ 符合 | ← |
| 1ms分时调度 | ⚠️ 有偏差 | ← |
| UART3 DEBUG输出 | ✅ 符合 | ← |
| UART0 DMA+环形队列 | ✅ 符合 | ← |
| 模块完全解耦 | ❌ 有违 | ← 全局变量暴露加剧 |
| 禁止全局变量暴露 | ❌ 有违 | ← hal_buzzer.h公开12个extern |
| 蜂鸣器状态机 | ✅ **已实现** | 🆕 双模式音效 |
| 显示驱动 | ✅ **已实现** | 🆕 SMG库+双缓冲 |
| MODBUS协议模块 | ❌ 未实现 | ← |
| JSON驱动UI引擎 | ❌ 未实现 | ← |
| 烹饪逻辑模块 | ❌ 未实现 | ← |
| 执行器模块 | ❌ 未实现 | ← |

---

## 五、总结

**进展**：Claude 本轮新增了 buzzer（双模式音效）和 display（SMG动态扫描）两个模块，各约300-400行代码，功能实现完整，引脚映射与 hw_ref.md 一致。

**遗留P0问题（3个）**：
1. 回调签名未按规格改回 `(Msg_t *msg, void *dataPtr)`
2. `MsgScheduler_UpdateCommData()` 未恢复
3. 按键映射与规格书不一致（KEY_RIGHT_P_SET 等规格书没有的键码）

**新增问题（1个）**：
4. hal_buzzer.h 公开12个全局变量 extern，违反"无全局变量"原则

**待完成模块**：proto(MODBUS) / json_ui(JSON引擎) / cook(烹饪逻辑) / actuator(执行器)
