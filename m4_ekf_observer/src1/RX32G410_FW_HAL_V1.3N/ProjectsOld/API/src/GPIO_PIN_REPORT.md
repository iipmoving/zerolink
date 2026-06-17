# GPIO脚位功能报告

## 文件信息

- **源文件**: `API_gpio.c`
- **生成日期**: 2026-06-05
- **适用平台**: RX32G410
- **应用场景**: 四通道电磁炉控制

---

## 1. 调试口 (DEBUG)

| 名称 | 端口 | 引脚 | 模式 | 功能描述 |
|------|------|------|------|----------|
| DEBUG_A | GPIOB | PIN_3 | 推挽输出 | 调试输出A |
| DEBUG_B | GPIOB | PIN_5 | 推挽输出 | 调试输出B |
| DEBUG_C | GPIOB | PIN_4 | 推挽输出 | 调试输出C |
| DEBUG_D | GPIOB | PIN_4 | 推挽输出 | 调试输出D |

---

## 2. 过零检测

| 名称 | 端口 | 引脚 | 模式 | 复用功能 |
|------|------|------|------|----------|
| ZERO | GPIOD | PIN_2 | 复用推挽 | AF6_TIM8 |

---

## 3. 电压检测

| 名称 | 端口 | 引脚 | 模式 | 功能描述 |
|------|------|------|------|----------|
| VOLTAGE | GPIOC | PIN_3 | 模拟输入 | 电压AD采样 (AN1_9) |

---

## 4. PWM输出 (PPG互补输出)

| 名称 | 端口 | 引脚 | 模式 | 复用功能 |
|------|------|------|------|----------|
| PWM1H | GPIOC | PIN_6 | 复用推挽 | AF13 (HRTIM_CHB1) |
| PWM1L | GPIOC | PIN_7 | 复用推挽 | AF13 (HRTIM_CHB2) |
| PWM2H | GPIOC | PIN_8 | 复用推挽 | AF13 (HRTIM_CHE1) |
| PWM2L | GPIOC | PIN_9 | 复用推挽 | AF13 (HRTIM_CHE2) |
| PWM3H | GPIOA | PIN_8 | 复用推挽 | AF13 (HRTIM_CHA1) |
| PWM3L | GPIOA | PIN_9 | 复用推挽 | AF13 (HRTIM_CHA2) |
| PWM4H | GPIOC | PIN_11 | 复用推挽 | AF12 (HRTIM_CHD1) |
| PWM4L | GPIOC | PIN_12 | 复用推挽 | AF12 (HRTIM_CHD2) |

---

## 5. IGBT保护 (过流保护)

| 名称 | 端口 | 引脚 | 模式 | 复用功能 |
|------|------|------|------|----------|
| BK12 | GPIOE | PIN_6 | 复用推挽 | AF13 (HRTIM_FLT3) |
| BK34 | GPIOC | PIN_10 | 复用推挽 | AF13 (HRTIM_FLT6) |

---

## 6. 谐振电流CMP输入

| 名称 | 端口 | 引脚 | 模式 | 功能描述 |
|------|------|------|------|----------|
| T1A | GPIOA | PIN_0 | 模拟输入 | CMP1输入 |
| T2A | GPIOA | PIN_1 | 模拟输入 | CMP2输入 |
| T3A | GPIOB | PIN_13 | 模拟输入 | CMP3输入 |
| T4A | GPIOB | PIN_14 | 模拟输入 | CMP4输入 |

---

## 7. 检锅相关

| 名称     | 端口    | 引脚     | 模式   | 功能描述          |
| ------ | ----- | ------ | ---- | ------------- |
| PAN    | GPIOA | PIN_4  | 模拟输入 | 检锅检测 (AN1_10) |
| PANSW1 | GPIOB | PIN_15 | 推挽输出 | 检锅切换1         |
| PANSW2 | GPIOF | PIN_1  | 推挽输出 | 检锅切换2         |
| PANSW3 | GPIOF | PIN_0  | 推挽输出 | 检锅切换3         |
| PANSW4 | GPIOA | PIN_11 | 推挽输出 | 检锅切换4         |

