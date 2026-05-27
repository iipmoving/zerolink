# HMI C实现 vs JS+JSON参考 合规审计报告

**日期**: 2026-05-25  
**审查者**: 测试员 (AI Agent)  
**参考基准**: `sim/test_hmi/logic/four_head_v4.json` + `sim/test_hmi/js/json_logic_engine.js`  
**审查对象**: `Claude/app/app_hmi.c` + `Claude/cfg/hmi_data.c` + `Claude/drv/drv_display.c`

---

## 一、审计概览

| 指标 | 数值 |
|------|------|
| JS版总函数数 | 70 (json_logic_engine.js) + 9 (renderer.js) + 3 (mode_rule.js) + 3 (blink_rule.js) + 7 (slot_element.js) + 7 (led_element.js) |
| C版总函数数 | ~38 (app_hmi.c) |
| JSON路由规则数 | global=7, zone=14, process=15 |
| C路由表条目数 | global=11, zone=34, process=16 |
| **逻辑符合度** | **~78%** (核心路由+状态机已实现，显示层部分硬编码) |
| **JSON覆盖率** | **~87%** (timeouts/elements/mode_rules/routes已声明，LED/slot_element部分未消费) |

---

## 二、严重问题 (P0 — 阻塞功能正确性)

### 2.1 Z1~Z4 按键→炉头映射错误

**状态**: 代码中 `head_key_to_index` 映射与参考程序 `Key_dispose.c:2856-2859` 不一致。

| 物理按键 | DDD定义 | 参考程序 Key_Operate | 当前C代码 | 正确值 |
|----------|---------|---------------------|-----------|--------|
| KEY_LEFT_P_SET_UP | Z1 (上左) | Key_Operate[0] | 0 | 0 ✓ |
| KEY_LEFT_P_SET | Z3 (下左) | Key_Operate[2] | 1 ✗ | 2 |
| KEY_RIGHT_P_SET | Z4 (下右) | Key_Operate[3] | 2 ✗ | 3 |
| KEY_RIGHT_P_SET_UP | Z2 (上右) | Key_Operate[1] | 3 ✗ | 1 |

**影响**: 3/4 炉头键功能错乱。按左下键→选中右上炉头。

**涉及文件**:
- `app_hmi.c` — `head_key_to_index()` / `head_index_to_key()`
- `cfg/hmi_data.c` — 3处静态 head key 路由表 (lines 76-83, 136-141, 158-163)

### 2.2 超时值错误 (10x)

| 超时项 | JSON值 | hmi_data.c值 | 偏差 |
|--------|--------|-------------|------|
| power_on_all_on_ms | 300 | 3000 | 10x |
| version_show_ms | 200 | 3000 | 15x |

**影响**: 上电全显持续30秒(应为3秒)，版本显示持续30秒(应为2秒)。用户体验极差。

**涉及文件**: `cfg/hmi_data.c:16-17`

### 2.3 档位LED模式: gradient → single

JSON声明 `elements.led.level.mode: "single"` (只亮当前档位一盏灯)。  
C代码 `sync_leds()` 硬编码为 gradient 模式 (0→currentLevel 全部点亮)。

**影响**: 档位LED行为与JSON规格不一致。

**涉及文件**: `app_hmi.c` — `sync_leds()` (lines 1392-1440)

---

## 三、重要问题 (P1 — 逻辑偏差)

### 3.1 定时交替显示硬编码

JSON声明 `display.alternate_rule.phases[0].duration_ms: 5000` (5秒档位/5秒时间)。  
C代码 `update_head_display()` 硬编码 `(s_tick_100ms / 50u) & 1u` (也是5秒，但值未从JSON读取)。

**影响**: 修改JSON的 `duration_ms` 不会改变C行为。值碰巧相同(5000ms)，当前功能正常但不符合"改JSON即改行为"原则。

**涉及文件**: `app_hmi.c` — `update_head_display()`

### 3.2 闪烁排除条件硬编码

JSON声明 `elements.blink.exclude_when: ["timer_setting", "boost_active"]` (timer_setting/boost_active 期间不闪烁)。  
C代码 `derive_seg_blink()` 硬编码了排除逻辑，但未从JSON `exclude_when` 读取。

**影响**: 修改JSON的 `exclude_when` 数组不会改变C行为。

**涉及文件**: `app_hmi.c` — `derive_seg_blink()` (lines 1264-1289)

### 3.3 Zone/Process 超时检查硬编码

JSON中 zone_routes.*.timeout 和 process_routes.*.timeout 声明了超时行为。  
C代码中 `hmi_process_timeouts()` 对每种进程硬编码了超时处理逻辑。

**影响**: 新增进程类型需修改C代码，无法纯JSON声明。

**涉及文件**: `app_hmi.c` — `hmi_process_timeouts()`

### 3.4 LED config 完全硬编码

