# 04 — 模块分解黄金输出验证

> **问题**: 从单体拆模块时，Agent 容易"重新实现"而非"精确搬运"。简化逻辑看似合理，但会引入行为偏差。
>
> **解决方案**: 新模块不是"重新实现"，而是"精确搬运"。验证标准不是"看着合理"，而是给相同输入，每个输出字段与原始代码逐字段一致。

---

## 一、三阶段分解流程

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
  ├─ 全量双引擎测试必须 100% PASS
  └─ Phase B + Phase C 都通过, 模块才算交付
```

---

## 二、Phase A: 录制黄金输出

在原始代码中，对每个待提取的函数插入录制代码:

```c
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

**录制内容**: 每个被提取函数的全部输出字段，每次调用一条记录。

**关键约束**: 录制代码不得修改任何业务变量。仅读取，仅写入 golden_output.json。

---

## 三、Phase B: 影子模式

```c
/* test_harness_shadow.c — 链接旧代码 + 新模块, 逐字段比对 */

#include "app_hmi.h"       /* 原始代码 */
#include "display_module.h" /* 新模块 */

void test_shadow_update_head_display(void) {
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
        ASSERT(old_chars[i] == (char)Display_GetSegChar(i),
               "seg_chars[%u]: old=%c new=%c", i, old_chars[i], (char)Display_GetSegChar(i));
    }
}
```

**判定**: 任意字段不一致 → 测试失败 → 修复 → 重新验证 → 直到 100% 一致。

---

## 四、Agent 指令规范（防止自行"简化"）

### 给 Agent 的强制提示词模板

```
你正在从 {original_file} 提取 {module_name} 模块。

⛔ 禁止重新实现。你必须精确搬运以下原始函数:
  - {function_1} (位于 {original_file}:{line_start}-{line_end})
  - {function_2} (位于 {original_file}:{line_start}-{line_end})

搬运规则:
  1. 函数体逐行保持, 只改以下三样:
     a. 全局变量 → 模块 static 变量
     b. 外部函数调用 → __weak 回调
     c. #include 路径
  2. 不调整算法、不合并分支、不优化条件
  3. 搬运完成后, 用 diff 比对原函数体与新函数体:
     - 除上述 a/b/c 三类变更外, 不得有其他差异

验收: Phase B 影子模式逐字段比对 100% 一致后才能标记完成。
```

### Agent 必须收到的信息

1. 原始函数的**完整源码**（不是功能描述，不是伪代码）
2. 函数输入参数的完整示例
3. 函数输出的完整字段列表
4. `golden_output.json`（至少 3 个不同场景的录制）

### 禁止给 Agent 的

- "你实现一个显示派生模块，根据状态派生段码" ← 这会导致 Agent 自行设计
- 功能描述文档替代原始代码

---

## 五、检查清单

每次从单体拆模块时:

- [ ] Phase A: 录制桩已插入, golden_output.json 已生成（≥3 场景）
- [ ] Phase B: 影子 harness 已编写, 全部 assert 通过
- [ ] Phase C: 双引擎回归 100% PASS（断言数不减少）
- [ ] Agent 收到的是原始源码，不是功能描述
- [ ] 新函数体与原始函数的 diff 只有三类合法变更（变量名 / 回调 / include）
- [ ] `interface_map.h` 条目已更新

---

## 六、适用与不适用

### 适用
- 单体文件拆分为多个模块
- 用新实现替换旧实现（重构）
- Agent 生成替代模块

### 不适用
- 全新功能（没有原始代码可对照）
- 接口层/接线层（main.c 之类，本身不包含业务逻辑）
- 纯配置数据（由脚本生成）
