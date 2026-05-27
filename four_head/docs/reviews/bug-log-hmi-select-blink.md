# HMI 测试错误记录：上电00 + 全屏闪烁

> 发现日期：2026-05-22
> 发现者：技术负责人（交互测试）
> 修复者：AI 程序员

## 错误 A：上电过程显示 "00"

### 现象

上电全显序列（8888 → V1.0/P1.0）执行后，数码管显示 "0 0 0 0" 而非预期的 "-- -- -- --" 闪烁。

### 根因

`onTimer1s()` 中的 `checkAllIdle()` 在上电序列期间被定时器触发。此时所有炉头为 idle/0，`checkAllIdle()` 返回 true，调用 `goStandingBy()`，将显示从全显 "8888" 覆写为 "0 0 0 0"。

### 时序

```
init() → runPowerOnSeqStep(0): seg_chars='8888'
  → setTimeout(3s) → runPowerOnSeqStep(1): seg_chars='V1.0/P1.0'
  → setTimeout(3s) → runPowerOnSeqStep(2): powered_off, seg_chars='----'
                              ↑
          onTimer1s() 在这3秒内触发 → checkAllIdle() → goStandingBy()
          → seg_chars 变为 '0000'，打断全显序列
```

### 修复

在 `onTimer100ms()` 和 `onTimer1s()` 顶部加隔离守卫：

```javascript
if (globalState.mode === 'power_on_seq' || globalState.mode === 'version_show') return;
```

### 避免方法

1. **所有后台定时检测必须在入口处声明"适用阶段"**——用白名单检查当前全局状态，不适用时立即返回
2. **上电序列（`power_on_seq`）和版本显示（`version_show`）** 是两个特殊的全局模式，期间禁止任何自动状态切换。新增定时任务时，必须检查是否需要加入守卫
3. 建议用注释标记每个 `setInterval` / `onTimer*` 回调的"禁区模式"列表

---

## 错误 B：所有炉头区域一起闪烁

### 现象

按炉头1键后，不仅 Z1 Slot（左上2位数码管）闪烁，Z2/Z3/Z4 区域也一起闪烁。

### 根因

`displayCache.seg_blink` 是全局布尔值。`selectHead()` 中 `displayCache.seg_blink = true` 将所有炉头区域标记为闪烁。渲染器 `drawSegment()` 检查 `displayData.seg_blink` 时无法区分哪个炉头在闪烁，导致全部区域一起闪烁。

### 修复

将 `seg_blink` 从 `boolean` 改为 `boolean[4]` 数组，每个炉头独立控制：

| 状态 | seg_blink[0..3] |
|------|----------------|
| POWERED_OFF | [T, T, T, T] — 全部闪烁 |
| STANDING_BY | [F, F, F, F] |
| 选中炉头0 | [T, F, F, F] — 仅炉头0闪 |
| 炉头0 cooking + 炉头1 selecting | [F, T, F, F] |

涉及修改点：
- **引擎** (json_logic_engine.js): `createDisplayCache`, `runPowerOnSeqStep`, `confirmSelect`, `confirmTimer`, `selectHead`, `handlePowerKey`, `enterBoost`, `goStandingBy`, `goPoweredOff`, `enterDeepSleep`, `togglePause`, `postDisplay` — 全部改为数组访问
- **渲染器** (renderer.js): `drawSegment` 改为逐炉头渲染，每炉头独立检查闪烁相位
- **测试** (test_runner.js): 所有 `d.seg_blink === true/false` 改为数组下标检查

### 避免方法

1. **"每个炉头独立"贯穿设计**——任何与炉头绑定的显示属性必须声明为数组 `[4]`，不应用全局标量
2. **渲染器绘制时明确"正在为哪个炉头绘制"**——`drawSegment` 应接收炉头索引参数，而非依赖全局标志
3. 新增每炉头 UI 属性时，复查 `postDisplay()` 是否按深拷贝传递（目前用 `.slice()` 和 `JSON.parse(JSON.stringify())`）

---

## 逻辑修正 C：选中态按档位键不应立即退出选中

### 原规格不准确

原规格书 Flow-03/04 描述"设档→ZONE_COOKING/ZONE_IDLE"为即时转换，实际应保持 ZONE_SELECTING 直到 15s 超时确认。

### 修正逻辑

选中态按档位键：
1. 更新 `power_level` 值
2. **保持** `node = 'selecting'`（继续闪烁）
3. **重置** 15s 计时器（`select_time = Date.now()`）
4. 15s 超时后统一由 `confirmSelect()` 判定 → cooking 或 idle

### 避免方法

规格书撰写时，"选中→操作→确认"三阶段必须明确每一步的状态变化。任何"操作后立即进入XX态"的描述需与技术负责人确认是否应改为延迟确认。

---

## 受影响数据流

| Flow | 变更内容 |
|------|----------|
| 03 | 设0档后加 `forceSelectTimeout`，保持选中 |
| 04 | 设5档后加 `forceSelectTimeout`，保持选中 |
| 05 | 多炉头设9档后加 `forceSelectTimeout` |
| 11 | 0档弹出序列后加 `forceSelectTimeout` |
| 27 | 两炉头各设档后各加 `forceSelectTimeout` |
| 32 | setup 中加 `forceSelectTimeout` 确认头1 |

## 规格书更新

`docs/specs/hmi-data-flow-table.md` 关键规则部分新增/修订：
- **选中闪烁**：明确"仅被选中炉头闪烁"
- **选中+档位键**：明确"保持选中、重置计时"
- **上电序列隔离**：新增守卫规则
- **LED档位灯**：补充"栈空但选中态"的 fallback

---

## 错误 D：炉头选中/工作期间误入休眠

### 现象

炉头选中态闪烁中，等待 30s 后系统直接进入 DEEP_SLEEP，全显关闭。

### 根因

`onTimer100ms()` 中的休眠检测仅检查 `globalState.mode === 'standing_by'` 和距 `standbySince` 的时间差，**未检查任何炉头是否处于活跃状态**。选中/工作期间全局模式仍为 `standing_by`，休眠计时器继续走时。

### 修复

在休眠条件中增加"四头全 idle"前置检查：

```javascript
var allIdleForSleep = true;
for (var n = 0; n < 4; n++) {
    if (heads[n].node !== 'idle') { allIdleForSleep = false; break; }
}
if (allIdleForSleep && Date.now() - standbySince >= STANDBY_SLEEP_MS) {
    enterDeepSleep();
}
```

### 避免方法

**全局模式的守卫条件不能只检查全局变量**——`standing_by` 期间炉头可能处于多种活跃状态。任何依赖全局模式的后台任务（休眠、自动关机等）必须同步检查全部炉头状态。

规格书补充：
- **休眠前置条件**：明确"30s 无操作 **且** 四头全 idle"
- **切换炉头=自动确认**：按另一炉头键时，当前选中炉头立即走确认流程（功率>0→压栈+cooking，功率=0→idle）
- **全局模式与炉头操作**：明确两者独立并行，不互相覆盖
