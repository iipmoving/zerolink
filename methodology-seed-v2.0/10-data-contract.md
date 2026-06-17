# 10 — 数据交互说明书：模块 I/O 显式对齐

---

## 一、解决什么问题

模块间数据传递最常见的坑：

| 问题 | 后果 |
|------|------|
| A 模块输出 5 个字段，B 模块输入只用了 3 个 | 未来需求变更时漏同步 |
| 两模块用不同顺序声明相同字段 | 每次都要字段级 memcpy，无法指针直传 |
| 模块间数据流隐含在代码中，无人能完整看见 | 新人/新 AI 改一个模块就破坏三条链路 |
| 分组模块对内对外数据混在一起 | 外层中间层需要知道内部细节，违反分层 |

**数据交互说明书**在编码前解决这些问题——显式列出每条数据流，对齐输入输出结构体。

---

## 二、核心原则

### 2.1 指针直传原则

> **两个模块之间传递数据，要么结构体布局完全相同（直接传指针/一次 memcpy），要么显式说明为什么不能。**

```
✅ 推荐: A_Output 和 B_Input 布局一致
   B 的 InputCallback 中: memcpy(g_input.para, pOut->para, sizeof(A_Output));

❌ 禁止: 字段级逐一手动复制（除非结构体确实不同构，且理由充分）
   B 的 InputCallback 中: in->field1 = out->field1; in->field2 = out->field2; ...
```

### 2.2 组内对齐优先

模块组（一组高内聚小模块）内部的数据流，必须全部指针直传。组内不允许字段级搬运。

```
模块组 "功率控制":
  ├── Power_Adc    输出 → AdcResult_t    (与 Power_Calc 的输入对齐)
  ├── Power_Calc   输入 ← AdcResult_t
  │                输出 → PowerCmd_t     (与 Power_Drv 的输入对齐)
  └── Power_Drv    输入 ← PowerCmd_t
```

### 2.3 组间隔离

模块组对外的数据结构是组的"公共接口"，组内模块不直接暴露给外部。

---

## 三、数据交互说明书格式

### 3.1 模块清单

```
## 模块清单

| 模块 | 层 | 职责 | 输入来源 | 输出去向 |
|------|----|------|---------|---------|
| Power_Adc | app | ADC 采样 | — | Power_Calc |
| Power_Calc | app | 功率计算 | Power_Adc | Power_Drv |
| Power_Drv | base_class | HRTIM 驱动 | Power_Calc | — |
```

### 3.2 I/O 结构体定义 (预对齐)

所有模块 I/O 结构体必须使用 `std_module.h` 中定义的命名宏，确保命名统一、便于工具检查：

| 宏 | 展开 | 用途 |
|----|------|------|
| `MODULE_INPUT(PowerCalc)` | `PowerCalc_Input` | 模块输入结构体 |
| `MODULE_OUTPUT(PowerCalc)` | `PowerCalc_Output` | 模块输出结构体 |
| `MODULE_OUTPUT_LINK(owner, consumer)` | `{owner}_to_{consumer}_Output_Link` | 两模块对齐的数据列结构体（必含 `same as` 注解） |

Link 类型用于生产者和消费者结构体完全一致的情形，必须加注解说明与谁相同：

```c
typedef struct {
    uint16_t current[4];         /* same as Adc_Output / Calc_Input */
    uint16_t voltage;
} MODULE_OUTPUT_LINK(Adc, Calc);
```

对每条数据流，显式列出生产者输出和消费者输入的字段布局，要求完全一致：

```
## 数据流 #1: Power_Adc → Power_Calc

// Power_Adc_Output  (MODULE_OUTPUT(Power_Adc))
// Power_Calc_Input  (MODULE_INPUT(Power_Calc))
// 约束: 两结构体 sizeof 相等, 字段顺序一致, 类型一致

struct {
    uint16_t    current[4];       // 4通道电流采样
    uint16_t    voltage;           // 总线电压
    uint16_t    temp_igbt;         // IGBT 温度
    uint8_t     valid_ch_mask;    // 有效通道掩码
    uint8_t     res[3];           // 对齐填充
} → sizeof = 18, 可直接 memcpy
```

### 3.3 数据流图 (文本)

```
ADC_IRQ
  │
  ▼
Power_Adc.DoWork ──→ g_output (Adc_to_Calc_Output_Link)
                        │
                        │ memcpy (InputCallback)
                        ▼
                    Power_Calc.g_input (Adc_to_Calc_Output_Link)
                      │
                      ▼ Power_Calc.DoWork ──→ g_output (Calc_to_Drv_Output_Link)
                                                 │
                                                 │ memcpy (InputCallback)
                                                 ▼
                                             Power_Drv.g_input (Calc_to_Drv_Output_Link)
```

