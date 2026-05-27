---
name: design-principle-json-isolation
description: 核心设计原则：可配置化→JSON声明，执行函数→单向独立孤岛，问题→隔离处理
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 8468b018-0dc2-4dea-925a-f289e7ef9c0b
---

核心理念：能配置化、参数化的信息尽可能用 JSON 表述，让最后的执行函数变成单向独立逻辑的程序段。

**Why:** 让每个问题停留在一个孤岛上。以后 AI 来修改也是明确的独立信息处理问题，不需要太多上下文。与程序解耦的目标一致。

**How to apply:**
1. 凡是能改动的参数、条件、映射关系 → 进 JSON
2. 执行函数只做单向解释：读 JSON → 收消息 → 查规则 → 执行动作
3. 新增功能时先问：这个行为能不能用 JSON 声明表达？如果不能，为什么？
4. 每个 JSON 段控制一个独立关注点（元素属性、路由规则、显示模式等），互不交叉

**判断标准：** 改一个行为是否需要改引擎代码？如果需要，那这个行为还没 JSON 化到位。

**反模式：** JSON 和代码各写一半逻辑，修改时两端都要改。

[[milestone-87pct-json-coverage]] [[feedback_json_coverage]]
