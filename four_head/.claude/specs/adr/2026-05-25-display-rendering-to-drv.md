# ADR: 显示渲染下沉 — 段码/LED 模式归 DRV 层

**日期**: 2026-05-25
**状态**: 已决策（2026-05-26 确认回调方案）
**决策者**: 技术负责人

---

## 背景

当前架构中，APP/API 层直接生成段码（SegCode）和 LED 点位（LED bitmap）传给 DRV 层映射 IO：

```
APP/API                         DRV
  api_display: 生成段码 ──────→ drv_display: 段码→IO
  api_display: 生成LED点位 ────→ drv_display: 点位→IO
  api_key:     热点炉头逻辑
```

问题：
- **硬件泄漏**：换段码库（SMG Lib）或改引脚 → 改 APP 层
- **职责混乱**：APP 层知道"数字5对应哪些段亮"，这是渲染知识，不是业务逻辑
- **与 JSON 架构矛盾**：HMI JSON 已确立"逻辑层出值、控件属性管渲染"的分层原则，C 代码却耦合在一起
- **LED 显示模式（单点/梯度）** 目前在逻辑层决定，同样属于渲染决策

## 决策

**三层模型：APP 逻辑出抽象值 → 显示装配器(半层)转段码 → DRV 统一接口收段码映射 IO。**

### 核心思路

如果 APP 直接出抽象值、DRV 负责全部转换，DRV 接口会膨胀（`ShowDigit`、`ShowPattern`、`SetBlink`、`SetLED`、`SetDP`……）。折中方案是在 APP 层加一个 **显示装配器（Display Assembler）**，它不参与业务逻辑，只做一件事：收抽象值，调 SMG Lib 生成段码，通过统一回调传给 DRV。

### 新分层

```
APP 逻辑层 (纯业务)                  
  app_cooking / app_power:           
    输出抽象值                        
    {zone:1, power:5, state:cooking} 
                                     
APP 半层 — 显示装配器 (api_display)   
  抽象值 → 段码 | LED点位 | DP | 闪烁  
  调用 SMG Lib | 决定显示模式          
         ↓ 统一回调: seg_code + led_bits + ctrl_flags
DRV (渲染+IO层)                       
  drv_display: 段码→IO | 点位→IO      
  只做硬件映射，不碰 SMG Lib          
```

### 接口契约：统一回调

DRV 暴露唯一入口，避免接口膨胀：

```c
/* DRV 层 —— 唯一入口 */
typedef struct {
    uint8_t  seg_code[11];   /* 11 COM 的段码, 由装配器填好 */
    uint16_t led_bits;       /* LED 点位 bitmap, 由装配器填好 */
    uint8_t  dp_map;         /* DP 映射, 由装配器填好 */
    uint8_t  blink_mask;     /* 闪烁掩码(哪些 COM 需要闪烁) */
} DisplayFrame_t;

void Drv_Display_Commit(const DisplayFrame_t *frame);
```

DRV 收到 `DisplayFrame_t` 后只管一件事：把位映射到 IO 引脚。不知道段码怎么来的、不知道 LED 为什么亮。

### 显示装配器职责 (api_display)

```c
/* api_display —— 半层, 装配 DisplayFrame_t */

/* APP 逻辑调用这些 —— 输出抽象值 */
void API_Display_SetZoneDigit(uint8_t zone_id, uint8_t digit);
void API_Display_SetZonePattern(uint8_t zone_id, SegPattern_t pattern); /* "--", "PA", "P" */
void API_Display_SetZoneBlink(uint8_t zone_id, bool blink);
void API_Display_SetPowerLED(uint8_t power_level);
void API_Display_SetHotHead(int8_t zone_id);
void API_Display_SetIndicator(Indicator_t id, bool on);

/* 装配器在每个显示周期(1ms/com)调用一次, 生成当前 COM 的段码 */
void API_Display_Assemble(uint8_t com_id, DisplayFrame_t *out);
```

