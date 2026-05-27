---
type: firmware_doc
status: draft
version: v1.0
tags: [firmware, lib, docs, embedded]
---

# 数码管标准显示固件库 (SMG_Disp_General_Lib) 用户手册

## 简介

`SMG_Disp_General_Lib` 是一个纯软件实现的数码管显示逻辑库，专为嵌入式应用设计。该库采用**黑盒交付**模式，用户无需关心内部实现细节，仅需通过头文件接口即可完成复杂的数码管显示任务。

### 核心特性
- **平台兼容性**：提供针对 **C51** 和 **ARM** 内核的预编译库文件（`.lib`），确保跨平台稳定运行。
- **硬件解耦**：库仅负责计算并填充显示缓冲区（`SMG_Disp_DATA`），不涉及任何底层 GPIO 操作或动态扫描逻辑。
- **多界面支持**：通过状态机管理，支持在同一系统中独立控制多个数码管显示界面。
- **丰富的显示模式**：内置数字、ASCII 字符、故障码、温度/功率格式、时间倒计时及多种边框动画效果。

### 适用硬件
- **数码管类型**：共阴极（Common Cathode）
- **位数支持**：默认支持 1-4 位数码管显示。

---

## 快速开始

### 1. 工程集成
用户只需在工程中包含头文件，并根据目标芯片链接对应的库文件：

| 目标平台 | 头文件 | 库文件 |
| :--- | :--- | :--- |
| **C51 系列** | `SMG_Disp_General_Lib.h` | `SMG_Disp_General_Lib_C51_L_1.4.LIB` |
| **ARM 系列** | `SMG_Disp_General_Lib.h` | `SMG_Disp_General_Lib_1.6.lib` |

> **注意**：版本号会随需求更新，请以实际提供的文件为准。库内部已处理 `<stddef.h>` 和 `<string.h>` 等依赖，用户无需额外配置。

### 2. 内存申请与初始化
用户需根据实际需要的显示界面数量，申请 `My_Disp_States` 数组。例如，若系统有 2 个独立的数码管显示区，则定义大小为 2。

```c
#include "SMG_Disp_General_Lib.h"

// 1. 定义状态数组：数量 = 独立显示的数码管组数
SMG_Disp_General_STATE_ My_Disp_States[2]; 

// 2. 定义显示缓冲区：每个数码管组对应一个 4 字节的缓冲区
unsigned char Disp_Buffer_Group1[4]; 
unsigned char Disp_Buffer_Group2[4]; 

void System_Init(void) {
    // 3. 初始化库：传入状态数组指针和数组大小
    if (Disp_General_Init(My_Disp_States, 2) != 0) {
        // 处理初始化失败（通常为内存不足）
    }
}
```

### 3. 闪烁同步机制
如果应用中使用了闪烁功能（如故障码闪烁、时间冒号闪烁），必须在系统的 **0.5s 定时器中断**或主循环中调用同步函数：

```c
// 假设 Sys_500ms_Flag 是系统产生的 0.5s 翻转标志位（0 或 1）
SMG_Disp_General_Lib_Flash_STA(Sys_500ms_Flag);
```

### 4. 硬件刷新（用户实现）
库本身不执行硬件输出。你需要在一个高频定时器（如 2ms）或主循环中，将 `Disp_Buffer` 的内容发送到物理数码管：

```c
void Timer_ISR(void) {
    // 1. 调用库函数更新缓冲区内容（例如显示高温动画）
    Disp_Hot(Disp_Buffer_Group1, 10, 0); 
    
    // 2. 用户自行实现的硬件扫描逻辑
    SMG_Port_Output(Disp_Buffer_Group1[Current_Bit]);
    SMG_Bit_Select(Current_Bit);
    Current_Bit = (Current_Bit + 1) % 4;
}
```

---

## 视觉参考标准

### 数码管段码样式
本库遵循标准的共阴极数码管段码映射。支持的字符 `a-z` 标准显示样式如下图所示：