---

## 8. 炉面温度检测 (BOTTOM)

| 名称 | 端口 | 引脚 | 模式 | 功能描述 |
|------|------|------|------|----------|
| BOTTOM1 | GPIOC | PIN_13 | 模拟输入 | 炉面热敏电阻1 (AN1_13) |
| BOTTOM2 | GPIOC | PIN_14 | 模拟输入 | 炉面热敏电阻2 (AN1_1) |
| BOTTOM3 | GPIOC | PIN_1 | 模拟输入 | 炉面热敏电阻3 (AN1_7) |
| BOTTOM4 | GPIOC | PIN_2 | 模拟输入 | 炉面热敏电阻4 (AN1_8) |

---

## 9. IGBT温度检测

| 名称    | 端口    | 引脚    | 模式   | 功能描述               |
| ----- | ----- | ----- | ---- | ------------------ |
| IGBT1 | GPIOC | PIN_4 | 模拟输入 | IGBT散热器温度1 (AN2_1) |
| IGBT2 | GPIOC | PIN_5 | 模拟输入 | IGBT散热器温度2 (AN2_2) |
| IGBT3 | GPIOB | PIN_0 | 模拟输入 | IGBT散热器温度3 (AN2_8) |
| IGBT4 | GPIOA | PIN_7 | 模拟输入 | IGBT散热器温度4 (AN2_7) |

---

## 10. 风机控制

| 名称 | 端口 | 引脚 | 模式 | 复用功能 |
|------|------|------|------|----------|
| FAN | GPIOA | PIN_10 | 复用推挽 | AF10_TIM2 (PWM控制) |
| FAN_AD | GPIOC | PIN_0 | 模拟输入 | 风机电流检测 (OP1P2) |

---

## 11. I2C通信接口

| 名称 | 端口 | 引脚 | 模式 | 复用功能 |
|------|------|------|------|----------|
| I2C1_SCL | GPIOB | PIN_8 | 复用开漏 | AF4_I2C1 |
| I2C1_SDA | GPIOB | PIN_9 | 复用开漏 | AF4_I2C1 |
| I2C2_SCL | GPIOB | PIN_9 | 复用开漏 | AF5_I2C2 |
| I2C2_SDA | GPIOB | PIN_8 | 复用开漏 | AF5_I2C2 |

> 注：通过 `I2C1_PORT` 宏选择使用I2C1或I2C2

---

## 12. UART通信接口

| 名称 | 端口 | 引脚 | 模式 | 复用功能 |
|------|------|------|------|----------|
| UART2_TX | GPIOA | PIN_14 | 复用推挽 | AF7_UART2 |
| UART2_RX | GPIOA | PIN_15 | 复用推挽 | AF7_UART2 |
| UART3_TX | GPIOB | PIN_9 | 复用开漏 | AF7_UART3 |
| UART3_RX | GPIOB | PIN_8 | 复用开漏 | AF7_UART3 |

> 注：通过 `COMM_UART` 宏选择使用UART2或UART3

---

## 13. 谐振电流检测 (OP正极输入)

| 名称 | 端口 | 引脚 | 模式 | 功能描述 |
|------|------|------|------|----------|
| CUR1 | GPIOA | PIN_2 | 模拟输入 | OP1P1 (谐振电流1) |
| CUR2 | GPIOA | PIN_5 | 模拟输入 | OP2P1 (谐振电流2) |
| CUR3 | GPIOB | PIN_2 | 模拟输入 | OP3P1 (谐振电流3) |
| CUR4 | GPIOB | PIN_10 | 模拟输入 | OP4P1 (谐振电流4) |

---

## 14. 运放接口 (OP)

