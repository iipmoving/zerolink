# 审查报告 v3 — data_switcher.c 数据流时间未修复

> **审查对象**: four_head data_switcher.c (实际代码) + check_report_v2.md (AI 测试报告)
> **审查结论**: 命名约定已修，**但关键的数据流时间错误未修复**

---

## 一、check_report_v2.md 审查

### 1.1 覆盖范围

| 检查项 | 结果 | 说明 |
|--------|------|------|
| OUTPUT 成员命名 `{Consumer}_params` | ✅ PASS | to_hmi → Hmi_params 等 |
| INPUT 成员命名 `{Producer}_params` | ✅ PASS | key → DrvKey_params 等 |
| 宏使用 (INPUT_CALLBACK/INPUT_GET_SLOT) | ✅ PASS | 已定义并使用 |
| 跨文件成员引用更新 | ✅ PASS | drv_key.c / data_switcher.c |
| 管道配对一致性 | ✅ PASS | 对称命名正确 |
| ST_NEW 不被 InputCallback 修改 | ✅ PASS | 已移除 status |= ST_NEW |

### 1.2 遗漏检查项

| 检查项 | 结果 | 说明 |
|--------|------|------|
| **数据流时序正确性** | **❌ FAIL** | AppPower 路由仍在 InputCallback 内，但 AppPower 在 Switcher_Run 中晚于 AppHmi 执行 |
| Switcher_Run 执行顺序 | ❌ FAIL | AppPower (SLOT_POWER=1) 在 AppHmi (SLOT_HMI=4) 之后 |
| INPUT_GET_SLOT 宏使用 | ❌ FAIL | 未使用宏，用手写指针访问 |

**结论**: check_report_v2.md **只验证了命名，未验证数据流时序**。测试覆盖不完整。

---

## 二、实际代码缺陷明细

### 2.1 缺陷 1 (严重): AppPower→AppHmi 时序错误

