---
name: key-release-tap-pattern
description: "长按/短按二义性消除 — 松手判定 TAP, 长按抑制误触发"
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 8468b018-0dc2-4dea-925a-f289e7ef9c0b
---

按键状态的使用模式：

**核心规则**: 带长按功能的按键，通过松手状态判断第一次单按是否需要触发。若按键最终为长按，则短按动作不应响应，防止误触发一次。

**实现** (已在 `drv_key.c` 中):
- `Key_Release()` 检查 `!s_long_sent` 才发送 `KEY_STATE_TAP`
- 长按 (≥1s): 发送 `KEY_STATE_LONG`, 不发送 `KEY_STATE_TAP`
- 短按 (<1s): 发送 `KEY_STATE_TAP`, 不发送 `KEY_STATE_LONG`
- TAP 和 LONG 互斥, 不会同时出现

**HMI 层用法**:
- 同时有 TAP 和 LONG 路由的按键 (如开关): TAP 事件只在短按时触发, LONG 只在长按时触发, 不会误响应
- 仅有 TAP 路由的按键 (如档位键): 屏蔽 `KEY_STATE_RELEASE` 和 `KEY_STATE_PRESS`, 只处理 TAP
- 非长按按键: 直接将松开状态 (`KEY_STATE_RELEASE`) 屏蔽即可

**Why:** 用户明确指示 — 长按按键在松手时才判定是长按还是短按, 避免按下瞬间误触发短按动作。

**How to apply:** 新增按键路由时, 若按键同时有 TAP 和 LONG 两条路由, 无需额外处理 (驱动层已保证互斥)。若按键只有 TAP 路由, 在 HMI 的 `on_key_event` 中继续过滤掉 PRESS/RELEASE, 只保留 TAP。
