# 02 — __weak 回调零依赖通信模式

---

## 一、原理

```c
/* === 发送方模块 === */

/* 定义空壳: 如果没人接收，静默丢弃 */
__weak void AppHmi_OnKey(uint16_t param, void *data_ptr)
{ (void)param; (void)data_ptr; }

/* 发送时直接调用 (不经过任何中间层) */
static void Key_PostEvent(uint8_t key_code, uint8_t key_state)
{
    uint16_t param = (uint16_t)key_code | ((uint16_t)key_state << 8);
    AppHmi_OnKey(param, NULL);       /* 调用 __weak 声明 */
    AppCooking_OnKey(param, NULL);   /* 多接收方: 逐一调用 */
}
```

```c
/* === 接收方模块 === */

/* 强符号实现: 链接器自动覆盖发送方的 __weak 空壳 */
void AppHmi_OnKey(uint16_t param, void *data_ptr)
{
    uint8_t key_code  = (uint8_t)(param & 0xFFu);
    uint8_t key_state = (uint8_t)((param >> 8) & 0xFFu);
    (void)data_ptr;
    /* ... 业务处理 ... */
}
```

### 链接器行为

```
发送方定义: __weak void AppHmi_OnKey(...) { }   ← 总是存在(空壳)
接收方定义:        void AppHmi_OnKey(...) { }   ← 如果存在则覆盖

结果:
  有接收方 → 链接器选强符号 → 接收方函数被调用
  无接收方 → 链接器选弱符号 → 空壳运行, 静默丢弃, 无运行时错误
```

---

## 二、__weak / Msg_Post / 数据交换机 三种通信机制

三者**不互相替代**，按调用时机和数据类型选择。

| 方面 | __weak 直调 | Msg_Post 消息 | 数据交换机 |
|------|-------------|---------------|-----------|
| **适用** | 同时间片连续执行 | 跨时间片 / 跨进程 | 周期性结构化数据路由 |
| **时序** | 同步，调用即执行 | 异步，队列缓冲 | 调度槽内顺序执行 |
| 发送代码 | `Receiver_OnXxx(param, &data)` | `Msg_Post(ID, param, &data)` | producer ProcessInput 写 g_output.para (不置状态位) |
| 接收注册 | 强符号同名函数 | `Register(ID, handler)` | consumer {Consumer}_On{Producer}Data(Para_Grp_t *pOut) — Switcher 显式调用 |
| 消息 ID | 不需要 | 需要，全局唯一 | 不需要 |
| 队列 | 无，直接调 | 环形队列 | 无，Switcher 显式 PULL |
| 运行时内存 | 0 | 队列缓冲+消息体 | Para_Grp_t g_input/g_output (MODULE_SKELETON 展开) |
| 独立编译 | 零外部依赖 | 需 msg_scheduler.o | 需 Switcher (注册 DoWork + pOut) |
| 多接收方 | 多个 __weak 逐一调用 | 多次 Msg_Post | Switcher 路由函数逐一调 consumer 回调 |
| 模块间互知 | 发送方知道函数名 | 发送方知道消息 ID | 模块间零互知，Switcher 持有全部路由 |

### 选择规则

```
问: "这个操作必须在同一次调度槽内完成？"
  → 是: __weak 直调 ← 调用即执行，无延迟
  → 否:
    问: "传递的是事件通知还是结构化数据块？"
      → 事件通知: Msg_Post        ← 写入队列，下个时间片消费
      → 结构化数据块: 数据交换机    ← Switcher PULL 路由 + Para_Grp_t, consumer 自己 memcpy

数据交换机的核心价值: 消除 consumer struct 副本 (_LINK_t)，
数据路由集中在 Switcher，owner 字段变更只改接线不改 N 个 consumer。
详见 08-data-switcher.md。
```

### 典型分工

| 场景 | 用谁 | 原因 |
|------|------|------|
| DRV → HAL | __weak | 同步硬件操作，需立即生效 |
| 主模块 → 算法集 | __weak | 连续计算，无延迟 |
| ISR → 业务模块 | __weak | 延迟敏感，不走中间层 |
| 按键 → 业务逻辑 | Msg_Post | 不在 ISR 中处理，延迟到主循环 |
| 状态变更广播 | Msg_Post | 多模块异步感知，不需即时 |
| 显示帧下发 | __weak 或回调指针 | COM 扫描每 1ms，不能异步排队 |
| 周期性数据路由 (ADC→Power 等) | 数据交换机 | 结构化数据块，模块不需要 _LINK_t 副本 |

---

## 三、实施步骤

### Step 1: 发送方添加 __weak 声明
```c
__weak void Receiver_OnXxx(uint16_t param, void *data_ptr)
{ (void)param; (void)data_ptr; }
```

### Step 2: 发送方调用
```c
/* 替换原有的 Msg_Post(ID, ...) */
Receiver_OnXxx(param, &data);
```

