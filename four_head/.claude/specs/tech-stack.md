# 技术栈

## 编程语言
- **语言**: C
- **标准**: C89（ARMCC V5.06 编译器兼容，无 `_Static_assert`，用 `CT_ASSERT` 宏替代）

## 编译器/IDE
- **编译器**: Keil MDK ARMCC V5.06 update 6
- **IDE**: Keil uVision 5
- **设备包**: SinOne.SC32F1xxx_DFP.1.1.5
- **调试器**: JLink

## 硬件平台
- **MCU**: 赛元 SC32L14TR8（ARM Cortex-M0+）
- **主频**: 48MHz HIRC（内部高速 RC，无外部晶振）
- **Flash**: 256K
- **SRAM**: 16K
- **堆栈**: Heap 512B / Stack 1280B
- **外设**:
  - TIM0: 125us 系统时基（8kHz）
  - TIM1: 蜂鸣器 PWM（2kHz 基频）
  - UART0: MODBUS 通讯（DMA0 RX + DMA1 TX, 57600）
  - UART3: 调试串口（阻塞 TX, 57600）
  - GPIO: 8 SEG + 11 COM 直驱数码管
  - TouchKey: 21 通道（ch7-24, ch28-30）
  - LVR: 2.9V
  - JTAG: DISABLE（IO 模式）

## 依赖库
- **SMG_Disp_General_Lib_1.5.lib** — 段码转换 + 显示特效
- **Transition_Func_Lib_C_1.3.lib** — NTC 温度转换、PID、斜率计算（仅用纯数学函数）
- **SC_M0+_HighSensitive_lib_T1_V1.0.lib** — 触摸算法库
- **SC_M0+_HighSensitive_lib_Basic_V1.0.lib** — 触摸基础库
- **SC32F1XXX_Lib** — 赛元固件库（ADC, BTM, CRC, DMA, GPIO, IAP, OPTION, RCC, TIM, UART, WDT）
- **CMSIS (Cortex-M0+)** — ARM CMSIS 头文件

## 版本管理
- Git

---

*最后更新：2026-05-21，技术负责人*
