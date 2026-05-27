# Claude代码审核报告

> 审核日期：2026-05-20
> 审核范围：`Claude/` 目录下**所有应用层代码**（core/hal/drv/src/doc，不含lib固件库）
> 对比基准：4份规格文档 + AI提示词

---

## 一、总体概况

| 模块 | 状态 | 完成度 |
|------|------|--------|
| `core/msg_def.h` | ✅ 已完成 | 100% |
| `core/msg_scheduler.h/.c` | ✅ 已完成 | 95% |
| `hal/hal_timer.c/.h` | ✅ 已完成 | 90% |
| `hal/hal_uart.c/.h` (UART3 DEBUG) | ✅ 已完成 | 100% |
| `hal/hal_comm.c/.h` (UART0 DMA+环形队列) | ✅ 已完成 | 90% |
| `hal/hal_gpio.c/.h` | ✅ 已完成 | 80% |
| `hal/hal_key.c/.h` | ✅ 已完成 | 80% |
| `drv/drv_key.c/.h` | ✅ 已完成 | 70% |
| `src/main.c` | ⚠️ 部分完成 | 60% |
| `doc/hw_ref.md` | ✅ 已完成 | 100% |
| `buzzer.c` (蜂鸣器状态机) | ❌ 未实现 | 0% |
| `proto.c` (MODBUS协议层) | ❌ 未实现 | 0% |
| `json_ui.c` (JSON驱动UI引擎) | ❌ 未实现 | 0% |
| `cook.c` (烹饪逻辑) | ❌ 未实现 | 0% |
| `actuator.c` (执行器) | ❌ 未实现 | 0% |
| `test/test_module_a/b` | ✅ 测试对话Demo | 已完成(无需纳入正式) |

---

## 二、关键偏差与问题（按严重度排序）

### 🔴 P0 — 架构规则违背

#### 1. MsgHandler_t 回调签名变更

| 规格要求 | Claude实现 |
|---------|-----------|
| `typedef void (*MsgHandler_t)(Msg_t *msg, void *dataPtr)` | `typedef void (*MsgHandler_t)(MsgId_t id, uint16_t param, void *data_ptr)` |

**影响**：参数签名被拆开，而不是传递 Msg_t 结构体指针。
- Msg_t 的 data_ptr 本意是给调度器"透传"用的通用指针通道
- 拆开后回调里无法通过 msg->id 判断消息类型（虽然用id参数也能用，但破坏了接口一致性）
- 需要统一。建议回到 `(Msg_t *msg, void *dataPtr)` 以保持与规格一致

#### 2. 移除了 MsgScheduler_UpdateCommData()

**规格要求**（来自 `嵌入式灯板控制系统架构说明.md` 4.5节）：
> 通讯数据存储在调度器内部，通过`MsgScheduler_UpdateCommData()`更新后直接回调注册了 `MSG_COMM_DATA_UPDATE` 的模块

**Claude实现**：
```c
// msg_scheduler.h 中完全没有这个函数
// 第30行 MSG_COMM_DATA_UPDATE 虽保留了消息ID，但走的是普通 Msg_Post() 路径
// 数据指针混杂在 Msg_t.data_ptr 里
```

**影响**：
- 规格中的"集中存储+指针共享"机制被完全删除了
- 模块之前通过调度器内部数据区共享状态的设计被破坏
- CommData_t 需要移到 msg_def.h 或者每个模块各自维护

#### 3. Msg_Post 参数签名变更

| 规格要求 | Claude实现 |
|---------|-----------|
| `void Msg_Post(MsgId_t id, uint16_t param)` | `void Msg_Post(MsgId_t id, uint16_t param, void *data_ptr)` |

**影响**：新增了 data_ptr 参数，这是合理扩展，但与规格不兼容。
Msg_t 结构体也从 `{MsgId_t id; uint16_t param}` 变成 `{MsgId_t id; uint16_t param; void *data_ptr}`。
这是可接受的改进方向，但要注意 msg_scheduler.c 中 data_ptr 是**浅拷贝指针**——data_ptr 指向的对象必须在Msg_Post调用后保持有效直到消息被消费。如果调用栈上的局部变量被传递，会导致悬挂指针。

---

### 🟠 P1 — 架构设计与规格偏差

#### 4. 1ms 中断调度架构差异

| 规格要求 | Claude实现 |
|---------|-----------|
| ISR中直接调用 `BUZZER_Tick1ms()` → `MsgScheduler_Run1ms()` | ISR只设标志位 `s_1ms_pending`，主循环中检查后调用 |

**分析**：两种方式都能工作。Claude将 ISR 最小化的做法是业界常见的安全写法。但：
- 规格要求 ISR 中直接执行调度，延迟 ≤1ms
- Claude的做法：主循环中检查标志 → 调用，延迟取决于主循环时间
- **建议**：维持旗帜式做法可以，但需确保主循环检查频率 ≤1ms。当前代码通过 `if (HAL_Timer_1msElapsed())` 在 for(;;) 中检查，加上额外的10槽轮转，单次循环时间可能超过1ms。