![数码管标准显示样式](file:///C:/Users/ShiYunRuoYu/.real/users/user-33d8488c618644dcda42cf420d8d3f9d/sessions/2026-05-13/20260513_102016_97280006-709d-4ee4-b057-8313b2b9ede1/attachments/数码管标准显示.png)

> **说明**：图中展示了 `A-Z` 及常用符号在数码管上的实际点亮效果。部分字符（如 `K`, `M`, `V`, `W` 等）在数码管上受限于段位，采用了行业通用的近似显示方案。

---

## API 参考手册

### 基础显示功能

#### `Dsp_Font_ASCII`
**描述**：将 ASCII 字符串转换为数码管段码。支持 `0-9`, `A-Z` (部分字符区分大小写，详见上图), 空格, `-`。
**原型**：
```c
void Dsp_Font_ASCII(unsigned char *LedStr, char *font);
```
**参数**：
- `LedStr`：指向 4 字节显示缓冲区的指针。
- `font`：待显示的字符串指针。

**调用示例**：
```c
// 显示 "A-31"
Dsp_Font_ASCII(Disp_Buffer, "A-31"); 

// 显示 "PASS"
Dsp_Font_ASCII(Disp_Buffer, "PASS");
```

#### `Disp_Hex_H_L`
**描述**：以 16 进制格式显示两个字节的数据。
**原型**：
```c
void Disp_Hex_H_L(unsigned char *SMG_Disp_DATA, unsigned char data_h, unsigned char data_l);
```
**调用示例**：
```c
// 显示 "5512" (0x55 和 0x12)
Disp_Hex_H_L(Disp_Buffer, 0x55, 0x12);
```

---

### 业务场景显示

#### `Disp_Error_CODE`
**描述**：显示故障代码（如 E1, F5）。支持常亮或跟随系统标志闪烁。
**原型**：
```c
void Disp_Error_CODE(unsigned char DCode, unsigned char Number, unsigned char *Disp_DATA, unsigned char SET);
```
**参数**：
- `DCode`：故障前缀字符的段码（如 `SMG_Disp_E`）。
- `Number`：故障编号（0-99）。
- `SET`：`1` 为闪烁，`0` 为常亮。

**调用示例**：
```c
// 显示闪烁的 "E1"
Disp_Error_CODE(SMG_Disp_E, 1, Disp_Buffer, 1);
```

#### `Disp_Convention`
**描述**：通用的数值显示集合，支持功率、档位、温度等多种格式。
**原型**：
```c
void Disp_Convention(unsigned char *SMG_Disp_DATA, unsigned char Disp_MODE, unsigned int i);
```

**常用模式 (`Disp_MODE`) 枚举表**：

| 枚举值 | 含义 | 显示示例 | 备注 |
| :--- | :--- | :--- | :--- |
| `DF_Dip_def` | 常规 4 位数值 | `1234` | 自动消隐前导零 |
| `DF_Dip_P1` | 档位显示 (P1-P9) | `P 5` | 适用于电磁炉档位 |
| `DF_Dip_o` | 摄氏度显示 | `100` | 最后一位显示 °C |
| `DF_Dip_C` | 华氏度显示 | `212` | 最后一位显示 °F |
| `DF_Dip_16Hex` | 16 位 16 进制 | `ABCD` | 完整显示 4 位 16 进制 |

**调用示例**：
```c
// 显示功率 1200W
Disp_Convention(Disp_Buffer, DF_Dip_def, 1200);

// 显示档位 P5
Disp_Convention(Disp_Buffer, DF_Dip_P1, 5);
```

#### `Disp_Signed_Convention`
**描述**：支持有符号数值的显示，可处理负号。
**原型**：
```c
void Disp_Signed_Convention(unsigned char *SMG_Disp_DATA, unsigned char Disp_MODE, signed short num);
```
**调用示例**：
```c
// 显示 -25
Disp_Signed_Convention(Disp_Buffer, DF_Dip_def, -25);
```

#### `Disp_Time`
**描述**：复杂的时间显示逻辑，支持倒计时、正计时及多种闪烁策略。
**原型**：
```c
unsigned char Disp_Time(unsigned char *SMG_Disp_DATA, unsigned char Disp_MODE, unsigned char Flash, unsigned long Timer);
```
**返回值**：返回 `:` 冒号的闪烁状态（1 为亮，0 为灭），用户需据此控制小数点。

**常用模式 (`Disp_MODE`) 枚举表**：

| 枚举值 | 含义 | 输入单位 | 显示示例 |
| :--- | :--- | :--- | :--- |
| `DF_Dip_H_M` | 小时:分钟 | 分钟 | `01:30` |
| `DF_Dip_99M` | 0-99 分钟 | 分钟 | `59` |
| `DF_Dip_M_S` | 分钟:秒 | 秒 | `05:00` |
| `DF_Fla_ON` | 正常闪烁模式 | - | 配合 Flash 参数使用 |

**调用示例**：
```c
// 显示 1 小时 30 分，并获取冒号状态
unsigned char dot_status = Disp_Time(Disp_Buffer, DF_Dip_H_M, DF_Fla_ON, 90);
if (dot_status) {
    SMG_DOT_ON(); // 点亮冒号
} else {
    SMG_DOT_OFF(); // 熄灭冒号
}
```

---

### 动画与特效

#### `Disp_Hot`
**描述**：高温报警动画，显示 "Hot" 字样并从右向左跑马。
**原型**：
```c
void Disp_Hot(unsigned char *SMG_Disp_DATA, unsigned char Based_Speed, unsigned char IDx);
```
**调用示例**：
```c
// 在第一个显示通道（IDx=0）以速度 10 运行高温动画
Disp_Hot(Disp_Buffer, 10, 0);
```

#### `Disp_Flash_BianKuang`
**描述**：数码管边框跑马灯效果。
**原型**：
```c
void Disp_Flash_BianKuang(unsigned char *SMG_Disp_DATA, unsigned char Speed, unsigned char IDx);
```
**调用示例**：
```c
// 在第二个显示通道（IDx=1）运行边框动画
Disp_Flash_BianKuang(Disp_Buffer, 5, 1);
```

#### `Dsp_Roll_Font_ASCII`
**描述**：==新增功能== 支持长字符串在数码管上的双向滚动显示。
**原型**：
```c
void Dsp_Roll_Font_ASCII(unsigned char *LedStr, char *font, unsigned char speed, unsigned char SMG_Num, unsigned char IDx);
```
**参数**：
- `SMG_Num`：实际数码管位数（1-4）。
- `IDx`：显示通道索引。

**调用示例**：
```c
// 在 4 位数码管上滚动显示 "HELLO WORLD"
Dsp_Roll_Font_ASCII(Disp_Buffer, "HELLO WORLD", 5, 4, 0);
```

---

## 枚举类型详解

为了帮助用户正确使用 API，以下列出库中涉及的核心枚举类型及其完整取值。

### `Dsp_Font_ASCII` 字符支持表
下表列举了库中支持的标准 ASCII 字符及其在数码管上的显示形态：

| 字符类别 | 支持字符 | 说明 |
| :--- | :--- | :--- |
| **数字** | `0-9` | 标准七段显示 |
| **字母** | `A, b, C, d, E, F, H, h, L, n, o, P, q, r, S, t, U, y, Z` | 区分大小写的字符会显示不同形态 |
| **特殊符号** | `-` (横杠), ` ` (空格), `_` (下划线) | 用于组合显示 |

### 显示模式枚举 (`Disp_MODE`)
在 `Disp_Convention` 和 `Disp_Time` 中使用的模式选择：

| 枚举常量 | 值 (参考) | 功能描述 |
| :--- | :--- | :--- |
| `DF_Dip_def` | 0 | 默认 4 位十进制显示 |
| `DF_Dip_P1` | 1 | 档位显示 (P1-P9) |
| `DF_Dip_o` | 2 | 摄氏度显示 (°C) |
| `DF_Dip_C` | 3 | 华氏度显示 (°F) |
| `DF_Dip_H_M` | 4 | 小时:分钟显示 |
| `DF_Dip_99M` | 5 | 99 分钟内倒计时 |
| `DF_Dip_M_S` | 6 | 分钟:秒显示 |
| `DF_Dip_16Hex` | 7 | 16 进制全显 |

---

## 注意事项

1. **黑盒调用原则**：用户严禁尝试反编译或修改 `.lib` 库文件。所有功能必须通过 `SMG_Disp_General_Lib.h` 声明的接口进行调用。
2. **内存安全**：`My_Disp_States` 数组的大小必须与实际使用的 `IDx` 最大值匹配。若访问超出范围的 `IDx`，可能导致内存越界错误。
3. **闪烁同步**：如果不调用 `SMG_Disp_General_Lib_Flash_STA`，所有涉及闪烁的功能将默认为常亮或不闪烁。
4. **溢出处理**：当数值超过显示范围时，库会自动显示 "FULL" 错误提示。

---

## 版本历史

| 版本 | 日期 | 说明 |
| :--- | :--- | :--- |
| v1.0 | 2026-05-13 | 初始版本，完善 API 示例与枚举说明，整合视觉参考图 |
