---
name: golden-output-decomposition
description: 模块分解黄金输出验证——从单体拆模块时，新模块输出必须与原始代码逐字段匹配，不通过不合并
metadata:
  type: methodology
  originSessionId: 7eb949aa-9578-417f-ab12-d6428ae59747
  introduced: 2026-05-26
  trigger: display_module 显示派生偏差 (29条断言失败)
---

# 模块分解黄金输出验证 (Golden-Output Decomposition)

## 一、问题: 为什么 display_module 会偏离原始逻辑

### 1.1 事件回放

```
步骤1: Agent 创建 display_module.c — 基于高层描述 ("派生 seg_chars/LED/seg_mode")
步骤2: Agent 写了简化版显示派生逻辑 (~340行)
步骤3: emcc 编译通过 → 看起来 OK
步骤4: 双引擎测试 → 29 条断言失败
步骤5: 根因 — 简化版逻辑与原 app_hmi.c 的显示派生语义不同
步骤6: 临时修复 — main.c 绕道 state_module 查询 (state_module 保留了原始代码)
步骤7: display_module 收缩为 85 行 blink-only
```

### 1.2 根因

| 层面 | 缺失 |
|------|------|
| **流程** | 没有"新模块输出 vs 原始代码输出"的自动化比对步骤 |
| **Agent 指令** | Agent 收到的是"功能描述"，没有收到原始代码作为参考实现 |
| **验收标准** | 编译通过 ≠ 行为等价。没有定义"输出一致"的具体判定方法 |

### 1.3 核心教训

> **从单体拆模块时，新模块不是"重新实现"，而是"精确搬运"。**
> 搬运的验证标准不是"看着合理"，而是——给相同输入，新模块的每个输出字段与原始代码逐字段一致。

---

## 二、解决方案: 黄金输出验证工作流

### 2.1 三阶段分解流程

```
Phase A: 提取前 — 录制黄金输出
  ├─ 在原始单块代码中插入输出录制桩
  ├─ 跑完整测试序列, 录制每个被提取函数的输入/输出
  └─ 保存为 golden_output.json (不可手动修改)

Phase B: 提取中 — 影子模式验证
  ├─ 新模块编译为独立 .o
  ├─ 与原始代码链接到同一个 test harness
  ├─ test harness 注入相同输入 → 同时调旧函数和新模块
  └─ 逐字段比对输出 → 不一致即失败

Phase C: 验收 — 双引擎回归
  ├─ 全量双引擎测试 (test_wasm_basic.js) 必须 100% PASS
  └─ 只有 Phase B + Phase C 都通过, 模块才算交付
```

### 2.2 Phase A: 录制黄金输出 (具体操作)

在原始单块代码中, 对每个待提取的函数, 插入录制代码:

```c
/* 插入到原始 app_hmi.c 中 */
#ifdef RECORD_GOLDEN_OUTPUT
#include <stdio.h>
static FILE *g_golden = NULL;

static void golden_init(void) {
    g_golden = fopen("golden_output.json", "w");
    fprintf(g_golden, "[\n");
}

static void golden_record_seg_chars(uint8_t pos, char val) {
    fprintf(g_golden, "  {\"fn\":\"seg_chars\",\"pos\":%u,\"val\":%d},\n", pos, val);
}
/* ... 每个输出字段一个 record 函数 ... */
#endif
```

**录制内容**: 每个被提取函数的全部输出字段, 每次调用一条记录。

**关键约束**: 录制代码不得修改任何业务变量。仅读取, 仅写入 golden_output.json。

### 2.3 Phase B: 影子模式 (具体操作)

```c
/* test_harness_shadow.c — 链接旧代码 + 新模块, 逐字段比对 */

#include "app/app_hmi.h"       /* 原始代码的 display 函数 (标记为 old_) */
#include "modules/display_module.h" /* 新模块的 display 函数 */

/* 重命名原始函数避免冲突 */
#define update_head_display  old_update_head_display
#define derive_seg_mode      old_derive_seg_mode
/* ... */

void test_shadow_update_head_display(void) {
    uint8_t i, pos;

    /* 1. 设置相同输入状态 */
    set_test_state(heads, global, tick);

    /* 2. 调旧函数 */
    old_update_head_display(0);
    char old_chars[8];
    memcpy(old_chars, old_get_seg_chars(), 8);

    /* 3. 重置状态, 调新模块 */
    Display_OnStateChange(0, &head0, &global);
    Display_OnTickPhase(tick);

    /* 4. 逐字段比对 */
    for (i = 0; i < 8; i++) {
        uint8_t new_val = Display_GetSegChar(i);
        ASSERT(old_chars[i] == (char)new_val,
               "seg_chars[%u]: old=%c new=%c", i, old_chars[i], (char)new_val);
    }
}
```

