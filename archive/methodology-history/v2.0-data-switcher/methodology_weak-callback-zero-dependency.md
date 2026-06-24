---
name: weak-callback-zero-dependency
description: __weak回调零依赖架构方法论v2.0——彻底消除消息调度器，每个模块可独立编译测试，链接器自动接线
metadata:
  type: methodology
  originSessionId: 7eb949aa-9578-417f-ab12-d6428ae59747
---

# __weak 回调零依赖架构 v2.0

> v1.0 (MsgScheduler) → v2.0 (__weak 直调) 的重大演进。
> 里程碑: 每个模块可独立编译、独立测试，无任何运行时基础设施依赖。

---

## 一、为什么这样演进

v1.0 的 MsgScheduler 虽然消除了模块间 include 耦合，但仍引入了三个运行时依赖:

| v1.0 残留依赖 | 问题 |
|--------------|------|
| `core/msg_scheduler.h` | 每个模块必须 include 它才能 Msg_Post / Register |
| 消息 ID `#define` | 每个模块必须定义本地 ID 并保证数值全局唯一 |
| `MsgScheduler_Run1ms()` | main 循环必须调用调度器消费队列 |

v2.0 全部消除:
- 无需 `#include "core/msg_scheduler.h"`
- 无需消息 ID 定义
- 无需 `MsgScheduler_Run1ms()` 调用
- 无需 `MsgScheduler_Register()`
- 无需 `Msg_Post()`

**发送方直接调用 `Receiver_OnXxx(param, data)` —— 就像调用自己写的一个函数一样简单。**

---

## 二、核心机制: __weak 回调

### 2.1 原理

```c
/* === 发送方模块 (如 drv_key.c) === */

/* 定义空壳: 如果没人接收, 静默丢弃 */
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
/* === 接收方模块 (如 app_hmi.c) === */

/* 强符号实现: 链接器自动覆盖发送方的 __weak 空壳 */
void AppHmi_OnKey(uint16_t param, void *data_ptr)
{
    uint8_t key_code  = (uint8_t)(param & 0xFFu);
    uint8_t key_state = (uint8_t)((param >> 8) & 0xFFu);
    (void)data_ptr;
    /* ... 业务处理 ... */
}
```

### 2.2 链接器行为

```
发送方定义: __weak void AppHmi_OnKey(...) { }   ← 总是存在(空壳)
接收方定义:        void AppHmi_OnKey(...) { }   ← 如果存在则覆盖

结果:
  有接收方 → 链接器选强符号 → 接收方函数被调用
  无接收方 → 链接器选弱符号 → 空壳运行, 静默丢弃, 无运行时错误
```

### 2.3 与 MsgScheduler 对比

| 方面 | v1.0 MsgScheduler | v2.0 __weak 直调 |
|------|-------------------|-------------------|
| 发送代码 | `Msg_Post(ID, param, &data)` | `Receiver_OnXxx(param, &data)` |
| 接收注册 | `MsgScheduler_Register(ID, handler)` | 强符号函数定义为 `Receiver_OnXxx` |
| 消息 ID | 需要, 全局唯一 | 不需要 |
| include | `"core/msg_scheduler.h"` | 无 |
| 队列 | 环形队列, 深度8 | 无队列, 直接调 |
| 异步性 | 消费端异步(1ms处理1条) | 同步直调(调用即执行) |
| 内存 | 队列缓冲+消息结构体 | 0 |
| 多接收方 | 多次 Msg_Post(同一ID) | 多次直接调(不同函数名) |
| 自收(self-post) | `Msg_Post(ID, ...)` → handler | `on_xxx(param, data)` 静态函数直接调 |
| 独立编译 | 需 msg_scheduler.o 链接 | 只需本模块 .o, 零外部依赖 |
| 单元测试 | 需 mock MsgScheduler | 直接调函数, 纯 C 函数测试 |

---

## 三、实施方法

### 3.1 从 v1.0 迁移到 v2.0 (已验证的步骤)

**第一步: 发送方添加 __weak 声明**

```c
/* 旧: Msg_Post(COOK_MSG_POWER_OUT, (uint16_t)head_idx, &s_power_cmd); */
/* 新: 添加 __weak + 改为直调 */

__weak void AppPower_OnPowerCtrl(uint16_t param, void *data_ptr)
{ (void)param; (void)data_ptr; }

/* 调用处: Msg_Post(...) → AppPower_OnPowerCtrl(...) */
```

**第二步: 接收方去除 MsgScheduler 注册**

```c
/* 旧:
static void on_power_ctrl(MsgId_t id, uint16_t param, void *data_ptr) { ... }
MsgScheduler_Register(PWR_MSG_CTRL_IN, on_power_ctrl);  // ← 删除
*/

/* 新: 改用约定函数名, 去掉 MsgId_t 参数 */
void AppPower_OnPowerCtrl(uint16_t param, void *data_ptr)
{
    /* 业务逻辑不变 */
}
```

**第三步: 清理 include 和 define**

