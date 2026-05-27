# 硬件配置参考（提取自参考程序）

> 源: `参考程序/ai专用工程2026.4.27(06B显示板)/`
> 修改任何外设前先对照此文件。

---

## 1. 系统时钟

```
HIRC 48MHz → SYSCLK → HCLK(Div1) → APB0/APB1/APB2(Div1)
HXT = DISABLE (无外部高速晶振)
LXT = DISABLE (无外部32.768K)
LIRC = ENABLE
RCC_Unlock(0xFF)
RCC_SYSCLKConfig(RCC_SYSCLKSource_HIRC)
```

源文件: `MCU_User/SC_Init.c` → `SC_RCC_Init()`

---

## 2. 调试串口 UART3

| 参数 | 值 |
|------|-----|
| 引脚 | 44/45 (B组) |
| 重映射 | `UART_PinRemapConfig(UART3, UART_PinRemap_A)` |
| 波特率 | 57600 |
| 时钟 | APB 48MHz |
| 数据位 | 10B mode |
| 中断号 | `UART1_3_5_7816_IRQn` |
| 发送函数 | `UART_SendData(UART3, byte)` |

源文件: `MCU_Apps/UART_3_Communication.c` → `UART_3_Communication_Iniy()`

```c
// 初始化样板
UART_InitStruct.UART_BaudRate = 57600;
UART_InitStruct.UART_ClockFrequency = 48000000;
UART_InitStruct.UART_Mode = UART_Mode_10B;
UART_PinRemapConfig(UART3, UART_PinRemap_A);
UART_Init(UART3, &UART_InitStruct);
UART_ITConfig(UART3, UART_IT_EN | UART_IT_RX, ENABLE);
UART_ITConfig(UART3, UART_IT_EN | UART_IT_TX, ENABLE);
NVIC_EnableIRQ(UART1_3_5_7816_IRQn);
UART_TXCmd(UART3, ENABLE);
UART_RXCmd(UART3, ENABLE);
```

---

## 3. 定时器

### TIM0 — 系统时基
| 参数 | 值 |
|------|-----|
| 时钟 | APB0 = 48MHz |
| 预分频 | `TIM_PRESCALER_1` |
| Preload | `65535 - (24000000 / 4000)` → 125us |
| 中断 | `TIMER0_IRQn`，优先级0(最高) |
| ISR | `TIMER0_IRQHandler` → `Timer_interrupt_Dispose()` |

### TIM1 — 蜂鸣器 PWM
| 参数 | 值 |
|------|-----|
| 时钟 | APB0 = 48MHz |
| 预分频 | `TIM_PRESCALER_1` |
| Preload | `65535 - (24000000 / 2000)` → 2kHz基频 |
| 重设 | `SC_TIM1_Buzz_SET(unsigned short PWM)` 动态改频率 |
| 中断 | `TIMER1_IRQn` |
| ISR | `TIMER1_IRQHandler` → `Buzz_AND_MY_Buzz_Driver()` |

### TIM2 — I2C驱动定时器
| 参数 | 值 |
|------|-----|
| 预分频 | `TIM_PRESCALER_1` |
| Preload | `65535 - (24000000 / 2000)` → 2kHz |
| ISR | `Timer_IIC_Service_Lib(0)` |

### BTM — 1秒定时
| 参数 | 值 |
|------|-----|
| 频率选择 | `BTM_FreqSelect_1S` |
| ISR | `BTM_IRQHandler` → `System_Timer_BTM()` |

---

## 4. 蜂鸣器引脚

```c
// Buzz_Drive.h — 参考程序正在使用的版本
P_BUZZ(value)         GPIO_WriteBit(GPIOE, GPIO_Pin_1, ...)   // PE1: 蜂鸣器IO
P_BUZZ_Power(value)   GPIO_WriteBit(GPIOE, GPIO_Pin_4, ...)   // PE4: 蜂鸣器功率开关

// 初始化宏
P_BUZZ_OUT       // PE1→推挽输出
P_BUZZ_Power_OUT // PE4→推挽输出

// 关闭
DF_OFF_Buz_IO    // P_BUZZ(0) + P_BUZZ_Power(0)

// 中断控制蜂鸣器
DF_Interrupt_ON  → SC_TIM1_Buzz_ONOFF(1)  // 开TIM1中断
DF_Interrupt_OFF → SC_TIM1_Buzz_ONOFF(0)  // 关TIM1中断
```

源文件: `APP_Driver/Buzz_Drive.h`

---

## 5. 显示引脚（8 SEG + 11 COM）

