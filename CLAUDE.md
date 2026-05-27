# CLAUDE.md — 四头电磁炉低耦合控制程序

## 基础约定

- **规划文件 (.md) 存放于项目根目录** `D:\OBSIDIAN\MOVING IH\低耦合程序架构\`，与 CLAUDE.md 同级，文件名用英文 slug 格式（如 `sparkling-whistling-porcupine.md`）。
- **方法论/记忆文件 (.md) 必须双写**: 自动记忆目录 (`C:\Users\moving\.claude\projects\...`) + 项目目录 (`four_head/memory/`)。两份内容一致。这是为了新 AI 会话可独立从项目目录读取全部方法论，不依赖外部记忆系统。
- **里程碑节点必须 Git 提交**: 架构变更、方法论更新、重大功能完成等里程碑节点必须做一次 git commit。AI 应在里程碑完成后主动提议或执行提交。

## 核心架构原则

**业务模块之间零交叉include，消息调度器是唯一桥梁。**
- HAL层不include任何业务头文件
- DRV层只include msg_scheduler.h + HAL接口
- ISR只设时基标志，不在中断中做业务处理

## 硬件配置（必须与参考程序一致）

> 参考程序路径: `参考程序/ai专用工程2026.4.27(06B显示板)/`
> 任何涉及芯片引脚、端口、外设的配置，必须以参考程序为准。

### 调试串口: UART3
- 引脚: 44/45（B组）, `UART_PinRemapConfig(UART3, UART_PinRemap_A)`
- 总线: APB2（`RCC_APB2Periph_UART3`），时钟 48MHz
- 波特率: 57600（不是115200）
- 中断号: `UART1_3_5_7816_IRQn`（共享中断）
- 阻塞式TX，仅调试打印用

### MODBUS通讯串口: UART0 (hal_comm)
- 引脚: 48/47（默认映射，不需要 PinRemap）
- 波特率: 57600
- 时钟: APB0 = HIRC 48MHz
- 中断号: `UART0_2_4_IRQn` + `DMA1_IRQn`
- RX: DMA0 单次模式, 256字节缓冲, 轮询帧间隔(2次×10ms=20ms)判定帧结束
- TX: DMA1 单次发送, TC中断通知完成
- **SPI1_TWI1 不能做UART** —— 它只支持 SPI/TWI 模式
- 实现: `hal/hal_comm.c`

### 时基定时器: TIM0
- 时钟: APB0 = 48MHz（HCLK Div1）
- 预分频: `TIM_PRESCALER_1`
- 周期: 125us（Preload=5999, 48MHz/6000=8kHz）
- ISR 每 125us: 调用 `HAL_Key_Poll()` 驱动触摸状态机
- 每 8 次 ISR (1ms): 置 1ms 标志 + tick++

### 蜂鸣器: TIM1 + PE1
- `P_BUZZ(value)`: GPIOE Pin_1
- `P_BUZZ_Power(value)`: GPIOE Pin_4
- TIM1 产生 PWM 频率，TIM1 ISR 中调用 `Buzz_AND_MY_Buzz_Driver()`

### 显示: 8 SEG + 11 COM（IO直推数码管）
| SEG | 引脚 | 端口 | COM | 引脚 | 端口 |
|-----|------|------|-----|------|------|
| A | PC8 | GPIOC | 1 | PE11 | GPIOE |
| B | PB14 | GPIOB | 2 | PE12 | GPIOE |
| C | PE9 | GPIOE | 3 | PA10 | GPIOA |
| D | PC4 | GPIOC | 4 | PE10 | GPIOE |
| E | PD13 | GPIOD | 5 | PA6 | GPIOA |
| F | PB15 | GPIOB | 6 | PA1 | GPIOA |
| G | PE8 | GPIOE | 7 | PA0 | GPIOA |
| H | PC1 | GPIOC | 8 | PA7 | GPIOA |
|   |      |       | 9 | PC0 | GPIOC |
|   |      |       | 10 | PA5 | GPIOA |
|   |      |       | 11 | PA4 | GPIOA |

### 触摸按键: 21通道（参考 `S_TouchKeyCFG.h`）
- 触摸通道: 7-24, 28-30
- 按键映射: 参考 `参考程序/User_Main/APP_Driver/Key_Driver.c` 的 switch 表
- 组合键（相邻功率焊盘同时触摸→中间值）已在 drv_key.c 实现
- 调试IO: PB6、PB11

### 系统时钟
- HIRC 48MHz → SYSCLK → HCLK(Div1) → APB0/APB1/APB2(Div1)
- 外部晶振: 不使用（HXT=DISABLE, LXT=DISABLE）
- 复位电压: LVR 2.9V
- JTAG: 关闭（IO模式）

## 调度模式（10槽1ms轮转）

```
每1ms: HAL_Timer_1msElapsed() →
  起始段: MsgScheduler_Run1ms()  消费1条消息
  执行段: ExecSlot_Run()         轮转1个槽
  等待段: 通讯轮询（预留）

10槽 × 1ms = 10ms完整周期
模块每10ms被调用1次，自计数计时（不依赖hal_timer）
```

## 已用库（来自参考程序 Function_Application）

- `SMG_Disp_General_Lib_1.5.lib` — 段码转换+显示特效
- `Transition_Func_Lib_C_1.3.lib` — NTC温度转换、PID、斜率计算（仅用纯数学函数，不用时间控制模块）
- `SC_M0+_HighSensitive_lib_T1_V1.0.lib` — 触摸算法库

## 编译环境

- MCU: 赛元 SC32L14T（Cortex-M0+ 48MHz, 256K Flash, 16K SRAM）
- 编译器: Keil MDK, ARMCC V5.06
- C标准: C89（无 `_Static_assert`，用 `CT_ASSERT` 宏替代）
- `#pragma pack(4)` 用于 Msg_t 结构体

## 修改任何外设/引脚前必须

1. 先查参考程序对应文件，确认引脚/配置
2. 如果参考程序是注释掉的（如加热/风扇IO），说明实物未接，保持注释
3. 不要自己发明引脚号

## 模块时基调用

1. 任务调度槽的模块是每10ms调用一次，内部延时无同步要求的可内部计数，不需要引入外部计时信息，减少耦合
## 数码管COM扫描

1. 数码管COM每1ms扫描一个COM， 更新数据前要将此COM先关掉，COM无效，再更新数据，再切换到新的COM，才不会偷亮。

内部项目管理

1、低耦合程序架构/m4_ekf_observer  ：半桥/全桥加热控制程序
2、低耦合程序架构/four_head/Project： 解耦灯板项目文件 
3、低耦合程序架构/four_head/sim/test_hmi：JS+JSON人机交互逻辑验证程序
4、m4_ekf_observer/tools/ekf_tuner/m4_modbus_tool.py  半桥/全桥加热控制程序modbus 测试程序