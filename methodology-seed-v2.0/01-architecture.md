# 01 — AI 零耦合嵌入式架构 v2.4

> **最新版本**: v2.4 (2026-06, OUTPUT_LINK 指针化)
> **演进**: v1.0 MsgScheduler → v2.0 __weak 直调 → v2.2 PULL 范式 → v2.3 codeGen 驱动 → v2.4 OUTPUT_LINK
> **版本历史**: 参见 `archive/methodology-history/_INDEX.md`

---

## 零、设计哲学：约束系统，不是建议文档

### 0.1 定位

本方法论是一套**约束系统**，不是一套建议文档。

我们的角色不是"项目执行者"——我们是这套系统的**构建者**。系统构建者和项目执行者的区别：

| 执行者思维 | 构建者思维 |
|-----------|-----------|
| 我需要遵守规则 | **规则必须阻止违规** |
| 写文档提醒自己 | **违规在提交前被工具阻断** |
| 约定靠自觉 | **一致性由生成器保证** |
| 犯错后修复 | **错误状态无法通过验证** |

### 0.2 约束优先级

设计每条规则时，问的不是"文档有没有说明"，而是：

> **"如果将来一个不认识我们的 AI，不读任何文档，直接动手改代码——他会撞上什么？"**

按约束力从强到弱排列：

| 层级 | 机制 | 效果 | 示例 |
|------|------|------|------|
| **L0 物理阻断** | 编译器报错 | 绝对无法通过编译 | 类型不匹配 |
| **L1 提交阻断** | pre-commit tool，非零退出码 | 能编译但不能提交 | `check_deps.py` 检测到跨层 include |
| **L2 生成一致性** | 工具从单一数据源生成，手动改生成文件 → L1 阻断 | 正确结果由工具产生 | `generate_structs.py` + `check_structs.py` |
| **L3 文档** | 命名约定、注释规范 | 仅在 L0-L2 无法覆盖时使用 | `_IN`/`_OUT` 后缀约定 |

**L3 是最后手段，不是默认手段。** 一条规则如果只能做到 L3，就需要问：能不能升级到 L2 或 L1？

### 0.3 设计自检

每新增一条方法论规则，必须回答：

1. 这条规则的违反在哪个层级被阻断？
2. 如果阻断层级是 L3（纯文档），有没有可能变成 L2（工具检查）或 L1（pre-commit hook）？
3. 一个不读文档的 AI 可能怎么绕过它？绕过之后会撞上什么？

---

## 一、核心命题

传统嵌入式开发中，模块间依赖是人维护的。人管不住 → 加 include → 加耦合 → 改不动。

**本架构的答案**: 不是"告诉人不要加依赖"，而是让 **跨层 include 被工具阻断（L1）**，跨模块结构体不一致被生成器消除（L2），__weak 配对错误被工具检出（L1）。

`__weak` 回调直调：发送方定义 `__weak` 空壳函数，接收方提供强符号同名函数。链接器自动接线。无消息队列，无注册，无 ID。链接器本身提供 L0 保障——强符号覆盖弱符号，函数名不存在则链接失败。

---

## 二、五层架构

```
┌──────────────────────────────────────────┐
│  APP  业务逻辑层                           │
│  通信: __weak 回调直调 (Receiver_OnXxx)    │
│  禁止: include drv/ hal/ 其他app/          │
├──────────────────────────────────────────┤
│  API  装配器半层 (可选)                     │
│  职责: 抽象值→硬件值机械转换                 │
│  通信: 回调指针传 DRV                       │
├──────────────────────────────────────────┤
│  DRV  设备驱动层                            │
│  通信: 只 include hal/ + 定义 __weak 给APP │
│  禁止: include app/                        │
├──────────────────────────────────────────┤
│  PROTO 协议层 (可选)                        │
│  纯函数库: 编解码，无状态，无副作用           │
│  禁止: include app/ drv/ hal/              │
├──────────────────────────────────────────┤
│  HAL  硬件抽象层                            │
│  零依赖: 不知道上层存在                      │
│  禁止: include app/ drv/ proto/            │
└──────────────────────────────────────────┘
```

**唯一合法的跨层调用链**: APP → __weak 直调 → DRV → HAL

---

## 三、六大铁律（均标注约束层级）