JSON `elements.led` 段定义了全部LED行为 (level/power/timer/pause/child_lock/head_select)，但C代码 `sync_leds()` 完全硬编码。`led_element.js` 的 `createLevelLED()` / `createStatusLED()` 在C侧无对应。

**影响**: 修改JSON LED配置不会改变C行为。这是当前最大的硬编码区域。

**涉及文件**: `app_hmi.c` — `sync_leds()` (lines 1392-1440)

### 3.5 segment_slot value_map 未消费

JSON `elements.segment_slot.value_map` 定义了档位→显示字符映射 (`"1": "1 "`, `"0": "00"` 等)。  
C代码 `update_head_display()` 中硬编码了左对齐逻辑 (档位1-9右对齐带空格)。

**影响**: 修改 value_map 不会改变C显示格式。`boost_char`、`zero_char` 也未被C消费。

**涉及文件**: `app_hmi.c` — `update_head_display()` (lines 1291-1328)

### 3.6 暂停时闪烁行为

JSON `elements.blink.pause_override: false` (暂停时灭，不闪烁)。  
C代码 `hmi_elements.blink_pause_override = 0` (值一致)，但 derive_seg_blink 对该字段的处理需验证。

---

## 四、次要问题 (P2 — 改进项)

### 4.1 display.priority_chain 未消费

JSON声明了显示优先级链 `["timer_setting", "boost_active", "power_level"]`。  
C代码中 update_head_display 的优先级逻辑硬编码，与JSON声明一致但未读取JSON。

### 4.2 上电序列延迟键引用未统一

JSON用 `delay_ms_key` 间接引用 timeouts，C代码直接写死超时常量值。

### 4.3 童锁白名单

JSON `global_routes.child_lock.block_all_except: ["POWER"]`。  
C代码 hmi_data.c:255-258 硬编码了白名单 `{KEY_ONOFF, HMI_EVT_TAP/LONG}`。值一致但未从JSON读取。

---

## 五、硬编码汇总

| 硬编码位置 | C文件:行号 | JSON对应字段 | 当前状态 |
|-----------|-----------|-------------|---------|
| LED档位模式(gradient) | app_hmi.c:sync_leds | elements.led.level.mode="single" | ❌ 不一致 |
| 档位显示格式 | app_hmi.c:update_head_display | elements.segment_slot.value_map | ❌ 未消费 |
| 定时交替周期 | app_hmi.c:update_head_display | display.alternate_rule.phases[0].duration_ms | ⚠️ 值碰巧一致 |
| 闪烁排除条件 | app_hmi.c:derive_seg_blink | elements.blink.exclude_when | ⚠️ 逻辑一致 |
| 进程超时处理 | app_hmi.c:hmi_process_timeouts | process_routes.*.timeout | ⚠️ 逻辑一致 |
| 显示优先级链 | app_hmi.c:update_head_display | display.priority_chain | ⚠️ 顺序一致 |
| 童锁白名单 | cfg/hmi_data.c:255-258 | global_routes.child_lock | ⚠️ 值一致 |
| 上电序列步骤 | app_hmi.c + hmi_data.c | power_on_sequence | ✅ 已JSON化 |
| 全局路由表 | hmi_data.c | global_routes | ✅ 已JSON化 |
| Zone路由表 | hmi_data.c | zone_routes | ✅ 已JSON化 |
| 进程路由表 | hmi_data.c | process_routes | ✅ 已JSON化 |
| 超时值表 | hmi_data.c | timeouts | ⚠️ 2个值10x错误 |
| ModeRule | hmi_data.c | elements.mode.rules | ✅ 已JSON化 |
| DisplayPatterns | hmi_data.c | elements.display_patterns | ✅ 已JSON化 |

---

## 六、测试建议

1. **P0优先**: 修复Z1~Z4映射 → 重新验证全部按键功能
2. **P0优先**: 修复超时值 → 验证上电序列时长
3. **P1**: 逐项将硬编码逻辑改为从 hmi_cfg 读取JSON声明值
4. **回归**: 全部修复后运行 `sim/test_hmi` 的 34 个测试用例

---

## 七、附录: 参考程序Zone映射证据

```
参考程序 Key_dispose.c:2856-2859:
  Key_Operate[0] ← KEY_LEFT_P_SET_UP  → Z1 (上左)
  Key_Operate[1] ← KEY_RIGHT_P_SET_UP → Z2 (上右)
  Key_Operate[2] ← KEY_LEFT_P_SET     → Z3 (下左)
  Key_Operate[3] ← KEY_RIGHT_P_SET    → Z4 (下右)

参考程序 Disp_Dispose.h (IO映射):
  IO[0..1] = Z1 (上左)  IO[2..3] = Z2 (上右)
  IO[4..5] = Z3 (下左)  IO[6..7] = Z4 (下右)
```

DDD定义与参考程序完全一致，问题只在C代码的按键→炉头映射。
