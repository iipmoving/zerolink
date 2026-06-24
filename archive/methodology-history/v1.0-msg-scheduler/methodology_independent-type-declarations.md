---
name: independent-type-declarations
description: 跨模块结构体独立声明由AI管理——相同布局不同命名，core/interface_map.h作文档
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 7eb949aa-9578-417f-ab12-d6428ae59747
---

## 铁律四：独立声明，AI 管一致性

**跨模块传递的数据结构，每个模块独立声明自己的类型，互不 include。相同内存布局，不同命名。一致性由 AI 保证。**

**Why:** 用户 2026-05-26 明确要求。共享类型头文件（各模块 include 同一个 .h）增加了一层编译期耦合。每模块独立声明虽然代码重复，但消除了模块间编译依赖。这对人很难（需要手动保持一致性），但对 AI 很容易（AI 可自动对比 sizeof/offsetof）。

**How to apply:**
1. 新增跨层数据传递时，先在 `core/interface_map.h` 注册配对
2. 发送方模块用自己的类型名，接收方模块用自己的类型名
3. 两边的 struct 字段顺序、类型、数组大小必须完全一致
4. 修改任一方时，AI 自动同步修改配对结构体
5. `core/interface_map.h` 仅作文档参考，任何 .c/.h 不得 include 它

**前缀约定:**
- APP 层: `Hmi*` (HmiDisplayCache_t, HmiKeyCode_t, HMI_KEY_*)
- DRV 层: `Drv*` 或原名 (KeyCode_t, KEY_*, Drv_Comm_*)
- CORE: 无前缀 (Msg_t 等基础设施)

**验证方法:** 两个结构体的 sizeof + 每个字段 offsetof 必须相等。

**示例:**
```
APP: HmiDisplayCache_t { char seg_chars[8]; uint8_t seg_blink[4]; ... }
DRV: DisplayFrame_t    { char seg_chars[8]; uint8_t seg_blink[4]; ... }
     相同布局 ✅         sizeof 一致 ✅
     消息通道: MSG_DISPLAY_REFRESH (void* 传递)
```

**已固化位置:** `four_head/Claude/core/interface_map.h` — 当前所有跨模块配对