### 铁律一：层间隔离是绝对的 [L1: check_deps.py + pre-commit]

```
APP:   不得 include drv/ 或 hal/
DRV:   不得 include app/
HAL:   零依赖，不知上层存在
APP↔APP: 只通过 __weak 回调 (同步) 或 Msg_Post (异步) 通信，禁止直接 include 或调用
仅 DRV → HAL 合法
```

**约束机制**: `check_deps.py` 扫描所有 `#include` 语句，对照层白名单。违规 → 非零退出码 → pre-commit hook 阻断。

### 铁律二：三种通信机制互补 [L0: 链接器 + L1: check_weak_pairs.py]

**三种通信机制，按调用时机和数据类型选择，不互相替代。**

| 机制 | 适用场景 | 调用时机 | 中间层 |
|------|---------|---------|--------|
| `__weak` 直调 | 同时间片连续执行 | 同步，调用即执行 | 无（链接器接线） |
| `Msg_Post` 消息 | 跨时间片 / 跨进程 | 异步，队列缓冲 | msg_scheduler |
| 数据交换机 | 周期性结构化数据路由 | 调度槽内顺序执行 | __weak 链自动路由 + Switcher 顺序调用 |

```
/* 同步场景 — __weak 直调 */
发送方: __weak void Receiver_OnXxx(uint16_t param, void *data_ptr) {}  (空壳)
接收方:        void Receiver_OnXxx(uint16_t param, void *data_ptr) {}  (强符号)
调用:    Receiver_OnXxx(param, data)  — 立即执行，无队列，无注册

/* 异步场景 — Msg_Post */
发送方: Msg_Post(MSG_KEY_EVENT, param, &data)  — 写入环形队列
接收方: MsgScheduler_Register(MSG_KEY_EVENT, handler)  — 下个时间片消费

/* 结构化数据路由 — 数据交换机 (v2.2: Switcher 显式 PULL) */
发送方: ModuleA_DoWork() → ProcessInput 写 g_output.para (不置状态位)
中间层: Switcher 检查 producer 输出标志 → 显式调 consumer 回调
接收方: Consumer 回调 memcpy(g_input.para, ...) + 置 ST_NEW → ModuleB_DoWork() 消费
```

**选择规则（两层）**:

```
1. 这个操作必须在同一次调度槽内完成？
   → 是: __weak 直调 ← 调用即执行，无延迟
   → 否: 进入下一问

2. 传递的是事件通知还是结构化数据块？
   → 事件通知: Msg_Post        ← 写入队列，下个时间片消费
   → 结构化数据块: 数据交换机    ← __weak 链路由 + Switcher 顺序调用，Para_Grp_t 统一参数组
```

**约束机制**: L0 — 强符号自动覆盖弱符号（链接器保证），函数名不匹配则链接失败。L1 — `check_weak_pairs.py` 验证 `interface_map.h` 记录的配对是否在代码中存在、签名一致。数据交换机侧：`check_include.py` 阻断非 Switcher 文件的全路径 `_io.h` 引用。

### 铁律三：独立声明，生成器管一致性 [L2: generate_structs.py + check_structs.py]

跨模块传递的数据结构，每模块独立声明。相同内存布局，不同命名，互不 include。

一致性由 `generate_structs.py` 从 `cfg/structs.json` 自动生成保证。`check_structs.py` 验证生成文件未被手动修改。AI 只需编辑 JSON，不手动维护结构体副本。

#### 3.1 32 位对齐与 res[] 填充（通用规则）

**所有跨模块结构体必须 32 位对齐，sizeof 为 4 的倍数。** 不限于数据交换机——`Msg_t`、`__weak` 回调参数 struct、`structs.json` 定义的 struct 均适用。

```c
#pragma pack(4)

typedef struct {
    uint8_t  status;        // 状态字节
    uint8_t  res[3];        // 32位对齐填充 — sizeof 到此为 4
    uint16_t value;         // 2字节
    uint8_t  flag;          // 1字节
    uint8_t  res2[1];       // 补齐到 4 字节边界 — 总 sizeof = 8
} ModuleX_Data_t;           // sizeof 必须为 4 的倍数

#pragma pack()
```