### 3.4 组内数据流 (详细)

模块组内部的数据流需要更多细节，因为多个内部模块共享同一组数据结构：

```
## 模块组: "功率控制" 内部数据流

组内模块:
  Power_Adc  → Power_Calc  → Power_Drv

组内结构体:
  AdcResult_t  (Adc_to_Calc_Output_Link — Power_Adc 输出 / Power_Calc 输入)
  PowerCmd_t   (Calc_to_Drv_Output_Link — Power_Calc 输出 / Power_Drv 输入)

组外接口:
  本组对外暴露: PowerHw_Status_t (Drv 反馈给 App)
  本组接收外部: PowerHw_Command_t (App 发给 Drv)

对齐约束:
  Power_Adc.Output  ≡ Power_Calc.Input   → sizeof(Adc_to_Calc_Output_Link)
  Power_Calc.Output ≡ Power_Drv.Input    → sizeof(Calc_to_Drv_Output_Link)
```

---

## 四、工作流程

### 4.1 何时做

**项目规划阶段**（模块拆分完成后，编码开始前），以及**每次新增模块/修改模块 I/O 时**。

### 4.2 步骤

```
Step 1: 列出所有模块及其数据依赖
  产出: 模块清单表

Step 2: 对每条数据流, 对齐生产者输出和消费者输入
  产出: I/O 结构体定义 + sizeof 约束

Step 3: 检查组内数据流是否全部指针直传
  产出: 组内对齐确认

Step 4: 更新 data_switcher.c 的 InputCallback 为 memcpy (而非字段级复制)
  产出: 简洁的 InputCallback

Step 5: 将数据交互说明书提交到代码仓库 (docs/data-contract.md)
  产出: 可追溯的数据契约
```

### 4.3 验证

- 数据交互说明书与代码中的 `_io.h` 结构体定义**保持一致**（人工 code review）
- 所有 InputCallback 中不应出现字段级复制，除非结构体确实不同构
- `/check` 中的 `check_structs.py` 检查跨模块结构体一致性

---

## 五、与现有机制的关系

| 机制 | 时机 | 用途 | 与数据契约关系 |
|------|------|------|--------------|
| `interface_map.h` | 编码中 | __weak 回调配对表 | 互补: interface_map 管回调, 数据契约管数据流 |
| `cfg/structs.json` + `generate_structs.py` | 编码前 | 跨模块结构体生成 | 数据契约是 structs.json 的设计输入 |
| `check_structs.py` | 编码后 | 结构体一致性检查 | 验证数据契约的执行情况 |
| `MODULE_INPUT/OUTPUT/LINK` (std_module.h) | 编码中 | I/O 结构体命名宏 | 数据契约中定义的 I/O 类型通过此宏实现命名统一 |
| `_io.h` | 编码中 | 模块 I/O 结构体定义 | 数据契约中定义的 struct 在此实现 |
| 数据交互说明书 | **编码前** | 模块 I/O 对齐设计 | 统领以上所有 |

---

## 六、示例：功率控制模块组

### 数据交互说明书摘要

```
## 模块组: 功率控制 (PowerGroup)

### 组内模块链
  AdcSensor → PowerCalc → DrvHrtim

### 结构体对齐

1. AdcSensor.Output ↔ PowerCalc.Input
   共用: AdcSensor_to_PowerCalc_Output_Link { uint16_t current[4]; uint16_t voltage; }  /* same as AdcSensor.Output / PowerCalc.Input */
   → sizeof = 10, 完全一致, 指针直传

2. PowerCalc.Output ↔ DrvHrtim.Input
   共用: PowerCalc_to_DrvHrtim_Output_Link { uint16_t duty[4]; uint16_t period; uint8_t enable; uint8_t res; }  /* same as PowerCalc.Output / DrvHrtim.Input */
   → sizeof = 12, 完全一致, 指针直传

### 组外接口
   外部 → PowerGroup: HwStatus_t   (HRTIM 状态反馈)
   PowerGroup → 外部: HwCommand_t  (HRTIM 控制命令)
   → 中转由 data_switcher.c 的 InputCallback 完成 (memcpy)
```

---

## 七、常见反模式

| 反模式 | 问题 | 正确做法 |
|--------|------|---------|
| InputCallback 中逐字段复制 | 结构体不同步时漏字段 | 对齐结构体后用 memcpy |
| 模块组内模块暴露内部结构体到外部 | 外部模块依赖内部细节 | 组内结构体加 `/* internal */` 注释, 不被组外引用 |
| "先写着，后面再对齐" | 永远不会有时间回来对齐 | 编码前在数据契约中定好结构体 |
| 两个模块 I/O 字段相同但顺序不同 | memcpy 会错位 | 按数据契约统一顺序 |

---

*方法论版本: v2.3, 2026-06-10*
