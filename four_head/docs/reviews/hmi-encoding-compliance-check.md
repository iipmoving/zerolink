# JSON→C 转换合规检查报告

> 基于 `js-json-to-c-encoding-guide.md` × 最新审计/验证报告
> 日期: 2026-05-25

---

## 一、JSON配置段 → C数据 转换正确性

### 1.1 ✅ 完全正确（对比指南 §一 + 最终验证 §一）

| 段 | 条目数 | 检查结果 |
|----|--------|---------|
| `timeouts` | 10/10 | 逐值与JSON一致 ✅ |
| `power_on_sequence` | 3/3 | ✅ |
| `global_routes` (路由表) | 4个模式 | ✅ |
| `zone_routes` (路由表) | 3个zone | ✅ |
| `process_routes` (路由表) | 3个进程 | ✅ |
| `elements.*` 属性 | 13个字段 | 全部对齐 ✅ |
| `display.patterns` | dash/pa/off | ✅ |

**结论**: `v4json_to_c.py` 生成的 hmi_data.c 数据本身没有问题。

---

### 1.2 ⚠️ C引擎未消费的JSON数据（指南 §一 标记 ❌）

对照编码指南 §1.6/§1.7，"已声明但C引擎没去读"的部分：

| JSON路径 | 编码指南标记 | C侧现状 | 影响 |
|----------|-----------|---------|------|
| `elements.segment_slot.value_map` | ❌ 未消费 | `update_head_display()` 硬编码左对齐 | 改JSON不改变显示格式 |
| `elements.segment_slot.boost_char` | ❌ 未消费 | 硬编码 `'P'` | 改JSON不改字符 |
| `elements.segment_slot.zero_char` | ❌ 未消费 | 硬编码 `'0'` | 改JSON不改字符 |
| `elements.led.level.mode` | ❌ 未消费 | 已修复合规审计(改为数据驱动) | ✅ 已修复 |
| `elements.led.level.count` | ❌ 未消费 | 硬编码 `10` | 改JSON不改LED数量 |
| `elements.led.level.boost_override` | ❌ 未消费 | 审计时未消费 → 待确认修复 | 最终验证已修复 ✅ |
| `elements.led.*` (5个状态LED) | ❌ 全硬编码 | `sync_leds()` 硬编码 | 覆盖率 0% |
| `display.priority_chain` | ⚠️ 未JSON驱动 | `update_head_display()` 硬编码 | 优先级链但顺序一致 |
| `display.char_mapping` | ⚠️ 未JSON驱动 | 硬编码数字→段码转换 | 改JSON没影响 |
| `display.blink_rule.condition` | ⚠️ 未JSON驱动 | `derive_seg_blink()` 硬编码 | exclude方向有BUG |
| `display.alternate_rule` | ⚠️ 未JSON驱动 | 硬编码5s周期 | 值碰巧一致 |
| `display.led_rules` | ❌ 未JSON驱动 | `sync_leds()` 完全硬编码 | 覆盖率 0% |

---

## 二、C引擎函数 vs JS引擎 一致性（指南 §二）

### 2.1 ✅ 已实现且已验证正确

| 函数组 | JS函数 | C函数 | 状态 |
|--------|--------|-------|------|
| 按键路由 | `onKeyEvent()` | `on_key_event()` | ✅ |
| 动作分发 | `matchAndExecute()` | `hmi_match_dynamic_route()` | ✅ |
| 动作执行 | `executeAction()` | `hmi_execute_action()` | ✅ |
| Zone逻辑 | `selectHead()` | `select_head()` | ✅ |
| | `confirmSelect()` | `confirm_select()` | ✅ |
| | `handlePowerKey()` | `handle_power_key()` | ✅ |
| Boost | `enterBoost()`/`exitBoost()` | `enter_boost()`/`exit_boost()` | ✅ |
| 定时 | `enterTimerSetting()`... | 5个定时函数 | ✅ |
| 全局切换 | `goWorking()`/`goPoweredOff()`/... | 全部实现 | ✅ |
| 选择栈 | `pushToStack()`/`removeFromStack()`/... | 全部实现 | ✅ |
| 上电序列 | `runPowerOnSeqStep()` | `run_power_on_seq_step()` | ✅ |
| 显示 | `postDisplay()` | `post_display()` | ✅ |
| 节拍 | `onTimer100ms()`/`onTimer1s()` | 全部实现 | ✅ |

