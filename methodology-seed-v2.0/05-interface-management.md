# 05 — interface_map.h 与 AI 依赖管理

---

## 一、interface_map.h 是什么

`interface_map.h` 是 __weak 回调配对的**唯一真相源**（single source of truth）。它记录:

1. **结构体配对表**: 跨模块传递的数据类型，两端独立声明但布局相同
2. **__weak 通道注册表**: 每条通道的发送方(WEAK 空壳)、接收方(强符号)、函数签名

**关键规则**: `interface_map.h` 仅作文档参考，**任何 .c/.h 不得 include 它**。它是 AI 维护的知识库，不是编译依赖。

---

## 二、格式模板

```c
/* interface_map.h — __weak 回调配对文档 (v2.0)
 *
 * 本文件是 __weak 配对的唯一真相源。
 * 每对包含: 发送方(WEAK空壳) → 接收方(强符号实现)
 * AI 保证双方函数签名一致。
 * 任何 .c/.h 不得 include 本文件。
 */

#ifndef INTERFACE_MAP_H
#define INTERFACE_MAP_H

/* === 结构体配对表 === */

/* Pair S1: 显示缓存
 * APP:  HmiDisplayCache_t { char seg_chars[8]; uint8_t seg_blink[4]; ... }
 * DRV:  DisplayFrame_t    { char seg_chars[8]; uint8_t seg_blink[4]; ... }
 * 约束: sizeof() 相等, 字段 offsetof() 相等
 */

/* === __weak 通道注册表 === */

/* Pair A: key → state
 * 发送方: key_module.c    WEAK void HmiState_OnKey(uint16_t param, uint16_t unused, void *data_ptr) {}
 * 接收方: state_module.c  void HmiState_OnKey(uint16_t param, uint16_t unused, void *data_ptr)
 * 注: param = ((uint16_t)evt << 8) | key, 打包传递
 */

/* Pair B: state → display
 * 发送方: state_module.c   WEAK void Display_OnStateChange(uint8_t zone, const HmiHead_t *head, const HmiGlobalState_t *global) {}
 * 接收方: display_module.c void Display_OnStateChange(uint8_t zone, const HmiHead_t *head, const HmiGlobalState_t *global)
 */

#endif
```

---

## 三、AI 维护规则

### 新增通道
1. 在 `interface_map.h` 注册配对: 谁发、谁收、函数名、参数类型
2. 发送方: 添加 `__weak void Receiver_OnXxx(...) {}` 空壳
3. 接收方: 添加 `void Receiver_OnXxx(...) {}` 强实现
4. 两端签名必须与 `interface_map.h` 记录一致
5. 如需新结构体: 两端独立声明，`interface_map.h` 注册配对

### 修改通道
1. 改任一方函数签名 → AI 同步改配对
2. 改任一方结构体字段 → AI 同步改配对结构体
3. `interface_map.h` 更新

### 删除通道
1. 删除发送方 __weak 声明和所有调用点
2. 删除接收方强符号实现
3. `interface_map.h` 移除该条目

---

## 四、独立类型声明铁律

**跨模块传递的数据结构，每个模块独立声明自己的类型。相同内存布局，不同命名，互不 include。**

```c
/* === 发送方 (app_hmi.c) === */
typedef struct {
    char     seg_chars[8];
    uint8_t  seg_blink[4];
    uint8_t  seg_mode;
    uint8_t  leds_power;
    /* ... */
} HmiDisplayCache_t;

/* === 接收方 (drv_display.c) === */
typedef struct {
    char     seg_chars[8];
    uint8_t  seg_blink[4];
    uint8_t  seg_mode;
    uint8_t  leds_power;
    /* ... */
} DisplayFrame_t;
```

**约束**:
- 字段顺序、类型、数组大小必须完全一致
- AI 负责保证 `sizeof()` 和 `offsetof()` 相等
- 修改任一方时，AI 同步改配对
- 发送方不知道接收方的类型名，接收方不知道发送方的类型名

**为什么这样**: 共享类型头文件（各模块 include 同一个 .h）增加了编译期耦合。独立声明消除了模块间编译依赖。这对人很难，但对 AI 很容易——AI 可自动对比 sizeof/offsetof。

---

## 六、跨模块结构体生成规范 (v2.1)

> 本章是 v2.1 新增内容。此前独立类型声明由 AI 手动维护一致性，现改为生成式工具保证。

### 6.1 单一数据源

跨模块结构体不在各模块 `.h` 中手动声明，而是在 `cfg/structs.json` 中统一定义，由 `generate_structs.py` 生成各模块的独立声明。

```
人/AI 编辑 cfg/structs.json
        ↓
python tools/generate_structs.py
        ↓
生成各模块 types.h (带 AUTO-GENERATED 标记)
        ↓
python tools/check_structs.py  ← 验证一致性
```

详见 `07-struct-generation.md`。

### 6.2 命名约定: _OUT / _LINK 后缀

