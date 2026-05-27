---
name: ai-managed-dependency-injection
description: AI管理__weak回调配对替代代码级依赖注入——interface_map.h是唯一真相，消除所有编译期耦合
metadata:
  node_type: memory
  type: feedback
  originSessionId: 7eb949aa-9578-417f-ab12-d6428ae59747
---

## 铁律四：AI 管理 __weak 配对，消除依赖注入的代码耦合

**跨模块通信通过 __weak 回调直调实现。谁发谁收、函数签名——这些都记录在 `core/interface_map.h` 中，由 AI 维护，不通过代码引入。**

**Why:** 用户 2026-05-26 明确提出。传统架构中依赖注入需要模块引用服务注册表或共享接口，引入编译期耦合。本项目用 AI 替代依赖注入框架——通信关系写在 `interface_map.h`（仅作文档，禁止 include），AI 保证代码层面的发送方 __weak 声明和接收方强符号签名一致。耦合只存在于 AI 的知识中，不存在于 .c/.h 的 include 关系中。

**v2.0 演进 (2026-05-26 下午):** 连 MsgScheduler 也移除了。现在连"注册"动作都不需要——发送方定义 __weak 空壳 + 接收方定义强符号同名函数，链接器自动接线。

**对比:**
```
v1.0:   Sender → Msg_Post(MSG_X, data) → Scheduler → Register(MSG_X, handler) → Receiver
        ↑ 三方都依赖 msg_scheduler.h, 需要全局唯一 MSG ID

v2.0:   Sender → Receiver_OnXxx(param, data) → 直接执行
        ↑ 零中间层, 零依赖, 零注册, 零 ID

传统:   Sender → include ServiceRegistry → 查找 Receiver → 调用
        ↑ 引入了对 ServiceRegistry 的编译依赖
```

**How to apply:**
1. 新增跨模块通道时，先在 `interface_map.h` 记录配对: 谁发(__weak)、谁收(强符号)、函数名、数据类型
2. 发送方: 定义 `__weak void Receiver_OnXxx(uint16_t param, void *data_ptr) { }` 空壳
3. 发送方: 在需要时直接调用 `Receiver_OnXxx(param, data)` —— 不经过任何中间层
4. 接收方: 定义 `void Receiver_OnXxx(uint16_t param, void *data_ptr) { ... }` 强实现
5. 发送方不知道（也不需要知道）谁在收它的调用。链接器自动接线。
6. `interface_map.h` 是 __weak 配对的**唯一真相源**（single source of truth），禁止任何 .c/.h include 它
7. 所有跨模块函数使用统一签名: `void {Prefix}_On{Event}(uint16_t param, void *data_ptr)`

**AI 的责任:**
- 保证发送方的 __weak 声明和接收方的强符号函数签名完全一致 (参数类型/顺序/返回值)
- 保证 `data_ptr` 指向的结构体在两端独立声明且布局一致 (见 [[independent-type-declarations]])
- `interface_map.h` 保持与实际代码同步
- 多接收方: 发送方为每个接收方定义独立的 __weak 空壳，调用处逐一调用

**关键变更 (v1.0→v2.0):**
- 不再有 `Msg_Post` / `MsgScheduler_Register` / 消息 ID
- 不再有 `core/msg_scheduler.h` include
- 函数签名去掉了 `MsgId_t id` 参数，统一为 `uint16_t param, void *data_ptr`
