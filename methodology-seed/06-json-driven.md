# 06 — JSON 驱动三层架构

---

## 一、三层剥离

```
Zone Logic (孤岛函数) ← 纯计算，不依赖外部状态
    ↓
JSON Binding (路由+动作) ← rules.json 唯一数据源
    ↓
Presentation Controls ← SlotElement/LEDElement/BlinkRule/ModeRule
```

**原则**: 可配置的 → JSON 声明。执行函数 → 单向独立。问题 → 隔离处理。

---

## 二、JSON 规则结构

```
项目顶层:
  global:    全局模式规则 (powered_off/working/deep_sleep/paused)
  zone:      炉头状态规则 (idle/selecting/cooking)
  process:   正交进程规则 (timer_setting/timer_active/boost_active)

每个节点包含:
  routes:    按键→动作路由表
  enter:     进入模式时执行的动作
  exit:      退出模式时执行的动作
  guards:    超时守护条件
  timeout:   超时配置

配置元素:
  elements:  LED模式/档位/步长/闪烁相位等
  patterns:  显示模式字符串 ("--", "PA", 全灭等)
  timeouts:  各超时时长 (select_confirm/timer_confirm/boost_max等)
```

---

## 三、C 引擎如何消费 JSON

引擎只做解释，不包含业务规则:

```c
/* 路由查找: 从 JSON 配置表中线性扫描 */
route = hmi_find_route(cfg->routes, cfg->route_count, key, event);
if (route != NULL) {
    hmi_execute_action(route->action, route->param);
}

/* 模式切换: 执行 JSON 声明的 enter/exit 动作 */
go_working() {
    old_cfg = &cfg.global_nodes[s_global.mode];
    apply_actions(old_cfg->exit_actions);   /* JSON 声明的退出动作 */
    s_global.mode = WORKING;
    new_cfg = &cfg.global_nodes[WORKING];
    apply_actions(new_cfg->enter_actions);  /* JSON 声明的进入动作 */
}

/* 超时: 从 JSON 读取时长 */
timeout_ms = cfg.timeouts[SELECT_CONFIRM_MS];
if (elapsed >= timeout_ms) confirm_select();

/* 显示模式: ModeRule 优先级链 */
for (i = 0; i < cfg.mode_rule_count; i++) {
    if (cfg.mode_rules[i].condition == COND_PAUSED && s_global.paused) {
        s_display.seg_mode = cfg.mode_rules[i].seg_mode;
        return;
    }
}
s_display.seg_mode = cfg.elements->mode_default;  /* 回退默认值 */
```

---

## 四、JSON → C 数据生成

```
rules.json  ──→ json_to_c.py ──→ hmi_data.c (编译时嵌入)
            ──→ JS 引擎 (运行时加载)
```

**规则**: JSON 是唯一数据源。禁止在 C 代码中硬编码 JSON 已有的值（超时时长、档位步长、显示字符等）。

---

## 五、引擎禁止做的事

- 在 C 代码中硬编码 JSON 已有的配置值
- 绕过 JSON 路由直接在 C 里写 if-key-then-action
- JSON 规则未在 JS 引擎中验证通过就先写 C
- 在引擎中写业务规则判断（业务规则应 JSON 化或归入孤岛函数）