**规则**:
- `#pragma pack(4)` 包裹所有跨模块 struct 定义
- 结构体成员总字节数向上取整到 4 的倍数，不足用 `uint8_t res[N]` 填充
- `check_structs.py` 会验证 owner/consumer 的 sizeof 一致，对齐不一致会被检出
- `_io.h` 中的 Input_t/Output_t 同样遵守此规则（status 字节后的 `res[3]` 即是对齐填充）

### 铁律四：AI 管理 __weak 配对 [L1: check_weak_pairs.py + L3: 文档]

谁发谁收、函数签名——全部记录在 `interface_map.h`，由 AI 维护。模块不知道谁在收它的调用。链接器自动接线。

**约束机制**: L1 — `check_weak_pairs.py` 检测孤儿 WEAK 和孤儿强符号。L3 — 新 AI 需理解 `interface_map.h` 格式（文档约定，不可自动化部分）。

### 铁律五：统一命名约定 [L3: 文档]

所有顶层标识符（类型、函数、变量、常量、文件）必须带模块前缀。详见 §四 命名约定。

`{ModulePrefix}_{Name}_t` / `{ModulePrefix}_{Action}()` / `g_{name}` / `MODULE_{NAME}`

**约束机制**: L3 — 命名是约定，编译器不检查语义。违规不影响编译但会降低可读性。可通过 `check_weak_pairs.py` 间接验证 __weak 函数名配对。

### 铁律六：每个模块可独立编译测试 [L2: 生成器 + L1: check_deps.py]

无运行时基础设施依赖。任何 .c 文件可单独编译并链接测试框架。

### 铁律七：头文件私有化 — 模块 .h 只允许自身 .c 引用 [L0: 编译器]

**DRV/APP/PROTO 层的每个 .h 文件，include guard 的 `#define` 必须注释掉。**

```c
#ifndef MODULE_NAME_H
//#define MODULE_NAME_H   // ← 故意注释，禁用 include guard
...
#endif /* MODULE_NAME_H */
```

**效果**: 同一编译单元（一个 .c 文件）内，该头文件只能被包含一次。第二次 `#include` → `#ifndef` 仍为真 → 结构体/枚举/常量重定义 → **编译器直接报错**。

**为什么这是 L0**: 不依赖工具脚本，不依赖文档约定，不依赖人工审查。编译器直接阻断——AI 或开发者想伸手过界，编译就过不去。

**哪些层适用**:
| 层 | 头文件私有化 | 原因 |
|----|------------|------|
| APP | 是 | 只允许自身 .c + main.c 引用 |
| DRV | 是 | 只允许自身 .c + main.c 引用 |
| PROTO | 是 | 只允许自身 .c 引用（通过 __weak 抽象对接） |
| HAL | **否** | DRV 层需要引用 HAL 头文件 |

**为什么有效**:
- 函数声明可以重复（编译器允许），但 `typedef`/`struct`/`enum`/`#define` 常量不行
- 注释掉 `#define` 后 include guard 失效，每次 `#include` 都会重新展开全部内容
- 自身 .c 和 main.c 各包含一次——它们在不同 TU 中，互不影响
- 但如果某个头文件（如 `cfg/x.h`）也包含了你的模块头文件 → 同一个 .c 内出现两次 → 爆炸

**与 check_deps.py 的互补**:
| 层级 | 机制 | 阻断什么 |
|------|------|---------|
| **L0** | 注释 define（编译器） | 同一 TU 内重复包含 → 编译报错 |
| **L1** | check_deps.py（pre-commit） | 跨层 include（如 APP include DRV）→ 提交阻断 |

两者配合：L1 阻止直接跨层引用，L0 阻止任何形式的二次包含（包括未来新模块不小心引入的传递包含）。

**注意事项**:
- 如果模块 .h 和 cfg 数据 .h 之间存在循环引用（如 `app_hmi.h` ↔ `cfg/hmi_data.h`），必须先拆解循环才能应用此规则
- 拆解方法：移除 .h 中的 `#include "cfg/xxx.h"`，改为在 .c 中按顺序包含两个头文件

---

## 四、命名约定

**核心原则**: 所有顶层标识符（类型、函数、变量、常量）必须带模块前缀。模块前缀 = 模块名转 PascalCase（如 `app_power` → `AppPower`，`drv_key` → `DrvKey`）。全局符号通过前缀一眼识别归属，避免命名冲突。