#### 5. 10槽分时调度（规格未定义）

**Claude实现**：
```c
// main.c 的 ExecSlot_Run 将系统分为10个1ms槽位
// 每10ms完成一轮完整调度
```

**规格要求**：规格文档中没有定义10槽分时调度，所有模块的任务（UI_Task / COOK_Task）在主循环中无条件调用。

**分析**：10槽设计本身不违反架构，但当前 slot 3 用于 Drv_Key_Scan()——按键扫描每10ms才执行一次。如果触摸库要求更快的轮询频率（如参考程序的触摸库以125us为时基），10ms的间隔可能不够。

#### 6. TIM0 时钟配置与 hw_ref.md 矛盾

| hw_ref.md | Claude实现 |
|-----------|-----------|
| TIM0 Prescaler=1, Preload=65535-(24000000/4000) → **125us** | TIM0 Prescaler=64, Preload=749 → **1ms** |
| TIM0 中断优先级=0 (最高) | TIM0 中断优先级=1 (次高) |
| ISR 调用 `Timer_interrupt_Dispose()` | ISR 只设标志 |

**分析**：Claude 重新配置了 TIM0 为 1ms 时基，这是合理的——只需要一个1ms时基。但需要确认：
- Prescaler=64 是否合法（APB时钟48MHz，64分频后750kHz，Preload=749 → 1ms）
- SC32F1xxx 的 TIM_PRESCALER_64 枚举值是否正确
- **建议**：如果在真机上不跑，检查硬件定时器分频系数是否支持64分频

#### 7. 蜂鸣器状态机（buzzer.c）未实现

**规格要求**（来自 `项目结构程序示例.md` 第4节）：
```c
void BUZZER_Beep(uint8_t times, uint16_t intervalMs);  // 唯一直接回调
void BUZZER_Tick1ms(void);                               // 1ms中断中调用
```

**Claude**：只有 hal_gpio.c 提供了 `HAL_GPIO_Write(HAL_IO_BUZZER, level)` 和 `HAL_GPIO_Toggle(HAL_IO_BUZZER)` 的底层IO操作，**没有实现蜂鸣器状态机**。

更严重的是 `main.c` 中 heartbeat 占用了蜂鸣器：
```c
static void Heartbeat_OnTimer100ms(MsgId_t id, uint16_t param, void *data_ptr) {
    // 用蜂鸣器IO做心跳指示 —— 调试阶段可以，正式代码必须移除
    HAL_GPIO_Toggle(HAL_IO_BUZZER);
}
```

#### 8. 按键逻辑键码与规格书不匹配

**规格书要求**（四头电磁炉完整规格书）：
```
功能键: 开关、童锁、无区、暂停、定时
炉头选择键: 1、2、3、4
加减键: + / -（仅用于定时）
档位键: 0,1,2,3,4,5,6,7,8,9（10个独立键）

9档键: 松开检测（按下不触发）
0~8档键: 普通按下触发
```

**Claude实现**（drv_key.h）：
```c
typedef enum {
    KEY_NONE = 0,
    KEY_ONOFF,
    KEY_SUB, KEY_ADD,
    KEY_STOP,       // 规格书中叫"暂停"
    KEY_TIME_SET,
    KEY_BIND,       // 规格书中叫"无区"
    KEY_LOCK,       // 规格书中叫"童锁"
    KEY_RIGHT_P_SET, KEY_LEFT_P_SET,
    KEY_RIGHT_P_SET_UP, KEY_LEFT_P_SET_UP,   // 这些规格书中没有
    KEY_POWER_0 ~ KEY_POWER_9,   // 这里有了
} KeyCode_t;
```

问题：
1. `KEY_STOP` vs `KEY_PAUSE` —— 命名不一致，且规格书中暂停是全局暂停
2. `KEY_BIND` vs `KEY_ZONE` —— 命名不一致
3. `KEY_LOCK` vs `KEY_CHILD_LOCK` —— 命名不一致
4. `KEY_RIGHT_P_SET`/`KEY_LEFT_P_SET`/`KEY_RIGHT_P_SET_UP`/`KEY_LEFT_P_SET_UP` —— 这些在规格书中不存在。规格书的按键面板是10个独立档位键(0-9) + 加减键(用于定时)
5. 缺少 KEY_1 ~ KEY_4（炉头选择键）

**影响**：按键映射与规格书完全不一致，后续 json_ui.c 无法对接。

#### 9. CommData_t 未定义

**规格要求**（来自 `项目结构程序示例.md` msg_core.h）：
```c
typedef struct {
    uint16_t igbtTemp;
    uint16_t surfaceTemp;
    uint16_t coilCurrent;
    uint16_t systemState;
    uint8_t  faultCode;
    uint8_t  commStatus;
} CommData_t;
```

