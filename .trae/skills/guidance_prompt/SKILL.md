---
name: guidance_prompt
description: "审查报告与指导 — v2.3 命名约定整改. 用于 AI 处理 four_head 项目时，按照指导修复 v2.3 PULL 架构的命名约定和宏使用偏差. Triggers on: guidance, 审查报告, 命名约定, v2.3整改."
user-invocable: true
---

# /guidance_prompt — v2.3 命名约定整改指导

处理 four_head 项目 v2.3 迁移时，按照以下说明修复问题。

---

## 一、偏差摘要

### P0 — 必须修复：命名约定违规

**规则**：生产者 OUTPUT_LINK 成员名 = `{Consumer}_params`，消费者 INPUT_LINK 成员名 = `{Producer}_params`

| 文件 | 当前（错误） | 应改为 |
|------|-------------|--------|
| `drv_key_io.h` `MODULE_OUTPUT(DrvKey)` | `*to_hmi` | `*Hmi_params` |
| | `*to_cooking` | `*Cooking_params` |
| | `*to_seg_align` | `*SegAlign_params` |
| `app_power_io.h` `MODULE_OUTPUT(AppPower)` | `*to_drv_comm` | `*DrvCommMgr_params` |
| | `*to_hmi` | `*Hmi_params` |
| `app_hmi_io.h` `MODULE_OUTPUT(AppHmi)` | `*to_display` | `*Display_params` |
| | `*to_buzzer` | `*Buzzer_params` |
| `app_hmi_io.h` `MODULE_INPUT(AppHmi)` | `*key` | `*DrvKey_params` |
| | `*power` | `*AppPower_params` |
| `app_power_io.h` `MODULE_INPUT(AppPower)` | `*comm` | `*CommMgr_params` |
| | `*protect` | `*Protect_params` |
| | `*cooking` | `*Cooking_params` |

**受影响的源文件**（更新对所有重命名成员的引用）：
- `drv_key.c`：`s_out.to_hmi` → `s_out.Hmi_params`，依次类推
- `data_switcher.c`：`key_out->to_hmi` → `key_out->Hmi_params`，`hmi_in->key` → `hmi_in->DrvKey_params`，依次类推
- 任何引用了旧名称的 app_hmi.c/app_power.c/app_comm_mgr.c

### P1 — 应该修复：宏使用

**规则**：`data_switcher.c` 中的所有 InputCallback 实现应使用 `INPUT_CALLBACK` + `INPUT_GET_SLOT` 宏。

**当前**：
```c
void AppHmi_InputCallback(void) {
    // ... hand-written manual casts
    hmi_in->key = (Key_to_Hmi_Input_Link *)key_out->to_hmi;
    hmi_in->key->status |= ST_NEW;
}
```

**应改为**：在 `data_switcher.c` 中添加宏定义：
```c
#define INPUT_CALLBACK(producer, consumer) void consumer##_InputCallback(void)

#define INPUT_GET_SLOT(producer, consumer) \
    MODULE_OUTPUT(producer) *__out = (MODULE_OUTPUT(producer) *)s_slots[SLOT_##producer].pOut->para; \
    MODULE_INPUT(consumer) *__in = (MODULE_INPUT(consumer) *)s_slots[SLOT_##consumer].pIn->para; \
    __in->producer##_params = (void*)&__out->consumer##_params
```

然后使用组合回调模式（多个通道在一个回调函数中，因为 MODULE_SKELETON 的 DoWork 每个模块只调用一个 InputCallback）：

```c
void AppHmi_InputCallback(void) {
    /* 通道1: DrvKey → AppHmi */
    INPUT_GET_SLOT(Key, Hmi);
    /* 通道2: AppPower → AppHmi */
    AppPower_Output *__pwr = (AppPower_Output *)s_slots[SLOT_POWER].pOut->para;
    AppHmi_Input *__hmi = (AppHmi_Input *)s_slots[SLOT_HMI].pIn->para;
    __hmi->AppPower_params = (void*)&__pwr->Hmi_params;
}
```

### P1 — 应该修复：ST_NEW 处理

