# 审查报告与指导 — data_switcher.c 数据流时间修复

> **致正在处理 four_head 项目 v2.3 转换的 AI**：你修正了命名约定问题，但 `data_switcher.c` 中存在一个关键的数据流时间错误，导致 AppPower→AppHmi 管道永远无法工作。请按以下步骤修复。

---

## 一、根本原因：InputCallback 在消费者时间运行，而非生产者时间

**当前代码**：
```c
// Switcher_Run():
s_slots[SLOT_KEY].pDoWork();       // DrvKey 先运行
_route_key();
s_slots[SLOT_HMI].pDoWork();       // AppHmi 第二运行 → 期间调 AppHmi_InputCallback
_route_hmi();
...
s_slots[SLOT_POWER].pDoWork();     // AppPower 最后运行！
```

**问题**：`AppHmi_InputCallback` 在 `AppHmi_DoWork` 期间被调用（因为 `MODULE_SKELETON` 的 `DoWork` 调用了 `name##_InputCallback`）。此时，AppPower 还没有运行。读取 `s_slots[SLOT_POWER].pOut` 得到的是过时/零数据。

**规则**：每个数据管道必须在其**生产者** DoWork **之后立即路由**，而不是在消费者 DoWork 期间路由。

---

## 二、修复方案

### 2.1 AppHmi_InputCallback 只读 DrvKey（此时唯一可用的生产者）

```c
/* ===== AppHmi InputCallback — 只读 DrvKey（此时 AppPower 尚未运行） ===== */
INPUT_CALLBACK(Key, Hmi)
{
    INPUT_GET_SLOT(Key, Hmi);
}
```

删除 AppPower 通道的部分代码——它在这里永远无法工作。

### 2.2 AppPower→AppHmi 路由放在 AppPower DoWork 之后

```c
s_slots[SLOT_POWER].pDoWork();
/* 此时 AppPower 数据就绪，路由到 AppHmi */
INPUT_GET_SLOT(Power, Hmi);
```

### 2.3 _route_key 改为空（路由已经由 INPUT_GET_SLOT 完成）

```c
static void _route_key(void)
{
    /* 路由已由 INPUT_GET_SLOT(Key, Hmi) 在 Switcher_Run 中完成 */
}
```

### 2.4 Switcher_Run 整体结构

```c
void Switcher_Run(void)
{
    /* DrvKey: 按键事件生产者 */
    if (s_slots[SLOT_KEY].pDoWork) s_slots[SLOT_KEY].pDoWork();
    INPUT_GET_SLOT(Key, Hmi);
    _route_key();

    /* AppHmi: 消费 DrvKey 数据，产生 display/buzzer 输出 */
    if (s_slots[SLOT_HMI].pDoWork) s_slots[SLOT_HMI].pDoWork();
    _route_hmi();

    /* AppCommMgr: MODBUS 数据生产者 */
    if (s_slots[SLOT_COMM_MGR].pDoWork) s_slots[SLOT_COMM_MGR].pDoWork();
    // TODO: INPUT_GET_SLOT(CommMgr, Power) 等

    /* AppPower: 功率状态生产者 → AppHmi 消费 */
    if (s_slots[SLOT_POWER].pDoWork) s_slots[SLOT_POWER].pDoWork();
    INPUT_GET_SLOT(Power, Hmi);

    /* 其余模块 */
    if (s_slots[SLOT_PROTECT].pDoWork) s_slots[SLOT_PROTECT].pDoWork();
    if (s_slots[SLOT_SEG_ALIGN].pDoWork) s_slots[SLOT_SEG_ALIGN].pDoWork();
    if (s_slots[SLOT_COMM_MGR_DRV].pDoWork) s_slots[SLOT_COMM_MGR_DRV].pDoWork();
    if (s_slots[SLOT_BUZZER].pDoWork) s_slots[SLOT_BUZZER].pDoWork();
    if (s_slots[SLOT_DISPLAY].pDoWork) s_slots[SLOT_DISPLAY].pDoWork();
}
```

---

## 三、关于 OutputCallback 和层隔离

当前代码中存在两个 APP→DRV 的直接调用：

| 位置 | 调用 | 问题 |
|------|------|------|
| `app_hmi.c:1293` `post_display()` | `DrvDisplay_OnRefresh(0, &s_display)` | APP 层直接调 DRV 函数 |
| `data_switcher.c` `AppHmi_OutputCallback` | `DrvBuzzer_OnCtrl(1, NULL)` | APP 层直接调 DRV 函数 |

**本阶段不做修改**——display 的 1ms 实时路径和 buzzer 的即时响应是 four_head 现有架构设计的一部分。但它们不是 v2.3 PULL 范式的标准模式。标注 `/* @OUTPUT_CALLBACK: legacy — 1ms display sync */` 作为记录。

---

## 四、需要修改的文件

| # | 文件 | 修改内容 |
|---|------|----------|
| 1 | `four_head/src/core/data_switcher.c` | 重写 `AppHmi_InputCallback` → 只保留 DrvKey；将 AppPower 路由移到 `Switcher_Run` 中；清理 `_route_key` |
| 2 | `four_head/src/core/data_switcher.h` | 可选：添加 `INPUT_CALLBACK`/`INPUT_GET_SLOT` 宏声明（目前仅在 .c 中定义也可） |

---

## 五、验证标准

修改后确认：

1. ✅ `AppHmi_InputCallback` 不再引用 `s_slots[SLOT_POWER]`
2. ✅ `INPUT_GET_SLOT(Power, Hmi)` 在 `s_slots[SLOT_POWER].pDoWork()` 之后调用
3. ✅ `AppHmi_ProcessInput` 的两个输入源（`in->DrvKey_params` 和 `in->AppPower_params`）分别在各自数据就绪时指向有效数据
4. ✅ `_route_key()` 为空或只保留注释

---

## 六、不要做的事情

- ❌ 不要修改 `_register` 函数签名或注册逻辑——当前的 4 参数 `_register` 是正确的
- ❌ 不要删除 `AppHmi_OutputCallback`——buzzer 即时路由是合法的 @OUTPUT_CALLBACK 场景
- ❌ 不要重构 `post_display()` 或 `s_out` 类型——display 架构变更超出本次范围
- ❌ 不要修改其他模块的 `.c` 文件（`app_power.c`、`app_comm_mgr.c` 等）——只改 data_switcher.c
- ❌ 不要在你的技能目录中创建新文件——阅读现有的 `.claude/skills/` 文件即可

---

请修改后生成测试报告 `commCheck/check_report_v2.md`，验证：
1. AppHmi_InputCallback 不再引用 AppPower
2. AppPower 路由发生在 AppPower DoWork 之后
3. 编译通过（如果编译器可⽤）