内部实现：
1. `SetZoneDigit(zone, 5)` → 存抽象值到缓存
2. `Assemble(com_id)` → 取出该 COM 对应的 zone 抽象值 → 查 SMG Lib → 写 `seg_code[com_id]`、决定 `led_bits`、`dp_map`、`blink_mask`
3. 上层调用 `Drv_Display_Commit(&frame)` 下发

### LED 显示模式在装配器内决定

| 模式 | 效果 | 归属 |
|------|------|------|
| 单点 (single) | 仅该档位 LED 亮 | 装配器配置 |
| 梯度 (gradient) | ≤档位 的 LED 全亮 | 装配器配置 |

APP 逻辑层只知道 `power_level=5`，装配器根据配置决定 LED 显示模式。

### DP 控制

DP 作为热点炉头指示器（见通用语言 §8.1）：
- 装配器收到 `SetHotHead(1)` → 组装时 Z1 Slot 的 DP 置位
- DP 不跟随闪烁相位（数字闪烁时 DP 保持常亮）

### 为什么不是纯 DRV 方案

| 方案 | DRV 接口数 | DRV 复杂度 | APP 复杂度 |
|------|-----------|-----------|-----------|
| 纯 DRV（抽象值下到 DRV） | 多（数字/模式/闪烁/DP/LED…各一接口） | 高（集 SMG Lib + IO） | 低 |
| **折中 装配器半层** | **1（Commit）** | **低（纯 IO 映射）** | **中（装配器不参与业务）** |
| 现状（APP 直接出段码） | 1（段码数组） | 低 | 高（业务+段码混一起） |

折中方案的核心收益：**DRV 接口不膨胀，业务逻辑不沾渲染。**

### 回调机制：装配器 → DRV 的传参方式

装配器不直接调用 DRV 函数（禁止 `#include "drv_display.h"`），而是通过 DRV 在初始化时注册的回调指针传参：

```c
/* DRV 注册回调给装配器 */
typedef void (*DisplayCommitFn)(const DisplayFrame_t *frame);

/* api_display.h —— 装配器暴露注册入口 */
void API_Display_RegisterCommitCallback(DisplayCommitFn fn);

/* DRV 初始化时注册 */
Drv_Display_Init(void) {
    API_Display_RegisterCommitCallback(Drv_Display_Commit);
}
```

装配器每 1ms 调用注册的回调，DRV 收到 `DisplayFrame_t` 后映射 IO。

### 通信分界：1ms 同步回调 vs 消息队列

```
同一 1ms 内必须完成的操作：
  显示帧下发 ──→ 回调 (Drv_Display_Commit)

跨越多个 1ms 的操作：
  按键事件 ──→ Msg_Post(MSG_KEY_EVENT) → 调度器 → 处理器
  状态变更 ──→ Msg_Post(MSG_*) → 调度器 → 处理器
  定时事件 ──→ Msg_Post(MSG_TIMER_*) → 调度器 → 处理器
```

**规则**：新增模块时先问"这个操作必须在同一次 1ms 调度内完成？"是则回调，否则消息队列。

## 后果

### 正面
- **DRV 接口统一**：只有 `Commit(DisplayFrame_t)` 一个入口，不随功能膨胀
- **硬件变更隔离**：改引脚、换段码库 → 只改装配器（`api_display`）
- **业务逻辑干净**：`app_cooking` 不碰段码、不碰 LED bitmap
- **与 JSON 架构对齐**：装配器等价于 widgets.json，APP 逻辑等价于 rules.json
- **可测试性**：测业务逻辑只需检查抽象值输出；测装配器只需检查 DisplayFrame_t 内容

### 代价
- 新增装配器模块（约 200 行），维护一层转换
- 每 1ms COM 周期多一次装配调用（轻量，仅查表+移位）
- 现有 `api_display` 需重构

### 迁移路径
1. 定义 `DisplayFrame_t` 结构体和 `Drv_Display_Commit()` 接口
2. 新建装配器模块，实现 `API_Display_SetXxx()` + `Assemble()`
3. APP 逻辑逐步从旧 `api_display` 切到新接口
4. 旧段码生成代码移除，回归测试

---
*关联: [[ubiquitous-language]], [[hmi-json-message-test-plan]]*