### Step 3: 接收方强实现
```c
/* 去掉 MsgId_t 参数，改用约定函数名 */
void Receiver_OnXxx(uint16_t param, void *data_ptr) {
    /* 业务逻辑不变 */
}
```

### Step 4: 多接收方广播
```c
/* 为每个接收方声明独立的 __weak 空壳，逐一调用 */
Receiver1_OnData(param, &data);
Receiver2_OnData(param, &data);
Receiver3_OnData(param, &data);
```

### Step 5: 更新 interface_map.h
```c
/* Pair X: module_a → module_b */
/* 发送方: module_a.c  WEAK void ModuleB_OnEvent(uint16_t param, void *data_ptr) {} */
/* 接收方: module_b.c  void ModuleB_OnEvent(uint16_t param, void *data_ptr) */
```

---

## 四、__attribute__((weak)) 统一规则（ARMCLANG V6）

```c
// ARMCLANG V6 (Keil V5.38+) 不识别裸 __weak
// → 全项目统一使用 __attribute__((weak))

/* 发送方 — 正确 */
__attribute__((weak)) void Receiver_OnXxx(void *input)
{ (void)input; }

/* 接收方 — STRONG，不加任何修饰 */
void Receiver_OnXxx(void *input)
{
    // ...
}
```

**铁律**: 本项目编译器为 ARMCLANG V6.12，一律用 `__attribute__((weak))`，禁止裸 `__weak`。

**编码**: 所有源文件统一 UTF-8（无 BOM），Keil → Edit → Configuration → Editor → Encoding = UTF-8。

---

## 五、AI 行为规范：先反馈理解，再行动

每次接到指令后：

```
1. 用自己的话重述目标 → 等用户确认
2. 确认后再改代码
3. 不跳步，不猜测
```

**反例**:
- 用户说"把指针传进来" → AI 加了 data_ready、加了 power_out、套了 struct 壳 → 没确认就动手
- 用户说"存指针" → AI 复制了一份数据到本地 struct → 没理解就执行

**正例**:
- 用户说"传 &AdcFunRam.inputValue" → AI: "收到，只传这个指针，STRONG 里只存指针+设 APP_PPG_SetIcVcOk()，不调 PULL，对吗？" → 确认 → 执行

---

## 五、命名约定

```
格式: {ModulePrefix}_On{Event}(uint16_t param, void *data_ptr)

APP 模块: App{Name}_On{Event}
  例: AppHmi_OnKey, AppPower_OnPowerCtrl

DRV 模块: Drv{Name}_On{Event}
  例: DrvDisplay_OnRefresh, DrvBuzzer_OnCtrl

所有 __weak 回调和强符号实现必须使用相同签名。
param:  轻量数据(键码、索引、布尔值), 直接传值
data_ptr: 复杂数据指针, NULL 表示无数据
返回值: 统一 void
```

---

## 六、限制与注意

### 同步性
__weak 直调是同步的——调用即执行。ISR 中不能直接调用耗时函数。应所有调用在主循环中，ISR 只设标志。

### 单可执行文件
__weak 依赖链接器在编译时解析。所有模块必须链接到同一个可执行文件。不适用于动态加载插件。

### 多接收方
每个接收方需要独立的 __weak 空壳声明和独立的调用语句。发送方显式知道所有接收方（通过函数名），但不知道接收方是谁、做什么。

---

## 七、大模块调用纯算法集（不 include .h）

### 7.1 问题

传统做法: 大模块 `#include "algo.h"` → 调用算法函数。这引入了编译期依赖——算法集的类型定义、内部 include 全部暴露给调用方。

### 7.2 解法: __weak 隔离

```
app_power.c (主模块, 调用方)
  ├─ __weak AlgoRamp_Calculate(param)  ← 声明空壳（不 #include algo.h）
  └─ AppPower_Run():
        result = AlgoRamp_Calculate(...)  ← 直接调用

algo_ramp.c (纯算法集, 被调用方)
  └─ AlgoRamp_Calculate(param)      ← 强符号实现（覆盖 __weak 空壳）
```

**关键**: 调用方不 include 算法集头文件。两个 .c 之间唯一的耦合是函数名和签名——由链接器 + `interface_map.h` + `check_weak_pairs.py` 管理。

### 7.3 完整示例

