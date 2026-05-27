# HMI 状态模型 V6.0 — 双层架构

> 技术负责人与 AI 对齐后的最终模型。代码重构以此为准。

## 一、全局模式：A/B 双线

### A 线 — 公共不可操控

炉头不接受任何操作。仅响应长按开关（开机/唤醒）。

```
POWERING_UP(3s) → VERSION_SHOW(3s) → POWERED_OFF ←→ DEEP_SLEEP
                                          ↑              ↑
                                    全idle后30s     关机后30s无操作
```

### B 线 — 可操控

```
POWERED_OFF 长按开关1.5s → WORKING ←→ PAUSED(冻结)
```

- WORKING 下各炉头独立运行状态机
- PAUSED = 全局紧急停止，功率置零，状态机/定时器全部冻结不动，恢复后原样继续
- CHILD_LOCK = 按键过滤器，不改变模式

## 二、热点炉头队列（机制一）

### 数据结构
- `hotHead` = 当前操作目标，LED 永远跟它，优先级最高
- `selectStack` = 已确认非零功率炉头的 LIFO 栈（max 4）

### 规则
1. 按炉头N键 → 旧hotHead自动确认（>0压栈/=0移除），N成为新hotHead
2. 选中态按档位键 → 只更新power_level，保持selecting，不压栈
3. 15s超时确认 → power>0→cooking+压栈 / power=0→idle+移除
4. 四头全idle → 启动30s计时 → POWERED_OFF（不是立即关机）
5. LED永远跟hotHead；hotHead退出后跟栈顶；栈空全灭

## 三、计时器统一归零（机制二）

### 五个计时器
| 计时器 | 时长 | 触发后果 |
|--------|------|----------|
| 选中计时 | 15s | 确认选中 |
| 定时设置计时 | 5s | 自动确认定时 |
| 全空闲计时 | 30s | → POWERED_OFF |
| 关机计时 | 30s | POWERED_OFF → DEEP_SLEEP |
| Boost计时 | 5min | Boost → 原档位 |

### 清零规则
任何有效按键 → **全部五个计时器同时归零**。
无效按键（关机态误按、童锁拦截、A类模式等）不归零。

## 四、代码重构要点

### 当前代码的主要偏差
1. `globalState.mode = 'standing_by'` → 应改为 `'working'`
2. `goStandingBy()` → 应删除，改为 `goWorking()` 初始化四头idle
3. `checkAllIdle()` 直接调 `goStandingBy()` → 应改为启动30s计时器
4. `togglePause()` 改了 seg_chars → 应只设冻结标志，不改任何状态
5. 休眠检测在 STANDING_BY 做 → 应改为 POWERED_OFF 30s
6. 全空闲直接→待机 → 应改为全空闲30s→POWERED_OFF
7. `standbySince` → 应拆分为 `idleSince`（全空闲计时）和 `offSince`（关机计时）
8. 计时器清零不统一 → 应在有效按键入口统一调用 `resetAllTimers()`

### 新增数据结构
```javascript
var timers = {
    select: 0,      // 选中15s
    timer_set: 0,   // 定时5s
    idle: 0,        // 全空闲30s → POWERED_OFF
    off: 0,         // 关机30s → DEEP_SLEEP
    boost: 0        // Boost 5min (存炉头内也可)
};
```

### 新函数
```javascript
function resetAllTimers() {
    timers.select = 0;
    timers.timer_set = 0;
    timers.idle = 0;
    timers.off = 0;
    // boost 存各炉头内
}
```

### UI 面板拆分
测试界面状态面板拆为两部分：
- **一、全局状态**：当前模式(A线/B线)、暂停、童锁、热点炉头、选择序列
- **二、炉头独立工作模式**：每炉头状态/档位/定时/Boost 表格

## 五、讨论记录

- 2026-05-22：用户指出选中后 global_mode 仍为 standing_by 导致30s休眠误触发
- 2026-05-22：用户提出 A/B 双线模型，WORKING 替代 STANDING_BY
- 2026-05-22：用户明确 PAUSED 是全局紧急停止，冻结不改状态
- 2026-05-22：用户明确全空闲30s→POWERED_OFF（不是立即），再30s→DEEP_SLEEP
- 2026-05-22：用户明确计时器统一归零规则：任何有效按键→全部计时器归零