```c
/* 删除:
#include "core/msg_scheduler.h"
#define COOK_MSG_POWER_OUT  2u
#define PWR_MSG_CTRL_IN     2u
*/
```

**第四步: 多接收方广播**

```c
/* 旧: Msg_Post(MSG_DATA_READY, ...)  ← 只有一个接收方能注册 */
/* 新: 多个函数逐一调用 */
AppPower_OnRegData((uint16_t)head_idx, &s_reg_data);
AppCooking_OnRegData((uint16_t)head_idx, &s_reg_data);
AppProtect_OnRegData((uint16_t)head_idx, &s_reg_data);
```

**第五步: 自收(self-post)消除**

```c
/* 旧: Msg_Post(COOK_MSG_CTRL_IN, param, NULL); → 调度器 → handler */
/* 新: on_cooking_ctrl(param, NULL);  ← 直接调静态函数 */
```

**第六步: 更新 interface_map.h**

```c
/* 旧记录:
│ ID │ Sender            │ Receiver         │
│ 2  │ COOK_MSG_POWER_OUT│ PWR_MSG_CTRL_IN  │
*/

/* 新记录:
│ #  │ __weak AppPower_OnPowerCtrl()  │ void AppPower_OnPowerCtrl() │
│    │ 发送方: app_cooking.c          │ 接收方: app_power.c         │
*/

/* 第七步: 移除 main.c 中的 MsgScheduler 依赖 */
/* 删除: MsgScheduler_Init(); MsgScheduler_Run1ms(); */
/* 删除: #include "core/msg_scheduler.h" */
```

### 3.2 本项目迁移统计

| 项目 | 数量 |
|------|------|
| 转换的消息通道 | 16 条 |
| 修改的生产文件 | 10 个 |
| 删除的 Msg_Post 调用 | 17 处 |
| 删除的 MsgScheduler_Register 调用 | 16 处 |
| 新增的 __weak 空壳函数 | 16 个 |
| 删除的消息 ID #define | 34 个 |
| 新增的 #include <stddef.h> | 8 个 |
| armcc 编译结果 | 10/10 0e0w |
| check_deps.py | 0 violations |
| msg_scheduler.c/h 状态 | 废弃(可删除) |

---

## 四、命名约定

### 4.1 函数命名

```
格式: {ModulePrefix}_On{Event}(uint16_t param, void *data_ptr)

APP 模块: App{Name}_On{Event}
  例: AppHmi_OnKey, AppPower_OnPowerCtrl, AppCommMgr_OnDataUpdate

DRV 模块: Drv{Name}_On{Event}
  例: DrvDisplay_OnRefresh, DrvBuzzer_OnCtrl, DrvCommMgr_OnSendReq
```

### 4.2 模块前缀表

```
APP 层:  AppHmi       (app_hmi.c)
         AppCooking   (app_cooking.c)
         AppPower     (app_power.c)
         AppProtect   (app_protect.c)
         AppCommMgr   (app_comm_mgr.c)

DRV 层:  DrvDisplay   (drv_display.c)
         DrvKey       (drv_key.c)
         DrvBuzzer    (drv_buzzer.c)
         DrvCommMgr   (drv_comm_mgr.c)

入口:    main.c       (无前缀, 定义 __weak 给 APP 接收)
```

### 4.3 函数签名统一

```c
/* 所有 __weak 回调和强符号实现必须使用相同签名: */
void {Module}_On{Event}(uint16_t param, void *data_ptr);

/* param:  轻量数据(键码、索引、布尔值等), 直接传值 */
/* data_ptr: 复杂数据指针, NULL 表示无数据 */
/* 返回值: 统一 void —— 异步通知模型, 不期望同步返回值 */
```

---

## 五、配对文档维护 (interface_map.h)

### 5.1 结构

`core/interface_map.h` 是 __weak 函数配对的**唯一真相源**。它包含两部分:

1. **结构体配对表** — 跨模块传递的数据类型, 独立声明同布局
2. **__weak 通道注册表** — 每条通道的发送方(__weak)/接收方(强符号)/数据类型

### 5.2 AI 维护规则

```
新增通道:
  1. 发送方 .c: 添加 __weak void Xxx_OnYyy(...) { } 空壳
  2. 发送方 .c: 在需要时调用 Xxx_OnYyy(param, data)
  3. 接收方 .c: 添加 void Xxx_OnYyy(...) { ... } 强实现
  4. interface_map.h: 注册通道条目 (发送方/接收方/数据类型)
  5. 如需新结构体: 两端独立声明, interface_map.h 注册配对

修改通道:
  1. 改任一方函数签名 → AI 同步改配对
  2. 改任一方结构体字段 → AI 同步改配对结构体
  3. interface_map.h 更新

删除通道:
  1. 删除发送方 __weak 声明和所有调用点
  2. 删除接收方强符号实现
  3. interface_map.h 移除该条目
```

### 5.3 验证方法

