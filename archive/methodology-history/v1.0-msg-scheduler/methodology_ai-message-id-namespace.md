---
name: ai-message-id-namespace
description: [已废弃, v2.0不再需要] AI管理消息ID命名空间——模块各自定义本地ID，interface_map.h维护全局映射
metadata:
  node_type: memory
  type: feedback
  originSessionId: 7eb949aa-9578-417f-ab12-d6428ae59747
  status: deprecated
---

## [已废弃] 铁律六：AI 管理消息 ID 命名空间

> **v2.0 更新 (2026-05-26):** 随着 __weak 回调直调架构的实施，消息 ID 命名空间管理已不再需要。
> 消息 ID 随 `MsgScheduler` 一起被移除。跨模块通信现在通过函数名约定，不再使用数字 ID。
>
> 本文档保留作为 v1.0 历史记录。

---

## 历史背景 (v1.0)

在 v1.0 架构中，每个模块需要定义本地消息 ID (`#define`)，通过 `interface_map.h` 维护全局映射。AI 负责保证发送方和接收方的 ID 数值一致。

## v2.0 替代方案

v2.0 中:
- 消息 ID → 函数名 (`Receiver_OnXxx`)
- `#define MSG_*_IN/OUT` → 不需要
- `Msg_Post(ID, ...)` → `Receiver_OnXxx(param, data)`
- `MsgScheduler_Register(ID, handler)` → 强符号函数实现
- `check_msgs.py` → 待替换为 `check_weak_pairs.py`

详见 [[weak-callback-zero-dependency]] 和 [[ai-zero-coupling-methodology]] v2.0。

---

## 历史命名约定 (v1.0, 已废弃)

**格式**: `{MODULE_PREFIX}_MSG_{PURPOSE}_{DIRECTION}`

```
MODULE_PREFIX  每个模块唯一缩写
PURPOSE        消息用途（蛇形大写）
DIRECTION      _IN(接收) / _OUT(发送)
```

完整命名表见 v1.0 版 `interface_map.h` 或 v1.0 版 `methodology_ai-zero-coupling-complete.md`。

## 历史 AI 工作流 (v1.0, 已废弃)

```
新增消息通道:
1. 读取 interface_map.h → 取下一个可用 ID
2. 发送方: #define {PREFIX}_MSG_{PURPOSE}_OUT  N
3. 接收方: #define {PREFIX}_MSG_{PURPOSE}_IN   N
4. 发送方: Msg_Post(LOCAL_NAME_OUT, ...)
5. 接收方: MsgScheduler_Register(LOCAL_NAME_IN, handler)
6. 更新 interface_map.h
7. check_msgs.py 验证
```

---

*关联: [[weak-callback-zero-dependency]] [[ai-zero-coupling-methodology]] [[ai-managed-dependency-injection]]*