### 4.1 结构体/类型: `{ModulePrefix}_{DescriptiveName}_t`

```
格式: {ModulePrefix}_{DescriptiveName}_t

示例:
  AppPower_Status_t          — app_power 模块的状态结构体
  DrvDisplay_Frame_t         — drv_display 模块的帧结构体
  IhElecParams_CycleDataDef  — ih_elec_params 模块的周期数据定义
  Adc_Output_t               — app_adc 模块的输出槽（数据交换机）
  Power_Input_t              — app_power 模块的输入槽（数据交换机）

后缀:
  _t        — 通用结构体类型
  _IN_t     — 数据交换机输入槽（本模块消费）
  _OUT_t    — 数据交换机输出槽（本模块产出）
  _LINK_t   — [v1.0 遗留] consumer 副本，v2.1 __weak 链 + Para_Grp_t 已消除此模式
```

### 4.2 函数: `{ModulePrefix}_{Action}(...)`

```
格式: {ModulePrefix}_{Action}(参数)

示例:
  AppPower_DoWork(void)                — 模块主入口（数据交换机）
  AppPower_GetIO(Para_Grp_t **, Para_Grp_t **, void (**)(void)) — 暴露输入/输出槽 + DoWork 函数指针 (MODULE_EXPORT 生成)
  DrvKey_Init(void)                    — 初始化
  DrvDisplay_Commit(Frame_t*)          — 提交显示帧
  HalGpio_SetPin(uint8_t pin)          — HAL 层设引脚

__weak 回调（§铁律二）使用 _On{Event} 子格式:
  AppPower_OnAdcData(void *pData)      — 接收 ADC 数据
  DrvKey_OnKeyEvent(uint16_t param)    — 接收按键事件
```

### 4.3 变量: `g_{descriptive_name}` 或 `s_{descriptive_name}`

```
格式: {scope_prefix}_{snake_case_name}

  g_  — 文件级全局变量（file-scope static 或 extern）
  s_  — 函数级静态变量（static local）

示例:
  static AppPower_Input_t  g_in;       — 模块输入槽（全局）
  static AppPower_Output_t g_out;      — 模块输出槽（全局）
  static uint8_t s_initialized = 0;    — 懒惰初始化标志（函数内 static）

禁止: 无前缀的全局变量名（如 power, status, temp）——无法识别归属
```

### 4.4 常量/宏: `MODULE_NAME_{DESCRIPTIVE}`

```
格式: {UPPER_MODULE_NAME}_{UPPER_DESCRIPTIVE}

示例:
  APP_POWER_MAX_PWM          — app_power 模块的最大 PWM 值
  DRV_KEY_DEBOUNCE_MS        — drv_key 模块的去抖时间
  HAL_GPIO_PIN_COUNT         — hal_gpio 模块的引脚数
  MSG_SLOT_DEPTH             — msg_scheduler 模块的队列深度

禁止: 无模块前缀的裸常量（如 MAX_PWM, DEBOUNCE_MS）——无法识别归属
```

### 4.5 文件命名

```
小写 + 下划线: drv_key.c, app_hmi.h, msg_scheduler.c
模块前缀: 文件名体现所属层 (app_ / drv_ / hal_ / proto_ / core_)
_io.h 后缀: 数据交换机公开接口 (app_power_io.h, app_adc_io.h)
```

### 4.6 命名速查

| 元素 | 格式 | 示例 |
|------|------|------|
| 结构体 | `{Module}_{Name}_t` | `AppPower_Status_t` |
| 函数 | `{Module}_{Action}()` | `AppPower_DoWork()` |
| __weak 回调 | `{Module}_On{Event}()` | `AppPower_OnAdcData()` |
| 全局变量 | `g_{name}` | `g_in`, `g_power_cache` |
| 静态变量 | `s_{name}` | `s_initialized` |
| 常量/宏 | `MODULE_{NAME}` | `APP_POWER_MAX_PWM` |
| 文件 | `layer_module.c/.h` | `app_power.c`, `app_power_io.h` |

---

## 五、适用条件

- AI 参与编程（人独立维护散落的 __weak 配对不可持续）
- 单可执行文件（所有模块链接在一起，__weak 依赖链接器）
- 裸机/RTOS 嵌入式 C 项目
- 模块间通信量不大（__weak 直调是同步的）
- 所有调用在主循环中，ISR 只设标志