```bash
# 编译验证(最直接): 所有 .c 独立编译通过
armcc -c --cpu Cortex-M0+ -DSC32L14xx --c99 ... *.c → 0e0w

# 层依赖检查
python tools/check_deps.py Claude

# 函数签名一致性检查 (推荐新增工具)
# python tools/check_weak_pairs.py Claude
```

---

## 六、自动化工具

### 6.1 现有工具

| 工具 | 路径 | 检查项 |
|------|------|--------|
| check_deps.py | `tools/check_deps.py` | 层依赖: app 不 include drv/hal; drv 不 include app |
| check_msgs.py | `tools/check_msgs.py` | (v2.0 需更新: 去除 Msg_Post/Register 检查, 改为 __weak 配对检查) |

### 6.2 建议新增

```python
# tools/check_weak_pairs.py
# 检查项:
# 1. interface_map.h 中每条通道: 发送方文件是否存在 __weak 声明
# 2. interface_map.h 中每条通道: 接收方文件是否存在强符号实现
# 3. 函数签名一致性: 参数类型/顺序是否匹配
# 4. 结构体配对: sizeof 是否相等
# 5. 孤儿 __weak: 存在 __weak 声明但无强符号接收方(可能是漏删)
# 6. 孤儿强符号: 存在强符号实现但无发送方调用(可能是漏删)
```

---

## 七、适用条件与限制

### 适用场景
- 裸机/RTOS 嵌入式 C 项目
- 模块间通信量不大 (直调不会阻塞)
- 发送方和接收方在同一可执行文件中 (链接器可解析)
- AI 参与维护配对关系

### 不适用场景
- 需要跨进程/跨核通信 (无共享链接器)
- 需要消息持久化/延迟投递 (__weak 直调是同步的)
- ISR → 主循环解耦 (如果 ISR 中调用耗时, 需要手动缓冲)
- 动态加载插件 (__weak 在编译时解析)

### 关键限制: 同步性
```
v1.0 MsgScheduler: ISR → Msg_Post → 队列 → 主循环消费 → 回调
                    ↑ 异步, ISR 不会阻塞

v2.0 __weak 直调:  ISR → __weak 回调 → 直接执行业务逻辑
                    ↑ 同步, ISR 中不能调用耗时函数
```

本项目不受此限制影响——所有调用均在主循环 10 槽轮转中, 无 ISR 上下文。

---

## 八、迁移检查清单 (从 v1.0 到 v2.0)

迁移一个模块时的自检:

- [ ] 发送方: 添加 `__weak void Receiver_OnXxx(uint16_t, void*)` 空壳
- [ ] 发送方: 替换 `Msg_Post(ID, ...)` → `Receiver_OnXxx(param, data)`
- [ ] 发送方: 删除 `#include "core/msg_scheduler.h"`
- [ ] 发送方: 删除所有 `#define *_MSG_*` 
- [ ] 接收方: 删除 `MsgScheduler_Register(ID, handler)`
- [ ] 接收方: 将 handler 重命名为约定函数名, 去掉 MsgId_t 参数
- [ ] 接收方: 删除 `#include "core/msg_scheduler.h"`
- [ ] 接收方: 删除所有 `#define *_MSG_*`
- [ ] 两端: 检查 NULL 是否需要 `#include <stddef.h>`
- [ ] interface_map.h: 更新配对记录
- [ ] armcc 编译通过 (0e0w)
- [ ] check_deps.py 通过

---

## 九、架构意义

### 9.1 零依赖的终极形态

```
v1.0: APP ──Msg_Post──→ CORE(Scheduler) ──Register──→ DRV
                         ↑ 三个部分耦合到 CORE

v2.0: APP ──直接调用──→ DRV (通过 __weak 函数)
        ↑ 没有中间层, 链接器在编译时接线
```

每个 `.c` 文件现在可以:
- 独立编译: `armcc -c app_power.c` → 仅需本模块 .h
- 独立测试: 写测试框架调 `AppPower_OnPowerCtrl(param, data)` → 纯函数调用
- 独立替换: 更换实现只需换个 .c 文件, 不改任何其他代码

### 9.2 对 AI 的意义

人是管不住散落各处的 __weak 声明和强符号配对的——数量一多就乱。
但 AI 天生能:
- 成对管理: 新增/修改/删除都是原子操作
- 全局扫描: Grep 所有 __weak + 强符号 → 自动检测孤儿
- 签名验证: 参数类型/顺序一次性对比
- 文档同步: interface_map.h 与实际代码保持一致

**这就是 AI 原生架构——为人设计需要注册框架, 为 AI 设计只需要约定和一致性。**

---

*版本: 2.0 — 2026-05-26 确立于四头电磁炉项目*
*前身: [[ai-zero-coupling-methodology]] v1.0 (MsgScheduler 时代)*
*配套: [[independent-type-declarations]] [[ai-managed-dependency-injection]] [[interface_map.h]]*
*输出: 本方法论文档 + [[MANIFEST]] 完整文件清单*