**Claude的 msg_def.h**：完全没有定义 CommData_t，也没有定义 PowerCtrl_t / ProtectFault_t / ModbusData_t（这些被 msg_def.h 中的注释引用但不存在）。

---

### 🟡 P2 — 缺失模块 & 次要问题

#### 10. 以下模块完全未实现

| 模块 | 预期文件 | 依赖规格 |
|------|---------|---------|
| `proto.c` (MODBUS协议) | `inc/proto.h` + `src/proto.c` | 架构说明 + 提示词 |
| `json_ui.c` (JSON引擎) | `inc/json_ui.h` + `src/json_ui.c` | JSON实施文档 + 规格书 |
| `cook.c` (烹饪逻辑) | `inc/cook.h` + `src/cook.c` | 项目结构程序示例 第7节 |
| `actuator.c` (执行器) | `inc/actuator.h` + `src/actuator.c` | 项目结构程序示例 第8节 |

这些模块是项目的核心业务逻辑，Claude 在第一次迭代中只完成了基础设施层（core + hal + drv + test demo）。

#### 11. Msg_Post data_ptr 悬挂指针风险

```c
static void Slot_CommPoll(void) {
    uint8_t  rx_buf[COMM_RX_BUF_SIZE];  // 栈上变量
    HAL_Comm_Read(rx_buf, frame_len);
    // 如果这里 Msg_Post(..., rx_buf) 传递的是栈指针，
    // 1ms后回调执行时 rx_buf 已失效！
}
```

当前代码中 `Slot_CommPoll` 还没有调用 Msg_Post，但如果后续proto模块将栈上缓冲区的指针通过 data_ptr 传递，就会造成悬挂指针。

#### 12. 没有模块统一初始化入口

虽然 main.c 中有 `MsgScheduler_Register` 调用，但缺少 `App_Init()` 函数（规格要求有）。当前初始化散落在 main 中：
```c
HAL_UART_Debug_Init();    // 硬件的
HAL_GPIO_Init();
HAL_Comm_Init();
MsgScheduler_Init();      // 软件的
MsgScheduler_Register(...); // 回调注册
TestA_Init();
TestB_Init();
Drv_Key_Init();
HAL_Timer_Init();
```

---

## 三、与提示词要求的符合度

| 要求 | 符合度 | 说明 |
|------|--------|------|
| 消息调度器 | ✅ 符合 | 实现了环形队列+回调注册，接口稍有偏差 |
| 1ms分时调度 | ⚠️ 有偏差 | 10槽分时+旗帜式ISR vs 规格要求ISR直调 |
| UART3 DEBUG | ✅ 符合 | 57600-10B，pin44/45，阻塞式TX |
| UART0 DMA+环形队列 | ✅ 符合 | DMA0 RX 不定长 + DMA1 TX，帧间隔检测 |
| 模块完全解耦 | ❌ 有违 | `hal_timer.h` 无问题；但回调签名被改变 |
| 禁止全局变量暴露 | ✅ 符合 | scheduler使用 static 变量 |
| MODBUS协议模块 | ❌ 未实现 | proto.c 缺失 |
| JSON驱动UI引擎 | ❌ 未实现 | json_ui.c 缺失 |
| 烹饪逻辑模块 | ❌ 未实现 | cook.c 缺失 |
| 执行器模块 | ❌ 未实现 | actuator.c 缺失 |
| 蜂鸣器状态机 | ❌ 未实现 | 仅有GPIO底层操作 |

---

## 四、修改建议（按优先级）

### 建议立即修复
1. 回调签名改回 `(Msg_t *msg, void *dataPtr)` —— 保持与架构文档一致
2. 恢复 `MsgScheduler_UpdateCommData()` 函数 —— 规格架构的核心机制
3. 在 msg_def.h 中定义 `CommData_t` 和业务相关数据结构
4. 删除或注释掉 heartbeat 占用的蜂鸣器IO
5. 按键映射表按规格书重做（KEY_POWER/KEY_CHILD_LOCK/KEY_ZONE/KEY_PAUSE/KEY_TIMER/KEY_1~KEY_4/KEY_PLUS/KEY_MINUS/KEY_0~KEY_9_PWR）

### 建议近期完成
6. 实现 `buzzer.c`（BUZZER_Beep + BUZZER_Tick1ms 状态机）
7. 实现 `proto.c`（MODBUS 协议打包/解包/CRC16）
8. 实现 `json_ui.c`（节点+事件状态机引擎 + 对应规格书的按键处理函数）
9. 实现 `cook.c`（炉头烹饪逻辑 + 定时倒计时）
10. 实现 `actuator.c`（风机/加热执行器）
11. data_ptr 传栈指针的场景用 static 缓冲或 memcpy

### 注意事项
12. 确认 TIM0 prescaler=64 在 SC32F1xxx 上是否支持
13. 确认触摸库扫描频率是否适配10ms槽位轮询
