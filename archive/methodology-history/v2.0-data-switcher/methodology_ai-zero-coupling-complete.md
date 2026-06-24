---
name: ai-zero-coupling-methodology
description: AI零耦合嵌入式架构方法论v2.0——可复制的标准工作流，__weak回调直调架构，适用于任何C嵌入式项目
metadata:
  node_type: memory
  type: reference
  originSessionId: 7eb949aa-9578-417f-ab12-d6428ae59747
---

# AI 零耦合嵌入式架构方法论 v2.0

> v1.0 (2026-05-26 上午): MsgScheduler 消息总线
> v2.0 (2026-05-26 下午): __weak 回调直调 —— 彻底消除消息调度器
>
> 本项目（四头电磁炉）是此方法论的第一个验证案例。
> 方法论本身是产出，项目只是载体。

---

## 一、核心命题

**传统嵌入式开发中，模块间依赖是人维护的。人管不住 → 加 include → 加耦合 → 改不动。**

**AI 时代的新可能：AI 天生有一一配对能力。把模块间关系从"代码引用"变成"AI 维护的映射表 + __weak 约定"，编译期零耦合成为可能。**

v2.0 更进一步：连消息调度器这个"唯一桥梁"都不要了。发送方直接调 `Receiver_OnXxx(param, data)`，链接器接管一切。

---

## 二、五层架构 (v2.0)

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
│  禁止: include core/ app/ drv/ proto/      │
└──────────────────────────────────────────┘
```

**唯一合法的跨层调用链**: APP → __weak 直调 → DRV → HAL

**CORE 层已移除** —— msg_scheduler.c/h 废弃，不再需要。

---

## 三、六大铁律 (v2.0)

### 铁律一：层间隔离是绝对的（0/1，不能有口子）

```
APP 层: 不得 #include 任何 drv/ 或 hal/ 头文件
DRV 层: 不得 #include 任何 app/ 头文件
HAL 层: 零依赖，不知上层存在
APP↔APP: 只通过 __weak 回调通信，禁止直接 include 或调用
仅 DRV → HAL 合法
```

### 铁律二：__weak 回调是唯一跨模块机制

```
发送方: 定义 __weak void Receiver_OnXxx(uint16_t param, void *data_ptr) {} (空壳)
接收方: 定义        void Receiver_OnXxx(uint16_t param, void *data_ptr) {} (强符号)
发送时机: 直接调用 Receiver_OnXxx(param, data) —— 无队列，无注册，无ID
```

### 铁律三：独立声明，AI 管一致性

跨模块传递的数据结构，每模块独立声明自己的类型。相同内存布局，不同命名，互不 include。一致性由 AI 保证，记录在 `interface_map.h`。

### 铁律四：AI 管理 __weak 配对

谁发谁收、函数签名——全部记录在 `interface_map.h`，由 AI 维护。模块不知道（也不需要知道）谁在发、谁在收。链接器自动接线。

### 铁律五：统一命名约定，保证 AI 可自动 regex

`{ModulePrefix}_On{Event}(uint16_t param, void *data_ptr)`

### 铁律六：每个模块可独立编译测试

无 MsgScheduler 依赖，无队列，无注册。任何 .c 文件可以单独用 armcc 编译并链接测试框架。

---

## 四、核心文件

### 4.1 `core/interface_map.h` — AI 维护的映射表（文档，禁止 include）

两个注册表:

**结构体配对表**: 记录跨模块传递的独立声明结构体
**__weak 通道注册表**: 每条通道的发送方(__weak)/接收方(强符号)/数据类型

### 4.2 模块约定函数

每个模块:
- 对外暴露: `{Prefix}_On{Event}(uint16_t param, void *data_ptr)` 强符号函数
- 对外依赖: 在 .c 文件头部声明 __weak 空壳, 然后直接调用

### 4.3 自动化工具

- `tools/check_deps.py` — 层依赖扫描器
- `tools/check_msgs.py` — (待更新为 check_weak_pairs.py)

---

## 五、命名约定（AI 自动 regex 的基础）

### 5.1 函数命名: `{PREFIX}_On{EVENT}`

| 部分 | 说明 | 示例 |
|------|------|------|
| PREFIX | 接收方模块前缀 | AppHmi, DrvDisplay, AppPower |
| EVENT | 事件名 | Key, Timer100ms, PowerCtrl, RegData |

### 5.2 模块前缀注册表

```
APP 层:  AppHmi, AppCooking, AppPower, AppProtect, AppCommMgr
DRV 层:  DrvDisplay, DrvKey, DrvBuzzer, DrvCommMgr
入口:    main.c (无前缀)
```

### 5.3 类型命名

```
APP 层:  Hmi* (如 HmiDisplayCache_t)
DRV 层:  Drv* 或原名 (如 DisplayFrame_t)
CORE:    无 (Msg_t 已随 MsgScheduler 废弃)
```

---

## 六、AI 工作流

### 6.1 项目初始化

```
1. 建立五层目录结构 (app/ drv/ hal/ proto/)
2. 创建 interface_map.h (空映射表)
3. 创建 check_deps.py (层依赖检查)
4. armcc 编译通过 (0 error, 0 warning)
```

### 6.2 新增模块

```
1. 确定模块所属层级 (HAL/DRV/PROTO/APP)
2. 创建 module.h + module.c
3. .h 只声明公共接口和本模块类型
4. .c 顶部: 声明 __weak 空壳给需要通知的其他模块, 直接调用
5. 禁止 include 任何不在本层白名单内的头文件
6. check_deps.py → armcc 编译
7. 两步全部通过才可提交
```

### 6.3 新增跨模块通道

```
1. 确定发送方和接收方
2. 发送方 .c: 添加 __weak void Receiver_OnXxx(uint16_t, void*) {} 空壳
3. 发送方 .c: 在需要时调用 Receiver_OnXxx(param, data)
4. 接收方 .c: 添加 void Receiver_OnXxx(uint16_t, void*) { ... } 强实现
5. 如需传数据: 两端各独立声明结构体，不同名同布局
6. 更新 interface_map.h (结构体配对 + 通道条目)
7. check_deps.py → armcc 编译
```

### 6.4 每次编码后（强制）

```bash
python tools/check_deps.py Claude    # 层依赖检查
armcc -c ... → 0 error, 0 warning    # 编译验证
```

---

## 七、自动化工具

### 7.1 `check_deps.py` — 层依赖扫描器

检查 `#include` 语句是否符合层规则:
- `app/*.c` 不得包含 `drv/` 或 `hal/` 或 `core/msg_scheduler`
- `drv/*.c` 不得包含 `app/` 或 `core/msg_scheduler`
- `hal/*.c` 不得包含 `core/` `app/` `drv/` `proto/`