| 后缀 | 含义 | 示例 |
|------|------|------|
| `_OUT` | 本模块是**生产者**，对外输出 | `StateGlobal_OUT_t` |
| `_LINK` | 本模块是**消费者**，与 owner 双向维护 | `Power_AdcDef_LINK_t` |
| (无后缀) | 本模块**内部使用**，不跨模块 | `DisplayBuffer_t` |

- Owner struct: `{StructKey}_t` 或 `{StructKey}_OUT_t`
- Consumer struct: `{Module}_{StructKey}_LINK_t`
- `_LINK` = 联合管理，前后缀利于 Python 工具检索配对
- 前缀 `{Module}_` 标明所属模块，后缀 `_LINK` 标记跨模块联合维护

**_LINK 的替代路径**：

当 consumer 超过 2 个或 struct 字段频繁变更时，考虑废弃 `_LINK` 副本，改用数据交换机（`08-data-switcher.md`）。Switcher 持有所有模块的 IO 指针，运行时从生产者输出槽取数据填入消费者输入槽。模块不再需要独立声明与 owner 布局相同的 consumer struct。

**决策表**:

| 条件 | 建议 | 原因 |
|------|------|------|
| 1 个 consumer, 字段稳定 | `_LINK` 可接受 | 维护负担小 |
| ≥2 个 consumer | 数据交换机 | 改一个 Output_t → 改 N 个 _LINK 容易出错 |
| 字段频繁变更 | 数据交换机 | 只需改 Switcher 接线，不影响 consumer |
| consumer 需要多源汇聚 | 数据交换机 | Input_t 统一入口，Switcher 负责聚合 |
| ISR 延迟敏感 | `__weak` 直调 | 不走 Switcher，不受调度槽限制 |

`_LINK` 和 Switcher 的关系：Switcher 是更激进的那一档——连副本都不需要了。但 Switcher 引入了中间层调度开销（~1 帧延迟），不适合 ISR 路径。

**从 _LINK 迁移到 Switcher 的标志**: 发现自己在两个 `.h` 之间同步 struct 字段 → 立即考虑迁移。这就是 m4_ekf_observer 项目触发 v2.0 升级的原因。

### 6.3 与 interface_map.h 的分工

| 文档 | 管什么 | 维护方式 |
|------|--------|---------|
| `cfg/structs.json` | 结构体字段定义 + 消费关系 | 人/AI 手动编辑 |
| 生成 `types.h` | 每模块的结构体声明 | `generate_structs.py` 自动生成 |
| `interface_map.h` | __weak 函数配对 + 签名 | AI 手动维护 |

三者都不参与编译，都是被工具验证的对象。结构体变更改 JSON，通道变更改 `interface_map.h`，互不干扰。

### 6.4 AI-MANAGED 段标记规范

**跨模块结构体必须在 `.h` 文件中用明确的起止标记围起来。** 两个角色各有自己的段格式。

#### Owner 段 (生产者 .h)

```c
/* === INTERFACE STRUCTS (OWNER) ==================================
 * 本模块是以下结构体的 Owner (生产者).
 * @STRUCT 标记由 check_structs.py 解析验证.
 *
 * 消费者: {module_b} — 声明 {ModuleB}_{StructName}_LINK_t 副本
 * ================================================================ */

#pragma pack(4)

/* @STRUCT StructName  owner=this_module  suffix=OUT */
typedef struct {
    uint32_t field_a;              /* offset=0, size=4 */
    uint16_t field_b;              /* offset=4, size=2 */
} StructName;                      /* sizeof=N */

#pragma pack()

/* === END INTERFACE STRUCTS === */
```

#### Consumer 段 (消费者 .h)

```c
/* === AI-MANAGED INTERFACE STRUCTS (CONSUMER) ======================
 *
 *  双向维护区 — AI 从 owner 自动同步, 禁止手动编辑
 *
 *  消费者: this_module
 *  Owner:  owner_module
 *
 *  check_structs.py 自动验证 sizeof/offset 与 owner 一致.
 *  owner 字段变更后 AI 必须同步更新本段.
 *
 *  命名: {Module}_{OwnerStruct}_LINK_t  (_LINK = 联合管理)
 *
 * ==================================================================== */

#pragma pack(4)

/* @STRUCT Pair S{N} owner=owner_module  suffix=LINK  source=OwnerStruct */
typedef struct {
    /* 字段布局与 owner 完全一致 */
} Module_OwnerStruct_LINK_t;

#pragma pack()

/* === END AI-MANAGED INTERFACE STRUCTS === */
```

#### 铁律

| 规则 | 说明 |
|------|------|
| 起止标志必须存在 | 工具通过 `AI-MANAGED` / `INTERFACE STRUCTS` 关键字定位段边界 |
| 段内只放跨模块结构体 | 内部私有 struct 放段外 |
| `#pragma pack(4)` 包裹 | 保证 ARM 32-bit 对齐，sizeof/offsetof 一致 |
| `@STRUCT` 标签 | 每结构体一行，check_structs.py 解析 owner/consumer 配对 |
| `offset/sizeof` 注释 | 每个字段标注 `/* offset=N, size=N */`，人眼可验 |
| 禁止手动编辑 consumer 段 | AI 负责从 owner 同步，Python 工具阻断不一致 |