**当前**：InputCallback 通过别名指针写入生产者的 LINK 状态：
```c
hmi_in->key->status |= ST_NEW;  // hmi_in->key 与 key_out->to_hmi 指向同一地址！
```

**规则**：InputCallback 只做指针别名处理，不修改状态。状态管理规则：
- **生产者**：写入新数据后设置 `ST_OUT`（或 `ST_NEW`）
- **InputCallback**：仅做指针别名（`in->X_params = (void*)&out->Y_params`）
- **ProcessInput**：检查 `in->Producer_params->status & ST_OUT`，不清除（共享链接）
- 或：检查 `ST_NEW`，消费后清除（如果是每个消费者专用的链接）

### P2 — 需要考虑：路由函数清理

当前 `_route_key()` 清除生产者链接上的 `ST_NEW`。如果 ProcessInput 改为读取 `ST_OUT`，则路由函数不应清除生产者的状态位。

---

## 二、需要修改的文件

| # | 文件 | 修改 |
|---|------|------|
| 1 | `four_head/src/include/drv_key_io.h` | 重命名 OUTPUT 成员 |
| 2 | `four_head/src/include/app_power_io.h` | 重命名 OUTPUT + INPUT 成员 |
| 3 | `four_head/src/include/app_hmi_io.h` | 重命名 INPUT + OUTPUT 成员 |
| 4 | `four_head/src/drv/drv_key.c` | 更新成员引用 |
| 5 | `four_head/src/core/data_switcher.c` | 重命名成员 + 添加宏 + 修复 ST_NEW + 清理路由 |
| 6 | `four_head/src/app/app_hmi.c` | 如有必要更新 ProcessInput |
| 7 | `four_head/src/app/app_power.c` | 如有必要更新引用 |
| 8 | `four_head/src/app/app_comm_mgr.c` | 如有必要更新引用 |
| 9 | `four_head/src/core/data_switcher.h` | 可选：添加宏声明 |

---

## 三、验证

进行更改后，生成新的测试报告（`commCheck/check_report_v2.md`），涵盖：
1. **命名约定审计**：所有 OUTPUT 成员 → `{Consumer}_params`，所有 INPUT 成员 → `{Producer}_params`
2. **宏使用审计**：data_switcher.c 中的 InputCallback 使用 `INPUT_CALLBACK`/`INPUT_GET_SLOT`
3. **ST_NEW 完整性**：回调函数不修改生产者的状态位
4. **跨文件一致性**：所有引用已更新
5. **编译检查**：如果编译器可用

---

## 四、v2.3 核心概念速查

### 管道配对

```
Producer (输出方)                                Consumer (输入方)
─────────────────────────────                    ─────────────────────────────
MODULE_OUTPUT_PARAMS(Producer, Consumer)           MODULE_INPUT_PARAMS(Producer, Consumer)
  └── 字段布局: { field1, field2, ... }              └── 字段布局: 完全一致
MODULE_OUTPUT_LINK(Producer, Consumer)              MODULE_INPUT_LINK(Producer, Consumer)
  └── status + params[] 实例                         └── status + params[] 实例
MODULE_OUTPUT(Producer)                             MODULE_INPUT(Consumer)
  └── MODULE_OUTPUT_LINK {Consumer}_params (实例)    └── MODULE_INPUT_LINK* {Producer}_params (指针)
                                                          ↑ InputCallback 直穿赋值
```

### 实例 vs 指针

```c
// === Producer 的 io.h ===
typedef struct {
    MODULE_OUTPUT_LINK(Producer, Consumer)  {Consumer}_params;    // ← 实例 (以消费者命名)
} MODULE_OUTPUT(Producer);

// === Consumer 的 io.h ===
typedef struct {
    MODULE_INPUT_LINK(Producer, Consumer)* {Producer}_params;    // ← 指针 (以生产者命名)
} MODULE_INPUT(Consumer);
```

### 对称命名

```c
// in = consumer, out = producer
in->{Producer}_params = (void*)&out->{Consumer}_params;
// 例: in->DrvKey_params = (void*)&out->Hmi_params
```
