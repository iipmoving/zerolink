# JS→C 转换规则

**目标读者**: 将 JS 引擎逻辑转换为 C 代码的程序员
**配套文档**: `docs/architecture/js-json-to-c-encoding-guide.md` (完整映射参考)

---

## 一、转换工作流

```
JS引擎逻辑 (已通过测试验证)          C引擎逻辑
        │                                │
        ├─ 步骤1: 定位对应函数 ──────────┤
        ├─ 步骤2: 转换数据类型 ──────────┤
        ├─ 步骤3: 转换控制流 ────────────┤
        ├─ 步骤4: 双引擎对比测试 ────────┤
        └─ 步骤5: 确认一致后提交 ────────┘
```

**铁律**: 只有 JS+JSON 测试通过后，才允许转换为 C 代码。

---

## 二、数据类型转换规则

### 2.1 JS 字符串枚举 → C enum

```c
// JS: global_mode = 'powered_off'
// C:  s_global.mode = HMI_NODE_POWERED_OFF

// 映射表见 app_hmi.h:
typedef enum {
    HMI_NODE_POWERED_OFF = 0,
    HMI_NODE_DEEP_SLEEP,
    HMI_NODE_WORKING,
    HMI_NODE_PAUSED,
    HMI_NODE_COUNT
} HmiGlobalNode_t;
```

**规则**: 所有状态/模式值必须用 enum，禁止 magic number。

### 2.2 JS 动态对象 → C struct

```c
// JS: heads[0] = { node: 'cooking', power_level: 5, boost_active: true, ... }
// C:
typedef struct {
    HmiZoneNode_t node;           // 0=idle, 1=selecting, 2=cooking
    uint8_t       power_level;    // 0-9
    uint8_t       boost_active;   // 0/1
    uint8_t       timer_setting;  // 0/1
    uint8_t       timer_active;   // 0/1
    uint8_t       timer_value;    // 1-99 (minutes)
    uint32_t      select_ticks;
    uint32_t      timer_set_ticks;
    uint32_t      boost_remaining_ms;
} HmiHead_t;
```

**规则**: 一个 JS 对象 → 一个 C struct。字段一一对应，布尔值用 `uint8_t` (0/1)。

### 2.3 JS Map/Array → C 静态数组

```c
// JS: global_routes.powered_off.routes = [...]
// C:
static const HmiRoute_t s_routes_powered_off_2[] = {
    { KEY_ONOFF, HMI_EVT_LONG, HMI_ACT_GO_WORKING, 0 }
};
```

**规则**: 用 `static const` 数组，编译期确定，不动态分配。

### 2.4 JS 回调函数 → C 函数指针/直接调用

```c
// JS: MessageBus.register(MSG_KEY_EVENT, onKeyEvent)
// C:
MsgScheduler_Register(MSG_KEY_EVENT, on_key_event);
```

**规则**: 回调签名统一为 `void (MsgId_t, uint16_t, void*)`。

---

## 三、控制流转换规则

### 3.1 路由匹配: JSON路径 → switch/if 链

```c
// JS: 动态 JSON 匹配
// const routes = cfg.routes[global_mode];
// for (route of routes) { if (match) execute(route.action); }

// C: 编译期展开
static const HmiRoute_t *routes = global_cfg->routes;
for (i = 0; i < global_cfg->route_count; i++) {
    if (routes[i].key_code == key && routes[i].event == evt) {
        hmi_execute_action(routes[i].action, routes[i].param, key, 0);
        break;
    }
}
```

### 3.2 优先级链: if-else 顺序

```c
// JS: 优先级链遍历
// C: 等价的顺序 if-else（数组遍历）
for (i = 0; i < hmi_cfg.mode_rule_count; i++) {
    if (evaluate_condition(rule->condition)) {
        s_display.seg_mode = rule->seg_mode;
        return;
    }
}
s_display.seg_mode = hmi_cfg.elements->mode_default;
```

**规则**: 优先级顺序必须与 JSON 声明一致，不可打乱。

### 3.3 动态路由: 运行时解析

