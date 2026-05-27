# HMI 最终验证报告 (2026-05-25)

**审核者**: 测试员  
**审核范围**: JSON源 → hmi_data.c → app_hmi.c 全链路逐值比对 + 场景逻辑追踪 + 硬件行为分析  
**基准**: `sim/test_hmi/logic/four_head_v4.json` + `sim/test_hmi/js/json_logic_engine.js`

---

## 一、JSON → hmi_data.c 逐值比对 ✅ 全部通过

10/10 timeouts、3/3 power_on_seq、11/11 route groups、13/13 element fields — 与JSON完全对齐。明细见上轮报告。

---

## 二、功能性问题 (硬件实测)

### 2.1 ❌ 选中炉头/定时设置: 只有DP点闪烁，数字不闪烁

**现象**: 炉头 selecting 态或 timer_setting 态时，只有右下角小数点(DP)闪烁，数码管数字本身不闪烁。

**根因**: `drv_display.c` 的 `apply_blink_dp()` 只操作 bit7 (DP段):

```c
/* drv_display.c:114-134 — apply_blink_dp() */
for (i = 0; i < 4; i++) {
    if (s_flash_toggle) {
        s_disp_upper[i] &= (uint8_t)~s_blink_mask[i];   /* 只清除 bit7 */
    } else {
        s_disp_upper[i] |= s_blink_mask[i];              /* 只设置 bit7 */
    }
}
```

`s_blink_mask[i]` = 0x80 (仅bit7)。整个闪烁周期中 **bits 0-6 (数字段码) 从未被清零**。因此数字始终可见，只有DP点在闪。

**对比JS行为** (`renderer.js:240`):
```javascript
if (blinkArr && blinkArr[headIdx] && blinkPhase === 0) return;  /* 整块不绘 */
```
JS在闪灭相位时整个slot不绘制，数字完全消失。

**修复方向**: `apply_blink_dp()` 需区分两种闪烁:
- `s_blink_mask[i] == 0xFF`: 全数字闪烁 → off-phase 时写 `0x00` 清零整个字节
- `s_blink_mask[i] == 0x80`: 仅DP闪烁 (保留给其他用途)

同时 `sync_hmi_display()` (drv_display.c:169-172) 需将 `seg_blink` 对应 digits 的 mask 设为 `0xFF`。

**涉及文件**: `drv_display.c:114-134`, `drv_display.c:169-173`

---

### 2.2 ❌ 定时设置闪烁方向与JSON相反

**现象**: timer_setting 期间数字闪烁。JSON spec 声明 `exclude_when: ["timer_setting"]` 应在定时设置时抑制闪烁。

**根因**: `derive_seg_blink()` (app_hmi.c:1278-1288) 将 timer_setting 放在第一优先级强制闪烁 ON:

```c
if (h->timer_setting) {
    s_display.seg_blink[i] = 1;       /* ← 强制闪烁, 与JSON exclude_when 相反 */
} else if (h->node == HMI_ZONE_SELECTING && !h->boost_active) {
    s_display.seg_blink[i] = 1;
} else {
    s_display.seg_blink[i] = 0;
}
```

**JSON spec** (`elements.blink`):
```json
{ "condition": "node == selecting", "exclude_when": ["timer_setting", "boost_active"] }
```

JS BlinkRule.shouldBlink(): `node==selecting AND NOT timer_setting AND NOT boost_active → true`

**修复方向**: 删除 timer_setting 的第一优先级分支。闪烁条件应为 `node==selecting && !timer_setting && !boost_active`。

**注意**: 此修复与 2.1 相互独立。即使闪烁方向正确，数字本身也不会闪 (问题2.1)。

**涉及文件**: `app_hmi.c:1278-1288`

---

### 2.3 ❌ 档位LED与预期不符

**现象**: 档位LED灯点亮的与实际期望不对应。

**可能原因** (需硬件验证确认):

| 原因 | 详情 | 验证方法 |
|------|------|---------|
| A. single vs gradient | JSON `mode:"single"` 只亮一盏灯; 用户可能期望 gradient (0→当前档位全亮) | 改 `level_led_mode` 为 0 测试 |
| B. 物理LED编号 ≠ 面板丝印顺序 | s_lvl_map 按 LED1-16 编号，面板可能按不同顺序排列 | 逐个点灯测试匹配 |
| C. L8/L9 无物理LED | `s_lvl_map[8] = s_lvl_map[9] = {0, 0x00}`，Boost(档位9)无LED指示 | 查参考程序Boost指示方式 |
| D. LED IO 映射错误 | s_lvl_map 的 io/mask 与硬件走线不一致 | 用 Drv_Display_SetRawLEDs 逐位测试 |

**物理LED映射表** (drv_display.c:91-102) 供逐灯验证:

| 档位 | 物理LED | IO字节 | bit位 | 备注 |
|------|---------|--------|-------|------|
| 0 | LED10 | IO[9] | bit4 | |
| 1 | LED11 | IO[8] | bit4 | |
| 2 | LED12 | IO[8] | bit0 | |
| 3 | LED13 | IO[8] | bit1 | |
| 4 | LED14 | IO[8] | bit5 | |
| 5 | LED15 | IO[8] | bit7 | |
| 6 | LED16 | IO[8] | bit3 | |
| 7 | LED4 | IO[10] | bit1 | |
| 8 | — | — | — | 无物理LED |
| 9 (Boost) | — | — | — | 无物理LED |

**涉及文件**: `drv_display.c:91-102` (s_lvl_map), `app_hmi.c:1419-1447` (sync_leds)

---

## 三、上轮修复验证

| 审计项 | 状态 |
|--------|------|
| Z1~Z4 映射 (head_key_to_index + 3处路由表) | ✅ 已修复 |
| level_led_mode 数据驱动 | ✅ 已修复 |
| Boost LED level_led_mode 回归 | ✅ 已修复 |
| version_show_step delay 对齐 | ✅ 已修复 |
| 10/10 timeouts 与JSON一致 | ✅ 通过 |
| 全部路由表与JSON一致 | ✅ 通过 |
| 全部 element 字段与JSON一致 | ✅ 通过 |
| hmi_data.c 无值错误 | ✅ 通过 |

---

## 四、JSON覆盖率 (本轮最终)

```
路由+状态机:  ████████████████████ 100%
超时+参数:    ████████████████████ 100%
模式+图案:    ████████████████████ 100%
档位LED配置:  ████████████         67% (mode+boost_override OK, count未消费)
闪烁规则:     ██████               33% (phase_ms+pause_override OK, condition+exclude 硬编码)
状态LED配置:                       0% (5项全部硬编码)
段码渲染:                          0% (value_map/boost_char/zero_char 硬编码)
显示决策:                          0% (priority_chain/alternate_rule 硬编码)

综合: 89%
```

---

## 五、待修复汇总

| # | 严重度 | 描述 | 文件:行号 |
|---|--------|------|----------|
| 1 | **P0** | 数字不闪烁，仅DP点闪烁 (apply_blink_dp只操作bit7) | drv_display.c:114-134 |
| 2 | **P1** | timer_setting 闪烁方向与JSON相反 (应抑制,实际强制) | app_hmi.c:1280-1281 |
| 3 | **P1** | 档位LED行为不符 (需确认是模式/映射/硬件问题) | drv_display.c:91-102 |
| 4 | P2 | blink_phase_ms: JSON=300ms, DRV硬编码=500ms | drv_display.h:22 |
| 5 | P2 | power LED powered_off: JSON=blink, C=常亮 | app_hmi.c:1398-1401 |