**位置**: [data_switcher.c:136-148](file:///D:/OBSIDIAN/MOVING%20IH/ZEROLINK/four_head/src/core/data_switcher.c#L136-L148)

```c
/* ========== 通道2: AppPower → AppHmi (功率状态) ========== */
{
    Para_Grp_t *pOut = s_slots[SLOT_POWER].pOut;   // ← AppPower 还没运行！
    ...
    hmi_in->AppPower_params = (Power_to_Hmi_Input_Link *)power_out->Hmi_params;
}
```

**原因**: 这个代码在 `AppHmi_InputCallback` 中，而 InputCallback 是在 `AppHmi.DoWork()` 期间被 `MODULE_SKELETON` 调用的。此时 `AppPower.DoWork()` 尚未执行。

**Switcher_Run() 执行顺序** (第 206-226 行):
```
SLOT_KEY      → pDoWork()    ← DrvKey: ✅ 先于 AppHmi
SLOT_HMI      → pDoWork()    ← AppHmi: 期间调 InputCallback 读 AppPower → ❌ 数据无效
SLOT_COMM_MGR → pDoWork()
SLOT_POWER    → pDoWork()    ← AppPower: 此时才产生数据，但已无人消费
```

### 2.2 缺陷 2 (中): 未使用 INPUT_GET_SLOT 宏

**位置**: [data_switcher.c:113-149](file:///D:/OBSIDIAN/MOVING%20IH/ZEROLINK/four_head/src/core/data_switcher.c#L113-L149)

InputCallback 使用手写指针访问 (`Para_Grp_t *pIn = s_slots[SLOT_HMI].pIn;`)，而不是 `INPUT_GET_SLOT(Key, Hmi)` 宏。

宏定义 (第 29-32 行) 已存在于文件顶部，但未使用。

### 2.3 缺陷 3 (低): _route_key 中有空检查代码

**位置**: [data_switcher.c:153-165](file:///D:/OBSIDIAN/MOVING%20IH/ZEROLINK/four_head/src/core/data_switcher.c#L153-L165)

`_route_key()` 函数体已无实际路由功能，但仍保留完整的空指针检查和读 `s_slots[SLOT_KEY]` 的代码。属于 dead code。

### 2.4 缺陷 4 (低): Switcher_Run 注释混乱

**位置**: [data_switcher.c:206-226](file:///D:/OBSIDIAN/MOVING%20IH/ZEROLINK/four_head/src/core/data_switcher.c#L206-L226)

```
/* Phase 1+2 交错: Producer DoWork → 立即路由 → Consumer DoWork */
```

但这个注释描述的架构与实际代码不符。实际代码是"Producer DoWork → 在 Consumer InputCallback 中路由"。

---

## 三、根因分析

### 3.1 AI 为什么没修？

guidance_prompt.md 已经明确指出问题并给出了修复方案。但 AI 只执行了：
- 命名重命名 (to_hmi → Hmi_params 等) — 机械替换，容易做
- 移除 `status |= ST_NEW` — 机械删除，容易做

但**没有执行**：
- 将 AppPower 路由移出 InputCallback
- 在 Switcher_Run 中 AppPower.DoWork() 之后插入 `INPUT_GET_SLOT(Power, Hmi)`

**原因推测**: AI 不理解"数据流时序"这个概念的严重性。命名修正是语法级，时序修正是架构级。AI 倾向于做"能编译通过的修改"而非"逻辑正确的修改"。

### 3.2 工具缺失

现有 check 体系检查的是**静态结构**（命名、依赖、结构体布局），不检查**运行时序**（谁在谁之前执行）。时序错误没有自动化阻断手段。

---

## 四、正确修复方案

### 4.1 AppHmi_InputCallback 只读 DrvKey

```c
INPUT_CALLBACK(Key, Hmi)
{
    INPUT_GET_SLOT(Key, Hmi);
    /* 只有 DrvKey 在 AppHmi 之前运行，可以安全读取 */
}
```

### 4.2 AppPower 路由移到 AppPower.DoWork() 之后

```c
void Switcher_Run(void)
{
    /* DrvKey → AppHmi */
    if (s_slots[SLOT_KEY].pDoWork) s_slots[SLOT_KEY].pDoWork();

    /* AppHmi: 消费 DrvKey */
    if (s_slots[SLOT_HMI].pDoWork) s_slots[SLOT_HMI].pDoWork();

    /* AppPower: 功率数据生产者 */
    if (s_slots[SLOT_POWER].pDoWork) s_slots[SLOT_POWER].pDoWork();
    /* 立即路由: AppPower → AppHmi */
    INPUT_GET_SLOT(Power, Hmi);

    /* 其余模块... */
}
```

### 4.3 关键原则

| 原则 | 说明 |
|------|------|
| 每个管道在**生产者 DoWork 之后立即路由** | 不是等到消费者 DoWork 再路由 |
| InputCallback 只能读**已执行过的**生产者 | 不能读在它之后才运行的模块 |
| 使用 `INPUT_GET_SLOT` 宏 | 不用手写指针访问 |

---

## 五、正确性验证方法

修改后应检查：

1. `AppHmi_InputCallback` 中不出现 `SLOT_POWER` / `AppPower` / `power_out` 的任何引用
2. `Switcher_Run()` 中 `SLOT_POWER.pDoWork()` 调用之后紧跟着 `INPUT_GET_SLOT(Power, Hmi)`
3. 枚举定义中 `SLOT_POWER` 的值大于 `SLOT_HMI`（即 AppPower 在 AppHmi 之后执行）
4. 所有输入数据管道使用 `INPUT_GET_SLOT` 宏，无手写指针访问

---

## 六、工具改进建议

### 6.1 增加 `check_switcher_timing.py` 工具

功能：读取 `data_switcher.c`，分析 Switcher_Run() 中的执行顺序，检查每个 InputCallback 访问的 SLOT 是否在正确的时序位置。

实现思路：
1. 解析 SLOT 枚举定义，建立 SLOT 名称→索引映射
2. 解析 Switcher_Run() 函数，建立执行顺序列表
3. 解析每个 InputCallback 函数，提取它访问的 SLOT
4. 验证：InputCallback 只访问在它之前已执行的 SLOT

### 6.2 修改 check SKILL

在 check SKILL 中增加一个步骤：

```
--- x/6 check_switcher_timing.py ---
如果项目使用 data_switcher，检查数据流时序是否正确
```

---

*审查报告生成: 2026-06-13*
