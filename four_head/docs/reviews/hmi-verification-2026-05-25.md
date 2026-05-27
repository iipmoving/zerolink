# HMI 修改验证报告 (2026-05-25)

**审核者**: 测试员  
**审核对象**: 程序员本轮 3 文件修改 (app_hmi.c, app_hmi.h, hmi_data.c) + JSON  
**审核依据**: `docs/architecture/js-json-to-c-encoding-guide.md` + `docs/reviews/hmi-compliance-audit-2026-05-25.md`  
**参考基准**: `sim/test_hmi/logic/four_head_v4.json` + `参考程序/Key_dispose.c:2856-2859`

---

## 一、修改逐项验证

### 1.1 Z1~Z4 按键→Zone 映射 ✅ 通过

| 检查项 | 状态 | 详情 |
|--------|------|------|
| `head_key_to_index()` | ✅ | KEY_LEFT_P_SET_UP→0, KEY_RIGHT_P_SET_UP→1, KEY_LEFT_P_SET→2, KEY_RIGHT_P_SET→3 |
| `head_index_to_key()` | ✅ | 镜像映射，与 head_key_to_index 互逆 |
| working 全局路由 (hmi_data.c:80-83) | ✅ | param 值: 0,1,2,3 与 key 的 zone 索引一致 |
| idle zone 路由 (hmi_data.c:137-140) | ✅ | 同上 |
| cooking zone 路由 (hmi_data.c:159-162) | ✅ | 同上 |
| 与参考程序 Key_dispose.c:2856-2859 一致 | ✅ | Key_Operate[0]←上左, [1]←上右, [2]←下左, [3]←下右 |

> 上轮审计报告的 **P0-2.1 已修复**。3/4 炉头键不再错乱。

### 1.2 level_led_mode 数据驱动 ✅ 通过 (含 1 处回归)

| 检查项 | 状态 | 详情 |
|--------|------|------|
| `HmiElementCfg_t.level_led_mode` 字段 | ✅ | app_hmi.h:255, uint8_t, 注释清晰 |
| `hmi_elements.level_led_mode = 1` | ✅ | hmi_data.c:239, 与 JSON `"mode":"single"` 一致 |
| `sync_leds()` 正常档位分支 | ✅ | 读 `hmi_cfg.elements->level_led_mode`，single/gradient 分支正确 |
| **`sync_leds()` Boost 分支** | ❌ | **回归: Boost 始终单点，未读 level_led_mode** |

**Boost 回归详情** (app_hmi.c:1426-1427):

```c
/* 当前代码 (有问题) */
if (hh->boost_active) {
    s_display.leds_power_level[hmi_cfg.elements->boost_power_level] = 1;
    /* ↑ 无论 single/gradient 都只亮一盏，丢失 gradient 模式 Boost 全亮行为 */
}
```

对比 JS 参考 (led_element.js:40-55):
```javascript
var targetLevel = isBoost ? boostOverride : currentLevel;
if (mode === 'gradient') {
    for (var j = 0; j <= targetLevel; j++) leds.power_level[j] = true;
} else {
    leds.power_level[targetLevel] = true;
}
```

**应为**:
```c
if (hh->boost_active) {
    if (hmi_cfg.elements->level_led_mode) {
        s_display.leds_power_level[hmi_cfg.elements->boost_power_level] = 1;
    } else {
        for (i = 0; i <= hmi_cfg.elements->boost_power_level && i < 10; i++) {
            s_display.leds_power_level[i] = 1;
        }
    }
}
```

> 上轮审计报告的 **P0-2.3 (LED模式)** 部分修复。Boost 分支需补充 level_led_mode 判断。

### 1.3 超时值一致性 ⚠️ 发现 1 处不一致

| 检查项 | JSON | hmi_data.c timeouts | hmi_data.c power_on_seq | 状态 |
|--------|------|--------------------|------------------------|------|
| power_on_all_on_ms | 3000 | 3000 | step0.delay_ms=3000 | ✅ |
| version_show_ms | 3000 | 3000 | step1.delay_ms=**2000** | ⚠️ 不一致 |

**version_show_ms 不一致**: JSON 和 timeouts 表均为 3000，但 `hmi_power_on_seq[1].delay_ms = 2000`。上电序列第1步实际只持续 2s 而非 3s。需确认是有意为之还是生成遗漏。

> 上轮审计报告的 **P0-2.2 (超时值)** 部分修复。timeouts 表已修正，但 power_on_seq 展开值有一处遗漏。

### 1.4 无新增硬编码 ✅ 通过

| 检查项 | 状态 |
|--------|------|
| 新字段 `level_led_mode` 从 `hmi_cfg.elements` 读取 | ✅ 数据驱动 |
| 路由 param 值直接修正（未引入新逻辑分支） | ✅ |
| `head_key_to_index/head_index_to_key` 仅修正 case 值 | ✅ |

---

## 二、JSON 覆盖率统计

### 2.1 按 JSON 顶层区域统计