```c
// ===== SEG 段选 (8线) =====
P_SEG_A(value)   GPIO_WriteBit(GPIOC, GPIO_Pin_8, ...)   // PC8
P_SEG_B(value)   GPIO_WriteBit(GPIOB, GPIO_Pin_14, ...)  // PB14
P_SEG_C(value)   GPIO_WriteBit(GPIOE, GPIO_Pin_9, ...)   // PE9
P_SEG_D(value)   GPIO_WriteBit(GPIOC, GPIO_Pin_4, ...)   // PC4
P_SEG_E(value)   GPIO_WriteBit(GPIOD, GPIO_Pin_13, ...)  // PD13
P_SEG_F(value)   GPIO_WriteBit(GPIOB, GPIO_Pin_15, ...)  // PB15
P_SEG_G(value)   GPIO_WriteBit(GPIOE, GPIO_Pin_8, ...)   // PE8
P_SEG_H(value)   GPIO_WriteBit(GPIOC, GPIO_Pin_1, ...)   // PC1

// ===== COM 位选 (11线) =====
P_LED_COM1(value)   GPIO_WriteBit(GPIOE, GPIO_Pin_11, ...)  // PE11
P_LED_COM2(value)   GPIO_WriteBit(GPIOE, GPIO_Pin_12, ...)  // PE12
P_LED_COM3(value)   GPIO_WriteBit(GPIOA, GPIO_Pin_10, ...)  // PA10
P_LED_COM4(value)   GPIO_WriteBit(GPIOE, GPIO_Pin_10, ...)  // PE10
P_LED_COM5(value)   GPIO_WriteBit(GPIOA, GPIO_Pin_6, ...)   // PA6
P_LED_COM6(value)   GPIO_WriteBit(GPIOA, GPIO_Pin_1, ...)   // PA1
P_LED_COM7(value)   GPIO_WriteBit(GPIOA, GPIO_Pin_0, ...)   // PA0
P_LED_COM8(value)   GPIO_WriteBit(GPIOA, GPIO_Pin_7, ...)   // PA7
P_LED_COM9(value)   GPIO_WriteBit(GPIOC, GPIO_Pin_0, ...)   // PC0
P_LED_COM10(value)  GPIO_WriteBit(GPIOA, GPIO_Pin_5, ...)   // PA5
P_LED_COM11(value)  GPIO_WriteBit(GPIOA, GPIO_Pin_4, ...)   // PA4

// 显示缓存长度: DF_LED_Count_Long = 11
// 扫描方式: 动态扫描，每个COM依次拉低，SEG输出段码
// 段码表: SMG_Disp_Code[] (来自 SMG_Disp_General_Lib.h)
//   SMG_Disp_0 = 0x3F, SMG_Disp_1 = 0x06, ... SMG_Disp_OFF = 0x00
```

源文件: `APP_Driver/LED_Drive.c`, `APP_Driver/LED_Drive.h`

---

## 6. 触摸按键

### 通道配置 (S_TouchKeyCFG.h)
```
通道总数: 21
通道位掩码: 0x71FFFF80 (ch7-ch24, ch28-ch30)
TKCFG[17] = {1,1,0,5,10,3000,200,100,2,1,1,4,0,1,65535,65535,20}
```

### PCB丝印 → MCU通道
```
TK1→Ch28  TK2→Ch29  TK3→Ch24  TK4→Ch30  TK5→Ch23
TK6→Ch22  TK7→Ch10  TK8→Ch11  TK9→Ch13  TK10→Ch7
TK11→Ch8  TK12→Ch21 TK13→Ch20 TK14→Ch19 TK15→Ch18
TK16→Ch17 TK17→Ch16 TK18→Ch15 TK19→Ch14 TK20→Ch12
TK21→Ch9
```

### 逻辑键码映射 → 见 drv_key.c s_key_map[]

---

## 7. 调试/测试引脚

```c
TP1: PB6   // _Dbug_Test_GPIO_()
TP2: PB11  // _Dbug_Test_GPIO_2()
```

源文件: `MCU_Apps/GPIO_Fun.c`

---

## 8. 已注释的外设IO（实物未接）

```c
// 风扇:      PE10, PE11
// 继电器:    PC0, PB5
// 电磁阀:    PE12, PA0
// 水泵:      PA1
// 霍尔/水位: PB9, PB6, PB10
// LED直推:   PA3, PB14, PC0, PC2, PC12, PA2, PB11, PB15, PC1, PC11, PA5, PD11
```

---

## 9. 中断向量汇总

| 中断 | 函数 | 用途 |
|------|------|------|
| TIMER0 | `Timer_interrupt_Dispose()` | 125us系统时基 |
| TIMER1 | `Buzz_AND_MY_Buzz_Driver()` | 蜂鸣器频率PWM |
| TIMER2 | `Timer_IIC_Service_Lib(0)` | I2C定时器 |
| BTM | `System_Timer_BTM()` | 1秒定时 |
| UART1_3_5_7816 | `UART_Communication_UART3Handler()` + UART5 | 串口3/5 |
| UART0_2_4 | `UART_Communication_UART0Handler()` | 串口0(通讯) |
| SysTick | (空) | 16kHz系统节拍 |
| ADC | (空) | AD转换完成 |

源文件: `MCU_User/SC_it.c`

---

## 10. 复位选项

```c
LVR: 2.9V
JTAG: DISABLE (IO模式)
RCC_AHBPeriphClockCmd(RCC_AHBPeriph_IFB, ENABLE)
OPTION_LVRConfig(OPTION_LVR_2_9V)
OPTION_JTAGCmd(DISABLE)
```

---
## 11. MODBUS通讯 UART0（本项目新增）

| 参数 | 值 |
|------|-----|
| 外设 | UART0 |
| 引脚 | 48/47 (默认映射，非Remap) |
| 波特率 | 57600 |
| 时钟 | APB0 48MHz |
| 数据位 | 10B mode |
| 中断号 | `UART0_2_4_IRQn`（共享中断）|
| RX | DMA0, 单次模式256B, 轮询帧间隔(20ms)检测帧结束 |
| TX | DMA1, 单次发送, TC中断通知完成 |

> SPI1_TWI1 不能做 UART —— 它只支持 SPI/TWI（I2C）模式。
> 参考程序注释 `通讯串口(Pin48/47，串口0)` 说明 pin48/47 用作 UART0。
> 本项目: `hal/hal_comm.c`
