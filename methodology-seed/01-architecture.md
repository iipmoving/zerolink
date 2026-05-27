# 01 — AI 零耦合嵌入式架构 v2.1

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
APP↔APP: 只通过 __weak 回调通信，禁止直接 include 或调用
仅 DRV → HAL 合法
```

**约束机制**: `check_deps.py` 扫描所有 `#include` 语句，对照层白名单。违规 → 非零退出码 → pre-commit hook 阻断。

### 铁律二：__weak 回调是唯一跨模块通信机制 [L0: 链接器 + L1: check_weak_pairs.py]

```
发送方: __weak void Receiver_OnXxx(uint16_t param, void *data_ptr) {}  (空壳)
接收方:        void Receiver_OnXxx(uint16_t param, void *data_ptr) {}  (强符号)
发送时机: 直接调用 Receiver_OnXxx(param, data)
无队列，无注册，无消息 ID
```

**约束机制**: L0 — 强符号自动覆盖弱符号（链接器保证）。函数名不匹配则链接失败。L1 — `check_weak_pairs.py` 验证 `interface_map.h` 记录的配对是否在代码中存在、签名一致。

### 铁律三：独立声明，生成器管一致性 [L2: generate_structs.py + check_structs.py]

跨模块传递的数据结构，每模块独立声明。相同内存布局，不同命名，互不 include。

一致性由 `generate_structs.py` 从 `cfg/structs.json` 自动生成保证。`check_structs.py` 验证生成文件未被手动修改。AI 只需编辑 JSON，不手动维护结构体副本。

### 铁律四：AI 管理 __weak 配对 [L1: check_weak_pairs.py + L3: 文档]

谁发谁收、函数签名——全部记录在 `interface_map.h`，由 AI 维护。模块不知道谁在收它的调用。链接器自动接线。

**约束机制**: L1 — `check_weak_pairs.py` 检测孤儿 WEAK 和孤儿强符号。L3 — 新 AI 需理解 `interface_map.h` 格式（文档约定，不可自动化部分）。

### 铁律五：统一命名约定 [L3: 文档]

`{ModulePrefix}_On{Event}(uint16_t param, void *data_ptr)`
`_IN` = 模块接收, `_OUT` = 模块输出, 无后缀 = 内部私有

**约束机制**: L3 — 命名是约定，编译器不检查语义。违规不影响编译但会降低可读性。这是少数只能靠文档支撑的规则。

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

### 函数: `{PREFIX}_On{EVENT}`

| 部分 | 说明 | 示例 |
|------|------|------|
| PREFIX | 接收方模块前缀 | AppHmi, DrvDisplay, AppPower |
| EVENT | 事件名 | Key, Timer100ms, PowerCtrl |

### 类型命名

```
APP 层:  Hmi* (如 HmiDisplayCache_t)
DRV 层:  Drv* 或领域名 (如 DisplayFrame_t)
```

### 文件命名

```
小写 + 下划线: drv_key.c, app_hmi.h, msg_scheduler.c
模块前缀: 文件名体现所属层 (app_ / drv_ / hal_ / proto_)
```

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
python tools/check_deps.py            # 层依赖检查
python tools/check_weak_pairs.py      # __weak 配对检查
python tools/check_structs.py         # 结构体一致性检查
armcc -c ... → 0 error, 0 warning     # 编译验证
```