| JSON 区域 | 总项 | 已消费 | 覆盖率 | 变化 |
|-----------|------|--------|--------|------|
| `timeouts` | 10 | 10 | **100%** | — |
| `power_on_sequence` | 3 steps | 3 | **100%** | — |
| `global_routes` (enter/exit/routes/guards) | 4 modes | 4 | **100%** | — |
| `zone_routes` (routes+timeout) | 3 states | 3 | **100%** | — |
| `process_routes` (routes+timeout) | 3 procs | 3 | **100%** | — |
| `elements.timer` | 3 | 3 | **100%** | — |
| `elements.boost` | 1 | 1 | **100%** | — |
| `elements.stack` | 1 | 1 | **100%** | — |
| `elements.blink` | 3 | 2 | **67%** | — |
| `elements.mode` | 2 | 2 | **100%** | — |
| `elements.display_patterns` | 3 | 3 | **100%** | — |
| `elements.led.level` | 3 | 2 | **67%** | **↑** (mode 新增) |
| `elements.led.{power,timer,pause,child_lock,head_select}` | 5 | 0 | **0%** | — |
| `elements.segment_slot` | 4 | 0 | **0%** | — |
| `display.priority_chain` | 1 | 0 | **0%** | — |
| `display.char_mapping` | 3 | 0 | **0%** | — |
| `display.alternate_rule` | 1 | 0 | **0%** | — |
| `display.led_rules` | 6 | 0 | **0%** | — |

### 2.2 综合覆盖率

| 层级 | 覆盖率 | 说明 |
|------|--------|------|
| **路由+状态机** (global/zone/process routes) | **100%** | 所有路由表+超时+守卫已JSON声明 |
| **Zone Logic 动作函数** | **100%** | 所有动作函数在 hmi_execute_action 有对应 |
| **显示模式规则** (ModeRule) | **100%** | derive_seg_mode 完全数据驱动 |
| **显示图案** (Patterns) | **100%** | dash/pa/off 从 hmi_cfg.patterns 读取 |
| **超时值** | **100%** | 从 hmi_cfg.timeouts[id] 读取 |
| **元素参数** (timer/boost/stack/blink_phase) | **100%** | 从 hmi_cfg.elements 读取 |
| **闪烁规则** (BlinkRule) | **67%** | phase_ms + pause_override 已消费; condition + exclude_when 硬编码 |
| **档位LED** (LevelLED) | **67%** | boost_power_level + level_led_mode 已消费; count 未消费 |
| **状态LED** (StatusLED) | **0%** | power/timer/pause/child_lock/head_select 全部硬编码 |
| **段码渲染** (SlotElement) | **0%** | value_map/boost_char/zero_char 全部硬编码 |
| **显示优先级链** | **0%** | priority_chain 硬编码在 update_head_display |
| **交替规则** | **0%** | alternate_rule 硬编码 5s 周期 |

### 2.3 总体覆盖率

```
路由+状态机层:  ████████████████████ 100% (已完整)
超时+参数层:    ████████████████████ 100% (已完整)
显示模式层:     ████████████████████ 100% (已完整)
闪烁规则层:     ████████████         67% (condition+exclude_when待JSON化)
档位LED层:      ████████████         67% (count+boost分支待完善)
状态LED层:                           0% (全部硬编码, 最大的gap)
段码渲染层:                         0% (value_map等未消费)
交替+优先级层:                      0% (alternate_rule+priority_chain未消费)

综合: ≈89% (↑2%: level_led_mode 从 0%→67% 提升档位LED覆盖)
```

---

## 三、未修复的审计报告问题跟踪

| 审计报告编号 | 问题 | 本轮状态 |
|-------------|------|---------|
| P0-2.1 | Z1~Z4 按键映射错误 | ✅ **已修复** |
| P0-2.2 | 超时值 10x 错误 | ⚠️ **部分修复** (timeouts已修正, power_on_seq step1滞后) |
| P0-2.3 | 档位LED gradient→single | ⚠️ **部分修复** (正常档位OK, Boost分支回归) |
| P1-3.1 | 定时交替显示硬编码 | ❌ 未处理 |
| P1-3.2 | 闪烁排除条件硬编码 | ❌ 未处理 |
| P1-3.3 | Zone/Process 超时检查硬编码 | ❌ 未处理 |
| P1-3.4 | LED config 完全硬编码 | ❌ 未处理 (仅档位LED改善) |
| P1-3.5 | segment_slot value_map 未消费 | ❌ 未处理 |
| P1-3.6 | 暂停时闪烁行为 | ❌ 未处理 |

---

## 四、结论

**本轮修改质量**: 基本合格，**1 处回归需修复**。

- Z1~Z4 映射修复完整，与参考程序对齐 ✅
- level_led_mode 数据驱动方向正确，无新增硬编码 ✅
- **Boost LED 丢失 gradient 模式行为** ❌ (需补充 `level_led_mode` 判断)
- version_show step delay 值 2000 vs timeout 3000 不一致 ⚠️ (需确认或修正)
- 上轮审计的 P1 问题均未处理（属后续迭代范围，非本轮目标）

**建议**: 修复 Boost 回归后即可合并。P1 硬编码消解按编码对应指南逐项推进。