### 不适用

- 跨进程/跨核通信
- 需要消息持久化/延迟投递
- ISR 中直接调用耗时业务逻辑
- 动态加载插件（__weak 编译时解析）

---

## 六、AI 工作流

### 新增模块
1. 确定模块所属层级
2. 创建 module.h + module.c
3. .h 只声明公共接口和本模块类型
4. .h 的 include guard: `#ifndef MODULE_H` / `//#define MODULE_H` (APP/DRV/PROTO 层强制注释)
5. .c 顶部声明依赖的 __weak 空壳 + 提供本模块强符号
6. 禁止 include 不在本层白名单内的文件
7. `check_deps.py` → armcc 编译 → 两步通过才提交

### 新增跨模块通道
1. 确定发送方和接收方
2. 发送方 .c: 添加 `__weak void Receiver_OnXxx(uint16_t, void*) {}` 空壳
3. 发送方 .c: 调用 `Receiver_OnXxx(param, data)`
4. 接收方 .c: 强符号实现同名函数
5. 如需新结构体: 编辑 `cfg/structs.json` → 运行 `generate_structs.py`
6. 如需传已有结构体: 确认 consumer 已在 `structs.json` 注册
7. 更新 `interface_map.h`
8. `check_deps.py` → `check_weak_pairs.py` → `check_structs.py` → armcc 编译

### 每次编码后（强制）
```bash
python ../.claude/tools/check_deps.py            # 层依赖检查
python ../.claude/tools/check_weak_pairs.py      # __weak 配对检查
python ../.claude/tools/check_structs.py         # 结构体一致性检查
armcc -c ... → 0 error, 0 warning     # 编译验证
```

---

## 七、模块三段式范式

### 7.1 总览

所有模块（无论大小）统一为三段式结构：**输入 → 计算 → 输出**。

模块入口有两种形式，取决于通信机制：

```
/* 形式 A: __weak / Msg_Post 模块 (铁律二传统模式) */
Module_Run()                    ← 主循环/调度槽直接调用
  ├─ [构造] 首次调用懒惰初始化    ← static initialized 自检，不需外部 Init
  │
  ├─ [输入] 回调/消息收参        ← 所有输入顶部集中，禁止中途插回调
  │    ├─ __weak 强符号函数      ← 同时间片同步数据
  │    └─ Msg_Post 消费         ← 跨时间片异步数据
  │
  ├─ [计算] 核心逻辑              ← 纯计算，不调输入/输出通道
  │    └─ 调子模块 __weak 直调   ← 子模块也不 include .h
  │
  └─ [输出] 回调/消息丢结果       ← 所有输出底部集中
       ├─ Consumer_OnResult()    ← 同步回调给同时间片下游
       └─ Msg_Post(...)         ← 异步给跨时间片下游


/* 形式 B: 数据交换机模块 (铁律二 Switcher 模式, 详见 08-data-switcher.md) */
Module_DoWork()                 ← Switcher 每周期调用
  ├─ [构造] 自检 g_in.status bit0=0 → _Constructor() → 置 bit0
  │
  ├─ [输入] 消费 g_in 字段        ← consumer STRONG 回调已 memcpy + 置 ST_NEW
  │    检查 g_in.status & ST_NEW ← 有新输入才继续, 否则 return
  │    消费 g_in 字段             ← 读取/搬运后 g_in.status &= ~ST_NEW
  │
  ├─ [计算] 核心逻辑              ← 纯计算, 不调输入/输出通道
  │
  └─ [输出] 更新 g_out 字段       ← 写 g_out.para (不置 ST_OUT)
       Switcher 显式路由 consumer 回调


/* 形式 C: std_module.h 宏骨架 (v2.2 推荐, 详见 09-std-module.md) */
MODULE_SKELETON()               ← 展开 g_input/g_output/Constructor/DoWork
  │
  ├─ Init()                     ← 初始化绑定 g_input.para / g_output.para
  │
  ├─ ProcessInput()             ← 每帧被 DoWork 调用
  │    ├─ [输入] 检查 g_input.info.status & ST_NEW → 消费 → &= ~ST_NEW
  │    ├─ [计算] 纯逻辑
  │    └─ [输出] 写 g_output.para (不置状态位, Switcher 负责路由)
  │
  └─ MODULE_EXPORT({Module})    ← GetIO (不再生成 __weak OnOutput)

  Consumer 回调: {Module}_On{Producer}Data(Para_Grp_t *pOut)
     ← Switcher 在 Producer DoWork 后显式调用
     ← memcpy(g_input.para, pOut->para, ...) + g_input.info.status |= ST_NEW

  @OUTPUT_CALLBACK 例外:
     Producer 写 g_output.para + 置 info.route → 立即调 Consumer DoWork (绕开 Switcher)
     Consumer ProcessInput 内 switch(info.route) 选择执行路径
     ← 适用: 蜂鸣器实时反馈等。须用户确认 + 白名单注册
```

