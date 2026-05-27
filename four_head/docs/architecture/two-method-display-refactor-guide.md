# 数码管显示双方法重构指引

> "数码管有两个独立方法：ShowASCII 负责普通内容，ShowEffect 负责特效类型+参数"
>
> 日期: 2026-05-25 | 状态: 待实施

---

## 一、问题诊断

当前 `seg_mode` 把"内容是什么"和"怎么显示"混在了一起：

```c
// app_hmi.h:113-120 — 当前枚举
HMI_SEG_MODE_POWER           // "功率显示" —— 实际是: 内容=档位数字, 特效=无
HMI_SEG_MODE_TIMER_SETTING   // "定时设置" —— 实际是: 内容=时间数字, 特效=闪烁
HMI_SEG_MODE_ASCII           // "暂停"     —— 实际是: 内容="PA",     特效=无
HMI_SEG_MODE_DASH            // "关机"     —— 实际是: 内容="--",     特效=无
HMI_SEG_MODE_OFF             // "全灭"     —— 实际是: 内容=' ',     特效=无
```

想加跑马灯就没地方塞——得新增 `HMI_SEG_MODE_MARQUEE`，而"跑马灯跑什么内容"和"跑马灯这个特效本身"是两个概念。

---

## 二、目标模型

```
显示 = ShowASCII(content) ⊕ ShowEffect(type, params)

内容层:  seg_chars[8] = "5", "03", "PA", "HOT"
         ↑ 始终存在，APP 只管这个

特效层:  seg_effect = NONE | BLINK | MARQUEE | FANGUN | FADE
         ↑ 可叠加，DRV 执行，不影响内容赋值
```

**关键认知**：跑马灯是一种特效（和闪烁同级），不是一种内容。任何内容都可以叠加任何特效：

| 内容 | 特效 | 显示结果 |
|------|------|---------|
| "HOT" | NONE | "HOT" 常亮 |
| "HOT" | BLINK | "HOT" 闪烁 |
| "HOT" | MARQUEE | "HOT" 跑马灯 |
| "5 " | NONE | "5" 常亮 |
| "5 " | BLINK | "5" 闪烁（选中） |

---

## 三、修改清单

### 阶段 0：概念迁移（不改行为，建立正交结构）

#### F001 — `app_hmi.h`：新增 `HmiSegEffect_t` 枚举

```c
/* 显示特效类型 —— 与内容正交 */
typedef enum {
    HMI_SEG_EFFECT_NONE = 0,    /* 无特效，直接显示 seg_chars */
    HMI_SEG_EFFECT_BLINK,       /* 闪烁（原 seg_blink[] 的作用） */
    HMI_SEG_EFFECT_MARQUEE,     /* 跑马灯（预留） */
    HMI_SEG_EFFECT_COUNT
} HmiSegEffect_t;
```

#### F002 — `app_hmi.h`：`HmiDisplayCache_t` 新增字段

```c
typedef struct {
    char    seg_chars[8];          /* 不变 */
    uint8_t seg_blink[4];          /* 保留，作为 BLINK 特效的 per-zone 参数 */
    HmiSegEffect_t seg_effect;    /* 新增 */
    uint8_t seg_mode;              /* 保留，暂时不变 */
    /* ... 其余字段不变 ... */
} HmiDisplayCache_t;
```

`seg_blink[4]` 暂时保留——它是 `effect=BLINK` 时的参数（哪些 Zone 闪）。后续可以收敛为 `effect_params` 联合体，但第一阶段不拆这个。

#### F003 — `app_hmi.h`：新增两个动作

```c
/* HmiAction_t 新增 */
HMI_ACT_SHOW_MARQUEE,        /* 启动跑马灯特效, param=速度档位 */
HMI_ACT_STOP_MARQUEE,        /* 停止跑马灯特效 */
```

#### F004 — `cfg/hmi_data.c`：JSON 对应字段映射

```json
// 在 elements.display_patterns 或新建 elements.effects:
"effects": {
    "marquee": {
        "speed_ms": 200,
        "loop": true
    },
    "blink": {
        "phase_ms": 300
    }
}
```

并在 `HmiElementCfg_t` 或配置表中增加对应字段。

---

### 阶段 1：DRV 层建立特效引擎

#### F101 — `drv_display.h`：新增特效接口