**判定**: 任意字段不一致 → 测试失败 → Agent 修复 display_module → 重新验证 → 直到 100% 一致。

### 2.4 为什么此流程能阻止本次偏差

| 本次缺失 | 新流程覆盖 |
|----------|-----------|
| Agent 不知道原始代码的输出 | Phase A 录制了 golden_output.json |
| 没有自动化比对 | Phase B 影子 harness 逐字段 assert |
| 编译通过即认为 OK | Phase C 双引擎 66 条断言必须 100% PASS |

---

## 三、Agent 指令规范 (防止 Agent 自行"简化")

### 3.1 Agent 创建替换模块时的强制提示词模板

```
你正在从 {original_file} 提取 {module_name} 模块。

⛔ 禁止重新实现。你必须精确搬运以下原始函数:
  - {function_1} (位于 {original_file}:{line_start}-{line_end})
  - {function_2} (位于 {original_file}:{line_start}-{line_end})
  - ...

搬运规则:
  1. 函数体逐行保持, 只改以下三样:
     a. 全局变量 → 模块 static 变量 (如 s_heads → s_heads)
     b. 外部函数调用 → __weak 回调 (如 Drv_Display_Commit → DrvDisplay_OnRefresh)
     c. #include 路径 (如 "app_hmi.h" → "modules/display_module.h")
  2. 不调整算法、不合并分支、不优化条件
  3. 搬运完成后, 用 diff 工具比对原函数体与新函数体:
     - 除上述 a/b/c 三类变更外, 不得有其他差异

验收: Phase B 影子模式逐字段比对 100% 一致后才能标记完成。
```

### 3.2 给 Agent 的原始代码最小上下文

Agent 必须收到的信息:

1. **原始函数的完整源码** (不是功能描述, 不是伪代码)
2. **函数被调用的完整参数示例** (输入)
3. **函数输出的完整字段列表** (输出)
4. **golden_output.json** (至少 3 个不同场景的录制)

禁止给 Agent 的是:
- "你实现一个显示派生模块，根据炉头状态派生段码" ← 这会导致 Agent 自行设计
- 功能描述文档替代原始代码

---

## 四、适用场景与限制

### 适用
- 单体文件拆分为多个模块
- 用新实现替换旧实现 (重构)
- Agent 生成替代模块

### 不适用
- 全新功能 (没有原始代码可对照)
- 接口层/接线层 (main.c 之类, 本身不包含业务逻辑)
- 纯配置数据 (cfg/hmi_data.c 由脚本生成)

### 与双引擎测试的关系

```
双引擎测试 (test_wasm_basic.js):  验证 JS引擎 ↔ WASM C引擎 整体行为一致
黄金输出验证 (shadow harness):    验证 新模块 ↔ 原始代码 函数级输出一致

两者互补:
  - 黄金输出: 细粒度, 快速定位到具体函数/字段
  - 双引擎: 端到端, 保证整体行为不退化
```

---

## 五、检查清单

每次从单体拆模块时, 逐项确认:

- [ ] Phase A: 原始代码已插入录制桩, golden_output.json 已生成 (≥3 场景)
- [ ] Phase B: 影子 harness 已编写, 全部 assert 通过
- [ ] Phase C: `node test_wasm_basic.js` → 全部 PASS (断言数不减少)
- [ ] Agent 收到的是原始源码, 不是功能描述
- [ ] 搬运后的函数体与原始函数的 diff 只有三类合法变更 (变量名/回调/include)
- [ ] 新模块的 `interface_map.h` 条目已更新

---

*关联: [[first-principle]] [[weak-callback-zero-dependency]] [[ai-managed-dependency-injection]]*
*触发: display_module 29 条断言失败 → 根因分析 → 本方法论*
*下次验证目标: 新模块拆分时应用此流程, 0 条偏差断言*