**关键区别**:

| | 形式 A (__weak) | 形式 B (Switcher v2.0) | 形式 C (std_module.h v2.2) |
|---|---|---|---|
| 入口函数 | `Module_Run()` | `Module_DoWork()` | `MODULE_SKELETON()` 展开 `DoWork()` |
| 输入来源 | __weak 强符号 | `g_in` 字段 (Switcher copy) | Switcher 显式调 consumer 回调 → memcpy 到 `g_input.para` |
| 输出去向 | __weak 回调 | `g_out` 字段 (Switcher copy) | Switcher 检查 producer 标志 → 调 consumer 回调 (PULL) |
| 数据搬运 | 无 (直接调) | Switcher 做 copy | consumer 自己做 `memcpy` |
| 参数槽 | 无 | 自定义 Input_t/Output_t | 统一 `Para_Grp_t { info + void* para }` |
| 构造机制 | `static uint8_t _init` | `g_in.status & 0x01` | `g_init_done` + `Constructor()` |
| 头文件 | `module.h` (`//#define`) | `_io.h` (`#define`) + 可选 `module.h` | `std_module.h` + `_io.h` (`#define`) 可选 |
| 推荐度 | 遗留 | 可用 | **v2.1 推荐** |

### 7.2 构造：懒惰初始化

三种形式的构造机制不同但等价：

**形式 A (__weak 模块)**: `static` 局部变量自检。

```c
void Module_Run(void)
{
    static uint8_t initialized = 0;
    if (!initialized) {
        initialized = 1;
        /* 初始化自己的状态变量 */
    }
    /* ... 三段式主体 ... */
}
```

**形式 B (Switcher v2.0)**: `g_in.status bit0` 自检。

```c
void Module_DoWork(void)
{
    if (!(g_in.status & 0x01)) {
        _Constructor();
        g_in.status |= 0x01;
    }
    /* ... 三段式主体 ... */
}
```

**形式 C (std_module.h v2.1)**: `MODULE_SKELETON()` 展开的 `Constructor()` 自动处理 — `memset` + 调 `Init()` + 置 `ST_INIT`。模块只需实现 `Init()` 绑定 `g_input.para` / `g_output.para`。

```c
MODULE_SKELETON();    /* Constructor 已包含 */

static void Init(void)
{
    memset(&s_in,  0, sizeof(s_in));
    memset(&s_out, 0, sizeof(s_out));
    g_input.para  = &s_in;
    g_output.para = &s_out;
}
```

**约束**: `main()` 不调用 `Module_Init()`。Switcher_Init() 只做 GetIO + Switcher_Register(pDoWork)，不知道模块内部构造逻辑。构造逻辑是模块私有的（DoWork 首次进入自检）。

### 7.3 输入段：提前规划，禁止中途回调

所有外部数据的入口集中在模块顶部。不允许在计算逻辑中间插入新的 `__weak` 回调声明或 `Msg_Post` 消费。

**规划原则**: 新模块时先问——"这个模块需要哪些外部数据？"→ 一次性声明所有输入回调 → 再写计算逻辑。

**回调插入分级**:

| 类型 | 位置 | 是否需要确认 |
|------|------|-------------|
| 输入回调（顶部） | 输入段 | 否 — 标准做法 |
| 输出回调（底部） | 输出段 | 否 — 标准做法 |
| 算法子集调用（__weak） | 计算段 | 否 — 必须的依赖，不 include .h |
| **即时回调（计算段中途）** | 计算段 | **是 — 必须用户确认** |

