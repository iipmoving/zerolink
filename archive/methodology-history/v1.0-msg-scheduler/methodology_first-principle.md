---
name: methodology-first-principle
description: 最高优先级——方法论是项目唯一产出，代码只是验证载体。每次修BUG必须做根因分析并更新方法论
metadata:
  node_type: memory
  type: feedback
  originSessionId: 7eb949aa-9578-417f-ab12-d6428ae59747
---

# ★ 最高原则：方法论是唯一产出

**本项目（四头电磁炉控制程序）的真正目的不是做出一个能用的电磁炉，而是建立一套 AI 零耦合嵌入式开发的、可复制的标准方法论。**

用户于 2026-05-26 明确：项目逻辑里的小 BUG 反而不太关心。为了训练 AI 掌握这套新方法，已是第三次充值。

## 工作优先级

```
1. 方法论与工作流更新    ← 最高优先级，每次问题的最终产出
2. 自动化验证工具        ← 方法论的代码化
3. 架构合规              ← 铁律不能破
4. 项目功能              ← 验证方法论的载体
```

## 每次修 BUG 的强制流程

发现任何问题 → 不直接修 → 先回答三个问题：

```
1. 根因是什么？
   → 是规则不够明确？还是规则明确但违反？
   → 是工具没拦住？还是没有对应的检查项？

2. 方法论缺了什么？
   → 缺了哪条铁律？
   → 缺了哪个检查项？
   → 缺了哪个命名约定？

3. 怎么防止再犯？
   → 更新方法论文档
   → 更新 check_*.py 脚本
   → 更新 interface_map.h 模板
```

**禁止**：修完代码就走，不更新方法论。

## 方法论输出物清单

每次会话结束时检查：

- [ ] `memory/methodology_*.md` — 有没有新增/更新的方法论条目
- [ ] `memory/MEMORY.md` — 索引是否同步
- [ ] `tools/check_*.py` — 验证脚本是否覆盖了新发现的违规类型
- [ ] `core/interface_map.h` — 映射表是否与代码一致
- [ ] `CLAUDE.md` — 铁律是否需要更新

## 成功标准

不是"项目编译通过、功能正常"。
而是——**换一个全新的 AI 对话，读一遍方法论文档，能独立搭建出相同架构的项目。**

---

*关联: [[ai-zero-coupling-methodology]] [[ai-managed-dependency-injection]] [[ai-message-id-namespace]] [[independent-type-declarations]] [[core-principles-iron-laws]]*