| 名称 | 端口 | 引脚 | 模式 | 功能描述 |
|------|------|------|------|----------|
| OP1N1 | GPIOA | PIN_3 | 模拟输入 | OP1负极 |
| OP1O | GPIOA | PIN_4 | 模拟输入 | OP1输出 |
| OP2N1 | GPIOA | PIN_6 | 模拟输入 | OP2负极 |
| OP2O | GPIOA | PIN_7 | 模拟输入 | OP2输出 |
| OP3N1 | GPIOB | PIN_1 | 模拟输入 | OP3负极 |
| OP3O | GPIOB | PIN_0 | 模拟输入 | OP3输出 |
| OP4N1 | GPIOB | PIN_11 | 模拟输入 | OP4负极 |
| OP4O | GPIOB | PIN_12 | 模拟输入 | OP4输出 |

---

## 15. 其他功能

| 名称 | 端口 | 引脚 | 模式 | 复用功能 |
|------|------|------|------|----------|
| HRTIM_SYN | GPIOB | PIN_6 | 复用推挽 | AF12 (HRTIM同步) |
| HRTIM_TEST1 | GPIOA | PIN_15 | 复用推挽 | AF12 (测试输出) |
| TP1 | GPIOA | PIN_15 | 推挽输出 | 测试点1 |
| TP2 | GPIOA | PIN_12 | 推挽输出 | 测试点2 |
| SCR | GPIOA | PIN_6 | 复用推挽 | AF1_TIM16 (可控硅触发) |
| Ceil1 | GPIOC | PIN_15 | 模拟输入 | 顶部温度1 (AN1_2) |
| Ceil2 | GPIOA | PIN_4 | 模拟输入 | 顶部温度2 (AN1_10) |

---

## GPIO分组索引表 (GPIO_PIN[])

| 索引 | 名称 | 索引 | 名称 | 索引 | 名称 |
|------|------|------|------|------|------|
| 0 | DEBUG_A | 17 | T2A | 34 | SDA |
| 1 | DEBUG_B | 18 | T3A | 35 | CUR1 |
| 2 | DEBUG_C | 19 | T4A | 36 | CUR2 |
| 3 | DEBUG_D | 21 | PANSW1 | 37 | CUR3 |
| 4 | ZERO | 22 | PANSW2 | 38 | CUR4 |
| 5 | VOLTAGE | 23 | PANSW3 | 47 | HRTIM_SYN |
| 6 | PWM1L | 24 | PANSW4 | 48 | TX_SEL |
| 7 | PWM1H | 25 | BOTTOM1 | 49 | RX_SEL |
| 8 | PWM2L | 26 | BOTTOM2 | 50 | HRTIM_TEST1 |
| 9 | PWM2H | 27 | BOTTOM3 | 51 | SCR |
| 10 | PWM3L | 28 | BOTTOM4 | 52 | Ceil1 |
| 11 | PWM3H | 29 | IGBT1 | 53 | Ceil2 |
| 12 | PWM4L | 30 | IGBT2 | | |
| 13 | PWM4H | 31 | IGBT3 | | |
| 14 | BK12 | 32 | IGBT4 | | |
| 15 | BK34 | 33 | FAN | | |
| 16 | T1A | 34 | FAN_AD | | |

---

## 端口时钟使能

```c
__HAL_RCC_GPIOA_CLK_ENABLE();
__HAL_RCC_GPIOB_CLK_ENABLE();
__HAL_RCC_GPIOC_CLK_ENABLE();
__HAL_RCC_GPIOD_CLK_ENABLE();
__HAL_RCC_GPIOE_CLK_ENABLE();
__HAL_RCC_GPIOF_CLK_ENABLE();
```

---

## 统计信息

| 类别 | 数量 |
|------|------|
| 调试输出 | 4 |
| PWM输出 | 8 |
| 模拟输入(温度/电流/电压) | 20 |
| I2C接口 | 4 |
| UART接口 | 4 |
| 检测切换 | 4 |
| 保护输入 | 2 |
| 其他功能 | 8 |
| **总计** | **54** |

---

*报告生成完成*