```c
/* 特效引擎 —— DRV 层拥有定时域和显示缓冲 */
void Drv_Display_Effect_Set(HmiSegEffect_t effect, const void *params);
void Drv_Display_Effect_Stop(void);
void Drv_Display_Effect_Tick(void);   /* 每 10ms 调用，推进帧 */
```

#### F102 — `drv_display.c`：特效状态机（核心新增）

```c
typedef struct {
    HmiSegEffect_t active;     /* 当前特效 */
    uint16_t       frame;      /* 帧计数器 */
    uint16_t       speed_ms;   /* 帧间隔 */
    union {
        struct { uint8_t blink_mask[4]; } blink;
        struct { uint8_t shift; uint8_t loop; } marquee;
    } cfg;
} EffectState_t;

static EffectState_t s_effect;

/* ShowASCII: 直接刷新内容到 s_io_work[] */
static void show_ascii(const char seg_chars[8])
{
    /* 已有逻辑: HAL_SMG_FontASCII → s_io_work[] */
}

/* ShowEffect: 特效状态机覆盖 s_io_work[] */
static void show_effect(void)
{
    switch (s_effect.active) {
    case HMI_SEG_EFFECT_NONE:
        break;  /* 不干预，ShowASCII 的结果直接使用 */
    case HMI_SEG_EFFECT_BLINK:
        if (s_effect.frame % 2 == 0) {
            /* 清掉闪烁 Zone 的段码 */
            apply_blink_off(&s_effect.cfg.blink);
        }
        break;
    case HMI_SEG_EFFECT_MARQUEE:
        /* 帧推进：移位 seg_chars → s_io_work[] */
        render_marquee_frame(s_effect.frame);
        break;
    }
}
```

#### F103 — `drv_display.c`：修改 `Drv_Display_Update()`

```c
void Drv_Display_Update(void)
{
    /* ... 现有 100ms 刷新逻辑 ... */

    /* 特效帧推进（每 10ms） */
    if (s_effect.active != HMI_SEG_EFFECT_NONE) {
        s_effect.frame++;
        show_effect();          /* 特效覆盖 s_io_work[] */
    } else {
        show_ascii(s_disp_upper, s_disp_lower);  /* 普通路径 */
    }

    s_io_dirty = 1;
}
```

**关键**：`show_ascii()` 始终先写入内容，`show_effect()` 在其上叠加变形。内容层不知道有没有特效。

---

### 阶段 2：APP 层使用新模型

#### F201 — `app_hmi.c`：`post_display()` 改造

```c
static void post_display(void)
{
    /* 1. 始终决定内容（不变） */
    update_all_displays(s_display.seg_chars);

    /* 2. 决定是否需要特效（替换原来的 mode→effect 隐式映射） */
    derive_seg_effect(&s_display);   /* 新函数 */
    derive_seg_blink(&s_display);    /* 保留，作为 BLINK 的 per-zone 参数 */

    Msg_Post(MSG_DISPLAY_REFRESH, 0, &s_display);
}
```

#### F202 — `app_hmi.c`：`derive_seg_effect()` 新函数

```c
static void derive_seg_effect(HmiDisplayCache_t *d)
{
    /* 优先级链（和 ModeRule 同模式） */
    if (isPoweredOff())       d->seg_effect = HMI_SEG_EFFECT_NONE;  /* "--" 常亮 */
    else if (isDeepSleep())   d->seg_effect = HMI_SEG_EFFECT_NONE;  /* 全灭 */
    else if (anySelecting())  d->seg_effect = HMI_SEG_EFFECT_BLINK; /* 选中=闪烁 */
    else if (isPaused())      d->seg_effect = HMI_SEG_EFFECT_NONE;  /* "PA" 常亮 */
    else                      d->seg_effect = HMI_SEG_EFFECT_NONE;  /* 正常 */
}
```

#### F203 — `app_hmi.c`：`on_hmi_display_refresh` 回调改造

```c
static void on_hmi_display_refresh(MsgId_t id, uint16_t param, void *data_ptr)
{
    HmiDisplayCache_t *d = (HmiDisplayCache_t *)data_ptr;

    /* 1. 内容始终刷新 */
    sync_hmi_display(d);    /* seg_chars → s_disp_upper/lower */

    /* 2. 启动/停止特效 */
    Drv_Display_Effect_Set(d->seg_effect, d);
}
```

---

## 四、JSON 方法论演进

### 三层架构更新

三层架构不变，但 Presentation Controls 内部新增正交维度：