**即时回调管控**: 计算段中间发现需要一个外部数据 → **先停下来问**，不是先加上再说。如果规划到位，输入段已经覆盖所有需要的外部参数，很少需要中途插入。

```c
void AppPower_Run(void)
{
    /* ====== 输入段 ====== */
    /* __weak 被覆盖 → 非空调用, 未覆盖 → 空壳跳过 */
    AppPower_OnTargetPower(POWER_DEFAULT, NULL);
    /* Msg_Post 异步消息 — 前一帧缓存的功率指令 */
    /* ... */
    
    /* ====== 计算段 ====== */
    uint16_t power = CalculateRamp(s_target, s_current);
    
    /* ====== 输出段 ====== */
    DrvPower_OnSet(power, NULL);    /* 同步 → 同时间片生效 */
    Msg_Post(MSG_POWER_DONE, 0, NULL);  /* 异步 → 下时间片通知 */
}
```

### 7.4 主模块调纯算法集：不 include .h

大模块调用内部算法集时，不 `#include "algo.h"`。在自身 .c 顶部声明 `__weak` 空壳，链接时由算法集的强符号覆盖。

```c
/* === app_power.c (主模块) === */
/* 声明 __weak — 替代 #include "algo_ramp.h" */
__weak uint16_t AlgoRamp_Calculate(uint16_t target, uint16_t current, void *cfg)
{ return target; }  /* 空壳：无算法集时直通 */

void AppPower_Run(void)
{
    /* ... 输入段 ... */
    uint16_t result = AlgoRamp_Calculate(s_target, s_current, &s_ramp_cfg);
    /* ... 输出段 ... */
}

/* === algo_ramp.c (纯算法集) === */
/* 强符号覆盖 __weak 空壳 */
uint16_t AlgoRamp_Calculate(uint16_t target, uint16_t current, void *cfg)
{
    RampCfg_t *c = (RampCfg_t *)cfg;
    /* 纯计算 — 不访问全局变量，不调 I/O */
    return (target > current) 
        ? min(current + c->step_up, target)
        : max(current - c->step_down, target);
}
```

**效果**: 大模块 .c 不 include 算法集 .h。`interface_map.h` 记录配对的弱/强关系，`check_weak_pairs.py` 验证签名一致。

### 7.5 模块大小与范式适用

```
std_module.h 宏骨架模块 (推荐, ~50-300行)
  ├─ MODULE_SKELETON()      ← 展开 Constructor + DoWork (形式 C)
  ├─ Init()                 ← 绑定 g_input.para / g_output.para
  ├─ ProcessInput()         ← 三段式: 输入→计算→输出
  ├─ MODULE_EXPORT({Name})  ← GetIO
  └─ {Consumer}_On{Producer}Data()  ← 每个数据源一个 consumer 回调 (Switcher PULL 调用)

主进程大模块 (APP层, ~200-500行, 形式 A 遗留)
  ├─ Module_Run()           ← 三段式入口: 输入→计算→输出
  ├─ Module_OnXxx() 强符号  ← 输入回调 (被外部 __weak 调)
  └─ __weak Algo_Run()      ← 调子模块的空壳声明

纯算法集 (无层归属, ~50-150行)
  ├─ Algo_Run(param)        ← 强符号覆盖上层的 __weak
  └─ 纯函数: 不调 I/O, 不访问全局, 不 include 业务层
```

三者结构相同（输入→计算→输出），区别在输入来源/输出去向。**新模块优先使用形式 C。**

### 7.6 通信选择速查

| 场景 | 机制 | 示例 |
|------|------|------|
| 同时间片连续调用（上下级） | `__weak` 直调 | 主模块 → 算法集 / DRV → HAL |
| 同时间片平行模块 | `__weak` 直调 | APP 显示 → 装配器 |
| 跨时间片异步通知 | `Msg_Post` | 按键事件 → 业务 / 状态变更广播 |
| 跨时间片延迟消费 | `Msg_Post` | 功率下发 → MODBUS 下一帧发送 |
| 周期性结构化数据路由 | 数据交换机 | ADC输出→Power输入 / 状态广播 |

**原则**: 同步走 `__weak`，异步走 `Msg_Post`，结构化数据路由走数据交换机。三者不互相替代，组合使用。