```c
// JS: resolveHeadRouteKey('HEAD_SELF', key)
// C:
static int8_t hmi_match_dynamic_route(uint8_t key, const HmiRoute_t *route)
{
    if (route->key_code == HMI_KEY_HEAD_SELF)
        return is_head_key_for_hot_head(key) ? 1 : -1;
    if (route->key_code == HMI_KEY_HEAD_OTHER)
        return is_head_key_for_other(key) ? 1 : -1;
    return (route->key_code == key) ? 0 : -1;
}
```

---

## 四、显示管线转换规则

```
JS: head state → SlotElement.render() → seg_chars → BlinkRule → ModeRule → postDisplay()
C:  HmiHead_t → update_head_display()  → seg_chars → derive_seg_blink() → derive_seg_mode() → post_display()
```

### 4.1 seg_chars 写入

每个炉头占 2 个字符位。映射规则 (index → seg_chars offset):
- head 0 (Z1) → seg_chars[0..1]
- head 1 (Z2) → seg_chars[2..3]
- head 2 (Z3) → seg_chars[4..5]
- head 3 (Z4) → seg_chars[6..7]

### 4.2 post_display 后处理

C 引擎在 `post_display()` 后必须调用 LED 同步:
```c
sync_leds();    // 根据 s_global.mode 和 s_hot_head 修正 LED 状态
post_display(); // 发送 MSG_DISPLAY_REFRESH
```

---

## 五、消息编码规则

### 5.1 按键事件编码

```
MCU: param = (evt << 8) | key
WASM: engine_post_key(key, evt) → on_key_event(MSG_KEY_EVENT, param, NULL)

evt: KEY_STATE_TAP=0x10, KEY_STATE_LONG=0x02
     HMI_EVT_TAP=0, HMI_EVT_LONG=1 (HMI路由匹配用, 与KEY_STATE不同!)
```

**注意**: 驱动层 `KEY_STATE_TAP=0x10` 与 HMI 层 `HMI_EVT_TAP=0` 是两套常量。
`on_key_event()` 负责转换: `evt = (param >> 8) == 0x10 ? HMI_EVT_TAP : HMI_EVT_LONG`。

### 5.2 定时器消息

```
MSG_TIMER_100MS (msgId=5): 每100ms, param=0
MSG_TIMER_1S    (msgId=6): 每1s,   param=0
```

---

## 六、常见转换错误

| 错误 | JS 表现 | C 修正 |
|------|---------|--------|
| enum 值用 magic number | — | 必须用 `HMI_NODE_*` / `HMI_ZONE_*` 枚举 |
| 忘记 sync_leds() 后 post_display() | — | LED 状态变更后必须调用这对函数 |
| 路由优先级与 JSON 不一致 | 结果不同 | 严格按 JSON routes 数组顺序 |
| 超时单位混淆 | ms | C 用 ms，tick 计数 `*100u` 转换 |
| 栈去重逻辑错误 | push 重复项 | `push_to_stack()` 先 `remove_from_stack()` |
| 显示区映射错误 | 炉头显示位置错 | Z1→[0,1], Z2→[2,3], Z3→[4,5], Z4→[6,7] |
| 上电序列步骤计数 | — | `power_on_step_ticks` 在首次进入步骤前为 0 |

---

## 七、验证方法

转换完成后，必须通过 WASM 双引擎对比测试:

```bash
cd sim/test_hmi/wasm
make wasm_test    # 编译 WASM + 运行测试
```

测试通过标准: 所有 assertState 和 assertDisplay 的 PASS/FAIL 全部 PASS，0 FAIL。

**测试覆盖要求**:
- 关键数据流: 每个 Zone Logic 动作函数至少一个端到端场景
- 边界条件: 0档、9档、Boost、多炉头切换
- 进程覆盖: timer_setting、timer_active、boost_active

---

## 八、参考

- 完整函数对应表: `docs/architecture/js-json-to-c-encoding-guide.md`
- C 引擎入口: `sim/test_hmi/wasm/app/app_hmi.c`
- WASM 适配器: `sim/test_hmi/wasm/wasm_adapter.js`
- 双引擎测试: `sim/test_hmi/wasm/test_wasm_basic.js`
- JSON 配置生成: `sim/test_hmi/wasm/cfg/hmi_data.c`

---

*维护规则: 新增 JS→C 转换规则时同步更新本文档。*