```
┌─ Zone Logic (引擎内置) ─────────────────────────────┐
│  selectHead / handlePowerKey / enterBoost           │
└─────────────────────────────────────────────────────┘
                         │ 读 JSON 参数
┌─ JSON Binding (声明式绑定) ─────────────────────────┐
│  ModeRule / BlinkRule / EffectRule / 路由表         │
│  → 从状态派生 seg_chars(内容) + seg_effect(特效)    │
└─────────────────────────────────────────────────────┘
                         │ 写入 displayCache
┌─ Presentation Controls (可配置元素) ─────────────────┐
│  ┌─ 内容方法 ─┐  ┌─ 特效方法 ──────────────────────┐ │
│  │ ShowASCII  │  │ ShowEffect(type, params)        │ │
│  │ SlotElement│  │  ├ BLINK: apply_blink_off()     │ │
│  │ value_map  │  │  ├ MARQUEE: render_frame()     │ │
│  │ seg_chars  │  │  └ FANGUN: ...(未来)           │ │
│  └────────────┘  └────────────────────────────────┘ │
│       ↓                        ↓                     │
│  s_io_work[] ←─────────────── 覆盖                   │
└─────────────────────────────────────────────────────┘
```

### JSON elements 扩展

```json
{
  "elements": {
    "segment_slot": { /* 内容属性: value_map, boost_char, zero_char */ },
    "effects": {
      "blink":    { "phase_ms": 300 },
      "marquee":  { "speed_ms": 200, "loop": true },
      "fangun":   { "speed_ms": 150, "direction": "left" }
    },
    "mode": { /* ModeRule 不变, 负责派生 effect 类型 */ }
  }
}
```

### EffectRule：ModeRule 的自然延伸

当前的 `ModeRule` 从状态派生 `seg_mode`（POWER/DASH/OFF/...）。改造后：

```
ModeRule 拆分:
  1. ContentRule: 状态 → seg_chars（内容）
  2. EffectRule:  状态 → seg_effect（特效类型 + 参数）

二者独立求值，不改对方的数据。
```

实际上连"闪烁选中"这种最简单的特效，也是 EffectRule 派生的——`seg_blink[4]` 只是 `effect=BLINK` 的 per-zone 参数，不是独立的显示决策。

---

## 五、实施顺序

| 序号 | 文件 | 内容 | 测试标准 |
|------|------|------|----------|
| F001 | `app_hmi.h` | 新增 `HmiSegEffect_t` 枚举 + `HmiDisplayCache_t` 加字段 | 编译通过 |
| F002 | `app_hmi.c` | `derive_seg_effect()` 新函数 | 行为不变：现有闪烁逻辑改为走 effect=BLINK |
| F003 | `drv_display.h` | 新增 `Drv_Display_Effect_*` 接口 | 声明清晰 |
| F004 | `drv_display.c` | `EffectState_t` + `show_effect()` + blink 从 APP 迁移到 DRV | 现有闪烁行为不变 |
| F005 | `drv_display.c` | 修改 `Drv_Display_Update()` 走 show_ascii / show_effect 分叉 | 34/34 测试全部通过 |
| F006 | `cfg/hmi_data.c` | JSON effects 段映射到 `hmi_elements` | 编译通过, 参数可读 |
| F007 | `drv_display.c` | 实现第一个新特效：跑马灯 `render_marquee_frame()` | 触发 marquee 后可见滚动 |
| F008 | `app_hmi.c` | `SHOW_MARQUEE` / `STOP_MARQUEE` 动作钩子 | 按键触发跑马灯 |

F001-F005 是架构准备，不改任何可见行为。F006-F008 是新特效落地。

---

## 六、为什么不是"新建文件"

1. `HmiSegEffect_t` 放在 `app_hmi.h` —— 和 `HmiSegMode_t` 同级，调用方一次 include
2. `EffectState_t` 放在 `drv_display.c` —— 纯内部状态，不给外部直接访问
3. JSON 特效属性放在 `elements.effects` —— 和 `elements.led`、`elements.segment_slot` 同级

新增文件会增加 include 依赖和概念碎片，现有文件边界清晰——HMI 概念在 `app_hmi.h`，显示执行在 `drv_display.c`，不改。

---

## 七、LED 梯度显示：LevelLED JSON `mode` 字段

### 原则