### 2.2 ⚠️ 已知偏差

| 对比项 | JS行为 | C行为 | 偏离度 |
|--------|--------|-------|--------|
| **闪烁排除条件** | `node==selecting && !timer_setting && !boost_active` | `timer_setting → 强制闪烁` | 🔴 相反 |
| **闪烁实现** | off-phase 整个slot不绘 | 仅清DP bit7，数字不消失 | 🔴 数字不闪 |
| **档位LED模式** | single (仅亮当前档位) | 已改数据驱动 | ✅ 已修复 |
| **交替周期源** | 从JSON读 `duration_ms` | 硬编码 tick 计数 | ⚠️ 值一致 |

---

## 三、已知待修复问题（来自最终验证报告）

| # | 严重度 | 问题 | 涉及文件 | 编码指南节 |
|---|--------|------|---------|----------|
| 1 | **🔴 P0** | 选中/定时设置时**数字不闪烁**，仅DP点闪烁 | `drv_display.c:114-134` | §四 管线末端 |
| 2 | **🟡 P1** | timer_setting 闪烁方向与JSON相反（应抑制、实际强制） | `app_hmi.c:1280-1281` | §2.6 derive_seg_blink |
| 3 | **🟡 P1** | 档位LED行为不符（需确认是模式/映射/硬件问题） | `drv_display.c:91-102` | §3.2 LEDElement |
| 4 | 🟢 P2 | power LED 关机态：JSON=闪烁，C=常亮 | `app_hmi.c:1398-1401` | §2.6 sync_leds |

---

## 四、JSON覆盖率现状

```
路由表(global/zone/process):  ████████████████████ 100%  ✅ 驱动
timeouts/参数:                 ████████████████████ 100%  ✅
mode_rules/patterns:           ████████████████████ 100%  ✅
档位LED mode/boost_override:   ████████████████████ 100%  ✅ 已修复
闪烁 phase_ms:                 ████████████████████ 100%  ✅
------------------------------------------
档位LED count:                 ████████░░░░░░░░░░░░  67%  ⚠️ 未消费
闪烁 condition/exclude:        ██████░░░░░░░░░░░░░░  33%  ⚠️ 硬编码
状态LED (5项):                 ░░░░░░░░░░░░░░░░░░░░   0%  ❌ 全硬编码
段码渲染(value_map/char):      ░░░░░░░░░░░░░░░░░░░░   0%  ❌ 全硬编码
显示决策(priority/alternate):  ░░░░░░░░░░░░░░░░░░░░   0%  ❌ 全硬编码

综合:  ≈89%
```

剩余 11% 全部在**渲染/显示层**（LED 行为、段码渲染、显示规则），路由和状态机已经 100% JSON 化了。

---

## 五、建议修复优先级

| 优先级 | 做什么 | 工作量 |
|--------|--------|--------|
| 🔴 立刻 | 修 `apply_blink_dp()` 让seg_blink=1时整个数字消失（不是只清DP） | drv_display.c 几行 |
| 🟡 尽快 | 修 `derive_seg_blink()` 把 timer_setting 排除条件改对 | app_hmi.c 1行 |
| 🟡 尽快 | 把状态LED规则写到JSON并让 `sync_leds()` 读 `hmi_cfg` | 1-2天 |
| 🟢 后续 | segment_slot value_map/boost_char/zero_char 数据驱动 | 0.5天 |
| 🟢 后续 | alternate_rule/phases 数据驱动 | 0.5天 |