### 7.2 `check_weak_pairs.py` (建议新增)

检查 __weak 配对:
- interface_map.h 中每条通道: 发送方是否有 __weak 声明
- interface_map.h 中每条通道: 接收方是否有强符号实现
- 函数签名一致性
- 孤儿 __weak / 孤儿强符号检测

---

## 八、适用条件

### 必须满足

1. **AI 参与编程** — 人独立维护散落的 __weak 配对是不可持续的
2. **单可执行文件** — 所有模块链接在一起 (__weak 依赖链接器)
3. **编译期可验证** — 需要可运行的检查脚本

### 适用项目类型

- 嵌入式 C 项目（裸机/RTOS）
- 多模块、有明确分层需求
- 团队规模 ≥ 1 人 + AI
- 对可维护性/可测试性有高要求

---

## 九、v1.0 → v2.0 演进总结

| 方面 | v1.0 | v2.0 |
|------|------|------|
| 消息机制 | MsgScheduler (队列+注册) | __weak 回调 (直调) |
| 消息 ID | 需要, 全局唯一 | 不需要 |
| include 依赖 | `core/msg_scheduler.h` | 无 |
| 运行时代码 | Msg_Post, Register, Run1ms | 无 |
| 多接收方 | 多次 Msg_Post | 多次函数调用 |
| 自收(self-post) | Msg_Post → 队列 → handler | 静态函数直接调 |
| 独立编译 | 需 msg_scheduler.o | 零外部依赖 |
| 运行时内存 | 队列+消息体 | 0 |
| 异步性 | 有 (队列解耦) | 无 (同步直调) |

---

*版本: 2.0 — 2026-05-26 确立于四头电磁炉项目*
*维护: AI（技术负责人角色）*
*核心配套: [[weak-callback-zero-dependency]] [[independent-type-declarations]] [[ai-managed-dependency-injection]]*
*文件清单: [[MANIFEST]]*