```c
/* ======== app_power.c (主模块) ======== */
#include "app_power.h"   /* 只包含自己的头文件 */
/* 不包含 algo_ramp.h */

/* __weak 声明 — 替代 #include "algo_ramp.h" */
__weak uint16_t AlgoRamp_Calculate(uint16_t target, uint16_t current,
                                    uint16_t step_up, uint16_t step_down)
{ return target; }  /* 空壳: 无算法集时直通 */

void AppPower_Run(void)
{
    /* 输入段 */
    /* 计算段 */
    s_power = AlgoRamp_Calculate(s_target, s_current,
                                  cfg.step_up, cfg.step_down);
    /* 输出段 */
}

/* ======== algo_ramp.c (纯算法集) ======== */
#include "algo_ramp.h"   /* 只包含自己的头文件 */

/* 强符号 — 覆盖上层 __weak */
uint16_t AlgoRamp_Calculate(uint16_t target, uint16_t current,
                             uint16_t step_up, uint16_t step_down)
{
    if (target > current) {
        return (target - current > step_up) ? current + step_up : target;
    } else if (target < current) {
        return (current - target > step_down) ? current - step_down : target;
    }
    return target;
}
```

### 7.4 三层结构映射

```
主模块 (APP)      __weak 调子模块          不含 #include "algo_*.h"
    ↓ __weak 直调
纯算法集           强符号覆盖              不含 #include "app_*.h" / "drv_*.h"
    ↓ (无返回值函数时 __weak 回调)
主模块            输出回调                 不含 #include "drv_*.h"
    ↓ (跨时间片)
Msg_Post          异步通知                 队列缓冲
```

**每层都不 include 下一层的头文件。** 唯一交叉点是函数名——由配对表 + 工具验证。

---

## 八、APP 模块间数据传递标准模式（案例）

### 8.1 反模式：task 层编排

```c
/* === app_task.c (调度层) — 错误做法 === */
#include "APP_ADC.H"
#include "app_power.h"

void Task_TimeChip1(void)
{
    uint8_t icVcOk = AdcValueFun();    // 直接调模块 A
    PowerTypeFun(icVcOk);              // 把 A 的结果传给 B — task 不该知道这个协议
}
```

**问题**:
1. task 层 `#include "APP_ADC.H"` — 跨模块 include
2. task 层知道 APP_ADC 和 app_power 之间的握手协议（`icVcOk` 这个值）
3. `PowerTypeFun` 用参数接收外部状态 — 输入来源不透明
4. 新增数据传递时要在 task 层加参数 → 所有模块签名跟着变

### 8.2 标准模式：__weak 输出 → STRONG 本地缓存 → 主入口消费

```
发送方 (APP_ADC)                     接收方 (app_power)
─────────────                        ─────────────
__weak AppAdc_OnDataReady(void) {}   static uint8_t _adc_data_ready = 0;
                                     void AppAdc_OnDataReady(void) {
void AdcValueFun(void) {                 _adc_data_ready = 1;  // 只搬运
    // ... 填充数据 ...              }
    AppAdc_OnDataReady();  ←──weak──→ 
    return;                           void PowerTypeFun(void) {
}                                         if (_adc_data_ready) {  // 消费
                                              // ... 使用已准备好的数据 ...
                                              _adc_data_ready = 0;
                                          }
                                      }
```

**关键原则**:
- **发送方**: 在输出段调 `__weak`，不关心谁接收
- **接收方 STRONG**: 只做"数据搬运"（复制值/存指针/设标志），不做业务逻辑
- **接收方主入口**: 消费本地缓存，不从参数拿外部状态
- **task 层**: 只按时序调用各模块入口，不传递数据

### 8.3 真实案例：APP_ADC → app_power (m4_ekf_observer)

**改前** (`app_task.c`):
```c
uint8_t icVcOk = AdcValueFun();    // task 直接调 APP_ADC
PowerTypeFun(icVcOk);              // 参数传递握手信号
```

**改前** (`app_power.c`):
```c
void PowerTypeFun(uint8_t newIcVc) {
    if (newIcVc) {
        APP_PPG_SetIcVcOk();       // 外部状态 → 内部标志
    }
}
```

**改后** (`APP_ADC.C`):
```c
/* __weak 桩 — Pair O */
__attribute__((weak)) void AppAdc_OnDataReady(void) {}

uint8_t AdcValueFun(void) {
    // ... 填充 g_adc_raw ...
    if (xReturn) {
        AppAdc_OnDataReady();      // 通知: 数据就绪 (不关心谁接收)
    }
    return xReturn;
}
```

**改后** (`app_power.c`):
```c
static uint8_t _adc_data_ready = 0;

void AppAdc_OnDataReady(void) {     // STRONG: 只设标志
    _adc_data_ready = 1;
}

void PowerTypeFun(void) {           // 签名去参数
    if (_adc_data_ready) {          // 消费本地缓存
        APP_PPG_SetIcVcOk();
        _adc_data_ready = 0;
    }
}
```

**改后** (`app_task.c`):
```c
void Task_TimeChip1(void) {
    AdcValueFun();                 // 只按时序调用
    PowerTypeFun();                // 无参数传递
}
```

**效果**: 新增数据字段只需扩展 `g_adc_raw` struct + `AppAdc_OnDataReady` 签名，task 层和 `PowerTypeFun` 参数列表不受影响。