```
逻辑层只输出: level = 5
LevelLED 根据自身属性决定呈现:
  mode = "single"   → 第 5 颗 LED 亮
  mode = "gradient" → 第 0~5 颗 LED 亮 (柱状条)
```

### JSON 属性

```json
"elements": {
  "led": {
    "level": {
      "count": 10,
      "mode": "gradient",        // 新增
      "boost_override": { ... }
    }
  }
}
```

### C 端落地

`hmi_data.c` 的 `HmiElementCfg_t` 加字段 `uint8_t level_led_mode`，枚举 `0=single, 1=gradient`。`levelLED.setLevel(n)` 内部读 `mode` 决定点灯策略。Zone Logic 永远只传 level 值。

### 与双方法模型的关系

LED 和数码管是同一个原则的两类呈现：

| | 内容 (逻辑层输出) | 呈现 (元素自己决定) |
|---|---|---|
| 数码管 | `seg_chars[8]` = "5", "03", "HOT" | `value_map` 查表, `effect` 特效 |
| LED | `level = 5` | `mode=single/gradient`, `boost_override` |

逻辑层不知道也不关心 LED 怎么亮——那是 LevelLED 的属性决定的。

---

## 八、长按阈值统一到 timeouts

### JSON 扩展

```json
"timeouts": {
  "select_confirm_ms": 15000,
  "deep_sleep_ms": 30000,
  "long_press_ms": 1500,          // 新增
  "key_debounce_ms": 50           // 新增（驱动层已实现，此处仅声明）
}
```

### 键码编码（已验证方案）

键码最高位标记状态：`0x80 | code = 按下`，`0x7F & code = 键码`。驱动层硬件去抖后直接投递 `MSG_KEY_EVENT`，长按判定是 Zone Logic 用 `long_press_ms` 做的——驱动层不参与长按逻辑。

---

## 九、消息总线对接规范

### 规则

**规则引擎是消息发布者，显示元素是消息订阅者。引擎不直接调 `Drv_Display_*`，元素不直接读 `heads[]`。**

```
APP (发布者)                           DRV (订阅者)
───────────                           ───────────
postDisplay():
  derive seg_chars, seg_effect
  Msg_Post(DISPLAY_REFRESH, cache)  ──→  on_display_refresh()
                                            ├→ ShowASCII(d.seg_chars)   → s_io_work[]
                                            └→ ShowEffect(d.seg_effect) → s_io_work[]

DRV 初始化时注册回调:
  API_Display_RegisterCommitCallback(Drv_Display_Commit)
                                            ↑
  装配器 1ms 调用此回调传 DisplayFrame_t ───┘
```

**显示下传路径**：APP 业务 → 装配器(Set* 设抽象值) → 回调(传 DisplayFrame_t) → DRV(IO 映射)。全程无直接 `#include` 依赖。

### 反模式（禁止）

```c
/* 错误: APP 直接调 DRV */
postDisplay() {
    Drv_Display_ShowASCII(...);   // 不允许：绕过了回调机制
}

/* 错误: 元素直接读引擎状态 */
void on_display_refresh() {
    level = heads[2].power_level;  // 不允许：应通过 displayCache 传入
}

/* 正确: 通过装配器 + 回调 */
void on_display_refresh(msg) {
    API_Display_SetZoneDigit(0, msg->power_level);   // 设抽象值给装配器
    API_Display_Assemble(&frame);                     // 装配器组装
    // 装配器内部调用注册的回调 → Drv_Display_Commit(&frame)
}
```

---

## 十、转换工具更新（gen_hmi.js）

JSON 规则稳定后，`Claude/cfg/gen_hmi.js` 需同步以下新增字段：

| JSON 路径 | C 落点 | 映射规则 |
|-----------|--------|---------|
| `timeouts.long_press_ms` | `hmi_timeouts.long_press_ms` | 直接数值 |
| `timeouts.key_debounce_ms` | `hmi_timeouts.key_debounce_ms` | 直接数值 |
| `elements.led.level.mode` | `hmi_elements.level_led_mode` | `"single"→0, "gradient"→1` |
| `elements.effects.blink.phase_ms` | `hmi_effects.blink_phase_ms` | 直接数值 |
| `elements.effects.marquee.speed_ms` | `hmi_effects.marquee_speed_ms` | 直接数值 |
| `elements.effects.marquee.loop` | `hmi_effects.marquee_loop` | `true→1, false→0` |

转换工具更新应在 JSON 规则评审通过后、C 端实现前完成。
