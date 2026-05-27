# 四头电磁炉低耦合控制程序 — 架构方案

## 一、项目概况

| 项 | 值 |
|---|---|
| 项目名 | four_head_ih_cooker |
| MCU | 赛元 SC32L14T (Cortex-M0+, 48MHz, 256K Flash, 16K SRAM) |
| 编译器 | Keil MDK, ARMCC V5.06 |
| 显示 | IO直推数码管（8 SEG + 11 COM, 两个4位实体） |
| 按键 | 21通道触摸，官方触摸库 |
| 核心原则 | 业务模块之间0交叉include，消息调度器是唯一桥梁 |

---

## 二、目录结构

```
four_head_ih_cooker/Claude/
├── inc/
│   ├── msg_def.h              // 消息ID枚举、Msg_t结构体
│   ├── msg_scheduler.h         // 调度器接口
│   ├── ui_engine.h             // UI引擎接口 (JSON驱动)
│   ├── drv_display.h           // 显示驱动层接口
│   ├── drv_key.h               // 按键驱动层接口
│   ├── hal_display.h           // 显示HAL接口
│   ├── hal_key.h               // 触摸按键HAL
│   ├── hal_comm.h              // UART通讯HAL
│   ├── hal_timer.h             // 1ms定时器HAL
│   └── hal_gpio.h              // GPIO HAL
├── src/
│   ├── msg_scheduler.c         // 调度器实现
│   ├── ui_engine.c             // JSON驱动状态机引擎 (通用，不含业务规则)
│   ├── ui_actions.c            // action函数库 (JSON按名调用)
│   ├── drv_display.c           // 显示命令→IO缓冲映射
│   ├── drv_key.c               // 按键扫描+去抖+映射
│   ├── hal_display.c           // IO直推扫描 (TIM0 ISR)
│   ├── hal_key.c               // 触摸库封装
│   ├── hal_comm.c              // UART0 DMA收发
│   ├── hal_uart.c              // UART3调试输出
│   ├── hal_gpio.c              // GPIO初始化
│   ├── hal_timer.c             // TIM0 1ms时基
│   └── main.c                  // 入口+10槽调度
├── rules/
│   └── ui_rules.json           // ★ 唯一真相源: UI状态机JSON规则
├── scripts/
│   └── json_to_c.py            // JSON → C const char[] 转换脚本
├── test/
│   ├── test_framework.py       // Python虚拟UI引擎 (加载同一JSON)
│   ├── test_spec_states.py     // 规格书全状态机回归测试
│   └── ui_rules.json           // 测试用JSON (软链或复制)
└── doc/
    └── hmi_ref.md              // HMI实现参考
```

> **没有 ui_config.c** —— 不需要 C 硬编码规则表。JSON 是唯一规则源。
> C 代码只做三件事：加载 JSON、执行规则、提供 action 函数。

---

## 三、消息定义 (msg_def.h)

```c
typedef enum {
    MSG_KEY_EVENT,            // 按键事件      param=键值
    MSG_COOKING_CTRL,         // 烹饪控制命令  param:高8=炉头号,低8=命令
    MSG_POWER_CTRL,           // 功率下发      data_ptr→PowerCtrl_t
    MSG_FAN_CTRL,             // 风机控制      param=档位0-3
    MSG_DISPLAY_REFRESH,      // 显示刷新      data_ptr→DisplayCmd_t
    MSG_TIMER_100MS,          // 100ms节拍    param=无
    MSG_TIMER_1S,             // 1秒节拍      param=无
    MSG_COMM_DATA_UPDATE,     // 通讯数据更新  data_ptr→ModbusData_t
    MSG_SYSTEM_ERROR,         // 系统错误      data_ptr→ProtectFault_t
    MSG_BUZZER_REQ,           // 蜂鸣请求      param:低8=次数,高8=间隔
    MSG_COUNT
} MsgId_t;
```

---

## 四、核心架构：JSON 驱动状态机

### 4.0 哲学

```
┌──────────────────────────────────────────────────────────────┐
│                                                              │
│   C 代码 = 通用引擎 + action 函数库                          │
│   JSON   = 所有业务规则 (状态/转换/显示/时序)                │
│                                                              │
│   改型时只改 JSON，不动 C。                                  │
│   AI 改数据比改逻辑安全 100 倍。                             │
│                                                              │
└──────────────────────────────────────────────────────────────┘
```

**C 代码职责（稳定层，产品生命周期内基本不变）：**
- 加载 JSON 文本 → cJSON 解析 → 构建规则表
- 接收 MSG_KEY_EVENT → 遍历当前节点 events[] → 匹配 key+type
- 执行匹配项的 action (按名字符串查 C 函数表)
- 跳转 next_node → 生成 DisplayCmd_t → 发送 MSG_DISPLAY_REFRESH
- 每 100ms tick → idle_timer 倒计时 → 触发 timeout_event

**JSON 职责（可变层，改型时只改这个文件）：**
- 所有状态节点的定义
- 所有按键事件的响应规则
- 所有显示行为 (内容/模式/LED/小数点)
- 所有超时参数

### 4.1 数据流

```
触摸HW ──→ hal_key ──→ drv_key ──→ MSG_KEY_EVENT
                                       │
                                       v
                              ┌─────────────────┐
                              │   ui_engine.c   │
                              │  (通用JSON引擎)  │
                              │                 │
                              │  load JSON      │
                              │  match event    │
                              │  call action    │
                              │  gen display    │
                              └──────┬──────────┘
                                     │
                         ┌───────────┼───────────┐
                         v           v           v
                    MSG_BUZZ    MSG_COOK    MSG_DISPLAY
                               (cook.c)         │
                                                v
                                          drv_display
                                          (显示命令→缓冲)
                                                │
                                                v
                                         hal_display
                                       (TIM0 ISR扫描)
```

### 4.2 显示状态枚举

```c
/** 一个 UINT8 替代所有分散的 BOOL 标志 */
typedef enum {
    DSP_CONSTANT        = 0,    /* 常亮                                */
    DSP_OFF             = 1,    /* 全灭                                */
    DSP_FLASH_500MS     = 2,    /* 闪烁 500ms亮/500ms灭                */
    DSP_FLASH_250MS     = 3,    /* 快闪 250ms                          */
    DSP_ALT_PWR_TIME    = 10,   /* 交替: 功率 ↔ 剩余时间 (各5秒)       */
    DSP_ALT_PWR_TEMP    = 11,   /* 交替: 功率 ↔ 温度 (各5秒)          */
    DSP_ALT_TIME_TEMP   = 12,   /* 交替: 时间 ↔ 温度 (各5秒)          */
    DSP_ALT_BOOST_TIME  = 13,   /* 交替: "P" ↔ 剩余时间 (各5秒)       */
    DSP_ALT_HBAR_VBAR   = 14,   /* 交替: 横杠 ↔ 竖杠 (组合设置态)     */
    /* 250种状态空间充裕 */
} DisplayState_t;
```

### 4.3 状态变量（JSON 规则操作的对象）

```c
/* 每个炉头独立状态 */
typedef struct {
    uint8_t  power_level;        /* 档位: 0-9, 10=Boost               */
    uint16_t timer_total_sec;    /* 定时总秒数 (0=无定时)              */
    uint16_t timer_remain_sec;   /* 定时剩余秒数                       */
    uint8_t  zone_group;         /* 组合归属: 0=独立, 1=组A, 2=组B     */
    uint8_t  boost_remain_sec;   /* Boost剩余秒数 (0=非Boost)          */
    uint8_t  pre_boost_level;    /* 进入Boost前的原档位                */
} HeadState_t;

/* 全局UI上下文 */
typedef struct {
    uint8_t  current_node;       /* 当前状态节点ID (字符串hash)        */
    uint8_t  prev_node;          /* 前一状态 (童锁解锁恢复用)          */
    uint8_t  selected_head;      /* 当前选中炉头 0-3, 0xFF=无          */
    uint8_t  child_lock   : 1;
    uint8_t  paused       : 1;
    uint16_t idle_timer_ms;      /* 无操作倒计时 (5s超时用)            */
} UI_Context_t;
```

### 4.4 显示输出

```c
typedef struct {
    uint8_t  digit_buff[2];      /* 两个数码管: SMG段码值              */
    uint8_t  dp_mask;            /* 小数点: bit0, bit1                */
    DisplayState_t disp_state;   /* 显示模式                           */
    uint8_t  led_mask;           /* 该炉头关联LED位                    */
} HeadDisplay_t;

typedef struct {
    HeadDisplay_t heads[4];
    uint8_t  global_leds[3];     /* 全局LED                            */
    uint8_t  special;            /* 0=正常, 1=全显, 2=版本号           */
} DisplayCmd_t;
```

### 4.5 硬件映射

```
IO_Drive_Disp_BUFF[11]:
  [0..1] = 炉头0 SMG (左数码管左两位)
  [2..3] = 炉头1 SMG (左数码管右两位)
  [4..5] = 炉头2 SMG (右数码管左两位)
  [6..7] = 炉头3 SMG (右数码管右两位)
  [8]    = LED[0]
  [9]    = LED[1]
  [10]   = LED[2]

8 SEG: PC8(A) PB14(B) PE9(C) PC4(D) PD13(E) PB15(F) PE8(G) PC1(H)
11 COM: PE11(1) PE12(2) PA10(3) PE10(4) PA6(5) PA1(6) PA0(7) PA7(8) PC0(9) PA5(10) PA4(11)
```

---

## 五、单向数据流

```
D0: 触摸硬件 → hal_key → SC_TK_Scan() → 32位通道位掩码
D1: 通道掩码 → drv_key → 去抖+查表 → MSG_KEY_EVENT(key_code, state)
D2: MSG_KEY_EVENT → ui_engine → JSON规则匹配 → 状态转换
D3: 状态转换 → ui_engine → 生成 DisplayCmd_t → MSG_DISPLAY_REFRESH
D4: MSG_DISPLAY_REFRESH → drv_display → Disp_DATA[4] → IO_Drive_Disp_BUFF[11]
D5: IO_Drive_Disp_BUFF → hal_display (TIM0 ISR) → GPIO SEG+COM 扫描
D6: MSG_TIMER_100MS → ui_engine → idle_timer 倒计时 + 显示时序
D7: MSG_TIMER_1S → ui_engine → 定时器倒计时 → timeout_event 触发
D8: MSG_TIMER_1S → cook → 定时+Boost超时检测 → MSG_COOKING_CTRL
```

---

## 六、JSON 规则文件格式

### 6.0 完整示例 —— 工作态 S5_WORKING

```jsonc
{
  "node_id": "S5_WORKING",
  "desc": "工作态",
  "entry_display": {
    "heads": "all",
    "smg_source": "power_level",
    "disp_state": "DSP_CONSTANT",
    "dp": "off",
    "leds": {
      "power": "on",
      "head_match": "on"
    }
  },
  "events": [
    {
      "id": "S5_E1",
      "key": "KEY_PAUSE",
      "type": "PRESS",
      "action": "pause_all",
      "next": "S11_PAUSE"
    },
    {
      "id": "S5_E2",
      "key": "KEY_HEAD_1",
      "type": "PRESS",
      "action": "select_head",
      "arg": 0,
      "next": "S4_FLASHING"
    },
    {
      "id": "S5_E3",
      "key": "KEY_HEAD_2",
      "type": "PRESS",
      "action": "select_head",
      "arg": 1,
      "next": "S4_FLASHING"
    },
    {
      "id": "S5_E4",
      "key": "KEY_HEAD_3",
      "type": "PRESS",
      "action": "select_head",
      "arg": 2,
      "next": "S4_FLASHING"
    },
    {
      "id": "S5_E5",
      "key": "KEY_HEAD_4",
      "type": "PRESS",
      "action": "select_head",
      "arg": 3,
      "next": "S4_FLASHING"
    },
    {
      "id": "S5_E6",
      "key": "KEY_ONOFF",
      "type": "LONG_PRESS",
      "timeout_ms": 1500,
      "action": "power_off_all",
      "next": "S2_STANDBY"
    },
    {
      "id": "S5_E7",
      "key": "KEY_LOCK",
      "type": "LONG_PRESS",
      "timeout_ms": 1500,
      "action": "child_lock_on",
      "next": "S10_CHILD_LOCK"
    },
    {
      "id": "S5_E8",
      "key": "KEY_POWER_0",
      "type": "PRESS",
      "guard": "head_is_selected",
      "action": "set_power_zero",
      "next": "S2_STANDBY"
    }
  ],
  "timeout": null
}
```

### 6.1 定时生效态 —— 交替显示

```jsonc
{
  "node_id": "S7_TIMER_ACTIVE",
  "desc": "定时生效",
  "entry_display": {
    "heads": "selected",
    "smg_source": "timer_remain",
    "disp_state": "DSP_ALT_PWR_TIME",
    "dp": "timer",
    "leds": {
      "timer": "on",
      "head_match": "on"
    }
  },
  "events": [
    {
      "id": "S7_E1",
      "key": "KEY_TIMER",
      "type": "PRESS",
      "action": "timer_re_edit",
      "next": "S6_TIMER_SETTING"
    },
    {
      "id": "S7_E2",
      "key": "KEY_ONOFF",
      "type": "LONG_PRESS",
      "timeout_ms": 1500,
      "action": "power_off_all",
      "next": "S2_STANDBY"
    }
  ],
  "timeout": {
    "guard": "timer_remain_zero",
    "action": "timer_expire",
    "next": "S2_STANDBY"
  }
}
```

### 6.2 Boost 态 —— 显示 "P"

```jsonc
{
  "node_id": "S8_BOOST",
  "desc": "Boost工作态",
  "entry_display": {
    "heads": "selected",
    "smg_source": "boost_symbol",
    "disp_state": "DSP_CONSTANT",
    "dp": "off",
    "leds": {
      "power": "on",
      "head_match": "on"
    }
  },
  "events": [
    {
      "id": "S8_E1",
      "key": "KEY_POWER_0",
      "type": "PRESS",
      "action": "boost_exit_to_level",
      "arg": 0,
      "next": "S5_WORKING"
    },
    {
      "id": "S8_E2",
      "key": "KEY_POWER_9",
      "type": "PRESS",
      "action": "boost_exit_to_level",
      "arg": 9,
      "next": "S5_WORKING"
    },
    {
      "id": "S8_E3",
      "key": "KEY_TIMER",
      "type": "PRESS",
      "action": "boost_exit_then_timer",
      "next": "S6_TIMER_SETTING"
    },
    {
      "id": "S8_E4",
      "key": "KEY_ONOFF",
      "type": "LONG_PRESS",
      "timeout_ms": 1500,
      "action": "power_off_all",
      "next": "S2_STANDBY"
    }
  ],
  "timeout": {
    "guard": "boost_timeout",
    "action": "boost_exit_to_prev",
    "next": "S5_WORKING"
  }
}
```

### 6.3 字段说明

| JSON 字段 | 类型 | 说明 |
|-----------|------|------|
| `node_id` | string | 状态唯一标识，如 `"S5_WORKING"` |
| `entry_display.heads` | string | `"all"` / `"selected"` / `"working_only"` / `"none"` |
| `entry_display.smg_source` | string | `"power_level"` / `"timer_remain"` / `"timer_total"` / `"dash"` / `"P"` / `"PA"` / `"Loc"` / `"Er"` / `"Hot"` / `"V"` / `"hbar_vbar"` |
| `entry_display.disp_state` | string | DisplayState_t 枚举名: `"DSP_CONSTANT"` / `"DSP_FLASH_500MS"` / `"DSP_ALT_PWR_TIME"` ... |
| `entry_display.dp` | string | `"off"` / `"timer"` (小数点常亮) |
| `entry_display.leds` | object | LED 名 → `"on"` / `"off"` / `"flash"` |
| `events[].key` | string | KeyCode_t 枚举名 |
| `events[].type` | string | `"PRESS"` / `"LONG_PRESS"` / `"RELEASE"` |
| `events[].timeout_ms` | int | 长按判定时间 (仅 LONG_PRESS) |
| `events[].guard` | string? | C guard 函数名，null=无条件 |
| `events[].action` | string | C action 函数名 |
| `events[].arg` | int? | action 参数 (如炉头号) |
| `events[].next` | string | 目标 node_id |
| `timeout.guard` | string? | 超时条件 C 函数名 |
| `timeout.action` | string | 超时动作 |
| `timeout.next` | string | 超时后跳转 node_id |

### 6.4 smg_source 与 C 侧渲染

```c
/* ui_engine 根据 smg_source 自动查表生成段码 */
static const SmgSourceEntry_t s_smg_sources[] = {
    {"power_level",   smg_render_power_level},    /* "0"-"9", "P"        */
    {"timer_remain",  smg_render_timer_mmss},     /* "15:00"             */
    {"timer_total",   smg_render_timer_mmss},
    {"dash",          smg_render_dash},           /* "--"                */
    {"P",             smg_render_char_P},         /* "P "                */
    {"PA",            smg_render_char_PA},        /* "PA"                */
    {"Loc",           smg_render_char_Loc},       /* "Loc"               */
    {"Er",            smg_render_error_code},     /* "Er0"-"Er9"         */
    {"Hot",           smg_render_char_Hot},       /* "Hot"               */
    {"V",             smg_render_version},        /* "V1.0" / "P1.0"     */
    {"hbar_vbar",     smg_render_zone_pattern},   /* 横杠/竖杠/方框       */
    {NULL, NULL},
};
```

> 新增 smg_source 只需在 C 侧加一行渲染函数，JSON 引用其名。渲染函数是纯数据→段码映射，不含业务逻辑。

---

## 七、C 侧实现

### 7.1 ui_engine —— 通用 JSON 执行器

```c
/**
 * ui_engine 不包含任何业务规则。
 * 它只是一个 JSON 解释器：
 *   1. 加载 JSON → cJSON 解析 → 构建规则链表
 *   2. 接收按键 → 查当前节点 events[] → 匹配 key+type+guard
 *   3. 执行 action (按名查函数表)
 *   4. 跳转 next_node → 生成 DisplayCmd_t
 *   5. 每 100ms tick → idle_timer → timeout 检测
 */

void UI_Engine_Init(void);           /* 加载 JSON 规则，初始化状态机      */
void UI_Engine_OnKey(uint8_t key, uint8_t state);  /* 按键输入            */
void UI_Engine_Tick100ms(void);      /* 100ms 节拍 (idle + 显示时序)     */
void UI_Engine_Tick1s(void);         /* 1s 节拍 (定时器 + timeout)       */
const DisplayCmd_t* UI_Engine_GetDisplay(void);  /* 获取当前显示命令      */
```

### 7.2 action 函数表

```c
/* 所有 action 函数签名统一 */
typedef void (*UIAction_t)(uint8_t arg);

/* JSON 中 "action": "xxx" → 查此表 */
static const struct {
    const char *name;
    UIAction_t  func;
} s_action_table[] = {
    {"power_on_init",        ui_act_power_on_init},
    {"power_off_all",        ui_act_power_off_all},
    {"select_head",          ui_act_select_head},
    {"set_power_level",      ui_act_set_power_level},
    {"set_power_zero",       ui_act_set_power_zero},
    {"boost_enter",          ui_act_boost_enter},
    {"boost_exit_to_level",  ui_act_boost_exit_to_level},
    {"boost_exit_to_prev",   ui_act_boost_exit_to_prev},
    {"boost_exit_then_timer",ui_act_boost_exit_then_timer},
    {"timer_start_setting",  ui_act_timer_start_setting},
    {"timer_confirm",        ui_act_timer_confirm},
    {"timer_cancel",         ui_act_timer_cancel},
    {"timer_re_edit",        ui_act_timer_re_edit},
    {"timer_expire",         ui_act_timer_expire},
    {"timer_adjust_plus",    ui_act_timer_adjust_plus},
    {"timer_adjust_minus",   ui_act_timer_adjust_minus},
    {"pause_all",            ui_act_pause_all},
    {"pause_resume",         ui_act_pause_resume},
    {"child_lock_on",        ui_act_child_lock_on},
    {"child_lock_off",       ui_act_child_lock_off},
    {"zone_enter",           ui_act_zone_enter},
    {"zone_toggle_head",     ui_act_zone_toggle_head},
    {"zone_confirm",         ui_act_zone_confirm},
    {"zone_cancel",          ui_act_zone_cancel},
    {NULL, NULL},
};
```

### 7.3 guard 函数表

```c
/* guard 函数: 返回 1=条件满足, 0=不满足 */
typedef uint8_t (*UIGuard_t)(void);

static const struct {
    const char *name;
    UIGuard_t   func;
} s_guard_table[] = {
    {"head_is_selected",     guard_head_is_selected},
    {"timer_remain_zero",    guard_timer_remain_zero},
    {"boost_timeout",        guard_boost_timeout},
    {"any_head_working",     guard_any_head_working},
    {"no_head_working",      guard_no_head_working},
    {"is_full_zone",         guard_is_full_zone},
    {"has_existing_zone",    guard_has_existing_zone},
    {NULL, NULL},
};
```

### 7.4 JSON 加载

```c
/* JSON 规则文本来源：编译时链接的 const char[] 数组 */
extern const char ui_rules_json[];   /* 由 json_to_c.py 从 ui_rules.json 生成 */
extern const uint32_t ui_rules_json_len;

static UI_Node_t s_nodes[MAX_NODES];  /* 解析后的规则表 */
static uint8_t s_node_count;

void UI_Engine_Init(void)
{
    cJSON *root, *states, *state_obj;
    uint8_t i;

    root = cJSON_Parse(ui_rules_json);
    states = cJSON_GetObjectItem(root, "states");

    s_node_count = (uint8_t)cJSON_GetArraySize(states);
    for (i = 0; i < s_node_count; i++) {
        state_obj = cJSON_GetArrayItem(states, i);
        ui_parse_node(state_obj, &s_nodes[i]);
    }

    cJSON_Delete(root);  /* 解析完毕，释放 JSON 源码 */

    /* 设置初始状态 */
    ui_ctx.current_node = NODE_ID("S0_POWER_ON");
}
```

### 7.5 JSON → C 编译链

```python
# scripts/json_to_c.py
import sys, json

def json_to_c(json_path, var_name="ui_rules_json"):
    with open(json_path, 'r', encoding='utf-8') as f:
        obj = json.load(f)
    compact = json.dumps(obj, separators=(',', ':'), ensure_ascii=False)
    escaped = compact.replace('\\', '\\\\').replace('"', '\\"')

    print(f'/* Auto-generated from {json_path} */')
    print(f'const char {var_name}[] = "{escaped}";')
    print(f'const unsigned int {var_name}_len = {len(compact)};')

if __name__ == '__main__':
    json_to_c(sys.argv[1])
```

```
构建流程:
  ui_rules.json ──→ json_to_c.py ──→ ui_rules_json.c ──→ 链接
```

---

## 八、标准元素注册表

> 元素 = 类型 + 属性(params) + 方法(C函数)。增加新元素不修改已有元素。

### 8.1 元素类型总览

```
元素类型          实例数  说明
──────────────────────────────────────
SMG    数码管      ~15    段码内容 + 显示模式
LED    指示灯      ~18    亮/灭/闪
DP     小数点      ~3     常灭/常亮/闪
BUZZ   蜂鸣器      ~5     短鸣/长鸣/双响/按键音
ACT    动作        ~20    状态变量修改函数
GUARD  条件判定    ~15    返回 bool 的纯函数
SUBFLOW 子流程     ~4     独立的 JSON 逻辑段
```

### 8.2 每个元素 = 类型定义 + 预置实例列表

类型定义写在 C 头文件里（稳定），实例列表写在 JSON 的 `elements` 段里（可扩展）。

---

### 8.3 SMG — 数码管元素

**属性 (params):**

| 参数 | 类型 | 说明 |
|------|------|------|
| `source` | string | 内容来源: `"power_level"` / `"timer_remain"` / `"timer_total"` / `"dash"` / `"P"` / `"PA"` / `"Loc"` / `"Er"` / `"Hot"` / `"version"` / `"hbar_vbar"` / `"text"` |
| `mode` | string | 显示模式: `"constant"` / `"off"` / `"flash"` / `"alternate"` / `"marquee"` / `"fade"` |
| `on_ms` | int | 亮持续 ms (flash/alternate/marquee 用) |
| `off_ms` | int | 灭持续 ms (flash 用) |
| `alt_source` | string | 交替时的第二内容来源 (alternate 用) |
| `alt_ms` | int | 交替时第二内容持续 ms |
| `text` | string | `source="text"` 时的直接文本 |
| `speed_ms` | int | 跑马灯滚动速度 (marquee 用) |

**方法 (C 函数，按 mode 选择):**

| mode | C 函数 | 说明 |
|------|--------|------|
| `constant` | `smg_render_constant()` | 直接输出 source 对应的段码 |
| `off` | `smg_render_off()` | 输出 0x00 |
| `flash` | `smg_render_flash()` | on_ms 亮 → off_ms 灭 → 循环 |
| `alternate` | `smg_render_alternate()` | source(on_ms)→alt_source(alt_ms)→循环 |
| `marquee` | `smg_render_marquee()` | 文本滚动 (扩展元素) |
| `fade` | `smg_render_fade()` | PWM 渐隐 (扩展元素) |

**预置实例:**

```jsonc
// —— 基础内容 ——
"smg_power_level":   { "type": "SMG", "params": { "source": "power_level", "mode": "constant" } },
"smg_timer_remain":  { "type": "SMG", "params": { "source": "timer_remain", "mode": "constant" } },
"smg_timer_total":   { "type": "SMG", "params": { "source": "timer_total", "mode": "constant" } },
"smg_dash":          { "type": "SMG", "params": { "source": "dash", "mode": "constant" } },
"smg_P":             { "type": "SMG", "params": { "source": "P", "mode": "constant" } },
"smg_PA":            { "type": "SMG", "params": { "source": "PA", "mode": "constant" } },
"smg_Loc":           { "type": "SMG", "params": { "source": "Loc", "mode": "constant" } },
"smg_Er":            { "type": "SMG", "params": { "source": "Er", "mode": "constant" } },
"smg_Hot":           { "type": "SMG", "params": { "source": "Hot", "mode": "constant" } },
"smg_version":       { "type": "SMG", "params": { "source": "version", "mode": "constant" } },
"smg_hbar_vbar":     { "type": "SMG", "params": { "source": "hbar_vbar", "mode": "constant" } },

// —— 带显示模式 ——
"smg_power_flash":   { "type": "SMG", "params": { "source": "power_level", "mode": "flash", "on_ms": 500, "off_ms": 500 } },
"smg_dash_flash":    { "type": "SMG", "params": { "source": "dash", "mode": "flash", "on_ms": 500, "off_ms": 500 } },
"smg_timer_flash":   { "type": "SMG", "params": { "source": "timer_total", "mode": "flash", "on_ms": 500, "off_ms": 500 } },
"smg_PA_flash":      { "type": "SMG", "params": { "source": "PA", "mode": "flash", "on_ms": 500, "off_ms": 500 } },

// —— 交替显示 ——
"smg_alt_power_time": { "type": "SMG", "params": { "source": "power_level", "mode": "alternate", "on_ms": 5000, "alt_source": "timer_remain", "alt_ms": 5000 } },
"smg_alt_boost_time": { "type": "SMG", "params": { "source": "P", "mode": "alternate", "on_ms": 5000, "alt_source": "timer_remain", "alt_ms": 5000 } },
"smg_alt_hbar_vbar":  { "type": "SMG", "params": { "source": "hbar", "mode": "alternate", "on_ms": 1000, "alt_source": "vbar", "alt_ms": 1000 } },

// —— 扩展 (阶段2) ——
"smg_marquee_intro":  { "type": "SMG", "params": { "source": "text", "text": "HELLO", "mode": "marquee", "speed_ms": 300 } },
```

---

### 8.4 LED — 指示灯元素

**属性:**

| 参数 | 类型 | 说明 |
|------|------|------|
| `led_id` | string | LED 名: `"power"` / `"lock"` / `"zone"` / `"pause"` / `"timer"` / `"head_1"`..`"head_4"` / `"plv_0"`..`"plv_9"` |
| `mode` | string | `"on"` / `"off"` / `"flash"` / `"breathe"` |
| `on_ms` | int | flash/breathe 时间 |
| `off_ms` | int | flash 灭时间 |

**方法:**

| mode | C 函数 |
|------|--------|
| `on` | `led_set_on()` |
| `off` | `led_set_off()` |
| `flash` | `led_set_flash(on_ms, off_ms)` |
| `breathe` | `led_set_breathe(on_ms)` (扩展) |

**预置实例:**

```jsonc
// —— 单灯 ——
"led_power_on":    { "type": "LED", "params": { "led_id": "power", "mode": "on" } },
"led_power_off":   { "type": "LED", "params": { "led_id": "power", "mode": "off" } },
"led_power_flash": { "type": "LED", "params": { "led_id": "power", "mode": "flash", "on_ms": 500, "off_ms": 500 } },
"led_lock_on":     { "type": "LED", "params": { "led_id": "lock", "mode": "on" } },
"led_lock_off":    { "type": "LED", "params": { "led_id": "lock", "mode": "off" } },
"led_zone_on":     { "type": "LED", "params": { "led_id": "zone", "mode": "on" } },
"led_zone_off":    { "type": "LED", "params": { "led_id": "zone", "mode": "off" } },
"led_pause_on":    { "type": "LED", "params": { "led_id": "pause", "mode": "on" } },
"led_pause_off":   { "type": "LED", "params": { "led_id": "pause", "mode": "off" } },
"led_timer_on":    { "type": "LED", "params": { "led_id": "timer", "mode": "on" } },
"led_timer_off":   { "type": "LED", "params": { "led_id": "timer", "mode": "off" } },

// —— 炉头选择灯 ——
"led_head_1_on":   { "type": "LED", "params": { "led_id": "head_1", "mode": "on" } },
// ... head_2, head_3, head_4 同理

// —— 档位灯 ——
"led_plv_5_on":    { "type": "LED", "params": { "led_id": "plv_5", "mode": "on" } },
// ... plv_0~plv_9

// —— 组 ——
"leds_all_on":     { "type": "LED", "params": { "led_id": "all", "mode": "on" } },
"leds_all_off":    { "type": "LED", "params": { "led_id": "all", "mode": "off" } },
```

> `led_id: "all"` 是特殊值，一键控所有 LED。不需为每个灯单独写实例。

---

### 8.5 DP — 小数点元素

**属性:**

| 参数 | 类型 | 说明 |
|------|------|------|
| `mode` | string | `"off"` / `"on"` / `"flash"` |
| `on_ms` | int | flash 时间 |
| `off_ms` | int | flash 时间 |

> DP 跟随当前选中炉头，无需指定 led_id。

**预置实例:**

```jsonc
"dp_off":   { "type": "DP", "params": { "mode": "off" } },
"dp_on":    { "type": "DP", "params": { "mode": "on" } },
"dp_flash": { "type": "DP", "params": { "mode": "flash", "on_ms": 500, "off_ms": 500 } },
```

---

### 8.6 BUZZ — 蜂鸣器元素

**属性:**

| 参数 | 类型 | 说明 |
|------|------|------|
| `pattern` | string | `"single"` / `"double"` / `"long"` / `"triple"` / `"melody"` |
| `duration_ms` | int | 单次鸣叫时长 |
| `gap_ms` | int | 间隔 |
| `freq_hz` | int | 频率 (0=默认) |

**方法:** `buzzer_beep(params)` — 唯一方法，按 params 执行不同鸣叫模式。

**预置实例:**

```jsonc
"buzz_key":        { "type": "BUZZ", "params": { "pattern": "single", "duration_ms": 50, "gap_ms": 0 } },
"buzz_timer_done": { "type": "BUZZ", "params": { "pattern": "long", "duration_ms": 1000, "gap_ms": 0 } },
"buzz_error":      { "type": "BUZZ", "params": { "pattern": "triple", "duration_ms": 200, "gap_ms": 100 } },
"buzz_double":     { "type": "BUZZ", "params": { "pattern": "double", "duration_ms": 100, "gap_ms": 100 } },
```

---

### 8.7 ACT — 动作元素

动作元素直接操作 `HeadState_t[]` 和 `UI_Context_t`。

**属性:**

| 参数 | 类型 | 说明 |
|------|------|------|
| `func` | string | C 函数名 |
| `arg` | int? | 参数 (炉头号/档位值/步进值) |

> `arg` 为 null 时用事件携带的 `{event.arg}` 动态传入。

**预置实例:**

```jsonc
// —— 开关机 ——
"act_power_on_init":      { "type": "ACT", "params": { "func": "power_on_init" } },
"act_power_off_all":      { "type": "ACT", "params": { "func": "power_off_all" } },

// —— 炉头选择 ——
"act_select_head_1":      { "type": "ACT", "params": { "func": "select_head", "arg": 0 } },
"act_select_head_2":      { "type": "ACT", "params": { "func": "select_head", "arg": 1 } },
"act_select_head_3":      { "type": "ACT", "params": { "func": "select_head", "arg": 2 } },
"act_select_head_4":      { "type": "ACT", "params": { "func": "select_head", "arg": 3 } },

// —— 档位 ——
"act_set_power_0":        { "type": "ACT", "params": { "func": "set_power_level", "arg": 0 } },
"act_set_power_1":        { "type": "ACT", "params": { "func": "set_power_level", "arg": 1 } },
// ... 2~8
"act_set_power_9":        { "type": "ACT", "params": { "func": "set_power_level", "arg": 9 } },
"act_set_power_zero":     { "type": "ACT", "params": { "func": "set_power_zero" } },

// —— Boost ——
"act_boost_enter":        { "type": "ACT", "params": { "func": "boost_enter" } },
"act_boost_exit_to_0":    { "type": "ACT", "params": { "func": "boost_exit_to_level", "arg": 0 } },
// ... 1~8
"act_boost_exit_to_9":    { "type": "ACT", "params": { "func": "boost_exit_to_level", "arg": 9 } },
"act_boost_exit_to_prev": { "type": "ACT", "params": { "func": "boost_exit_to_prev" } },
"act_boost_exit_then_timer": { "type": "ACT", "params": { "func": "boost_exit_then_timer" } },

// —— 定时 ——
"act_timer_start":        { "type": "ACT", "params": { "func": "timer_start_setting" } },
"act_timer_plus":         { "type": "ACT", "params": { "func": "timer_adjust", "arg": 1 } },
"act_timer_minus":        { "type": "ACT", "params": { "func": "timer_adjust", "arg": -1 } },
"act_timer_confirm":      { "type": "ACT", "params": { "func": "timer_confirm" } },
"act_timer_cancel":       { "type": "ACT", "params": { "func": "timer_cancel" } },
"act_timer_re_edit":      { "type": "ACT", "params": { "func": "timer_re_edit" } },
"act_timer_expire":       { "type": "ACT", "params": { "func": "timer_expire" } },

// —— 暂停 ——
"act_pause_all":          { "type": "ACT", "params": { "func": "pause_all" } },
"act_pause_resume":       { "type": "ACT", "params": { "func": "pause_resume" } },

// —— 童锁 ——
"act_child_lock_on":      { "type": "ACT", "params": { "func": "child_lock_on" } },
"act_child_lock_off":     { "type": "ACT", "params": { "func": "child_lock_off" } },

// —— 组合 ——
"act_zone_enter":         { "type": "ACT", "params": { "func": "zone_enter" } },
"act_zone_toggle_1":      { "type": "ACT", "params": { "func": "zone_toggle_head", "arg": 0 } },
"act_zone_toggle_2":      { "type": "ACT", "params": { "func": "zone_toggle_head", "arg": 1 } },
"act_zone_toggle_3":      { "type": "ACT", "params": { "func": "zone_toggle_head", "arg": 2 } },
"act_zone_toggle_4":      { "type": "ACT", "params": { "func": "zone_toggle_head", "arg": 3 } },
"act_zone_confirm":       { "type": "ACT", "params": { "func": "zone_confirm" } },
"act_zone_cancel":        { "type": "ACT", "params": { "func": "zone_cancel" } },
```

> 注意：`act_set_power_0` 和 `act_set_power_zero` 不同。前者把档位设为 0 但不停止炉头（闪烁中选 0 档），后者是直接停止该炉头（工作中按 0 键关机）。

---

### 8.8 GUARD — 条件判定元素

**属性:**

| 参数 | 类型 | 说明 |
|------|------|------|
| `func` | string | C 函数名，返回 1/0 |
| `arg` | int? | 参数 |

**预置实例:**

```jsonc
// —— 炉头状态 ——
"guard_head_selected":    { "type": "GUARD", "params": { "func": "head_is_selected" } },
"guard_head_power_zero":  { "type": "GUARD", "params": { "func": "head_power_is_zero" } },
"guard_head_power_active":{ "type": "GUARD", "params": { "func": "head_power_active" } },

// —— 定时 ——
"guard_timer_remain_zero":{ "type": "GUARD", "params": { "func": "timer_remain_zero" } },
"guard_has_timer":        { "type": "GUARD", "params": { "func": "has_timer" } },

// —— Boost ——
"guard_boost_timeout":    { "type": "GUARD", "params": { "func": "boost_timeout" } },
"guard_is_boost":         { "type": "GUARD", "params": { "func": "is_boost" } },

// —— 全局 ——
"guard_any_head_working": { "type": "GUARD", "params": { "func": "any_head_working" } },
"guard_no_head_working":  { "type": "GUARD", "params": { "func": "no_head_working" } },
"guard_idle_5s":          { "type": "GUARD", "params": { "func": "idle_5s" } },
"guard_idle_5s_pwr_zero": { "type": "GUARD", "params": { "func": "idle_5s_and_power_zero" } },
"guard_idle_5s_pwr_act":  { "type": "GUARD", "params": { "func": "idle_5s_and_power_active" } },

// —— 组合 ——
"guard_is_full_zone":     { "type": "GUARD", "params": { "func": "is_full_zone" } },
"guard_has_existing_zone":{ "type": "GUARD", "params": { "func": "has_existing_zone" } },
"guard_no_existing_zone": { "type": "GUARD", "params": { "func": "no_existing_zone" } },
```

---

### 8.9 SUBFLOW — 子流程元素

`SUBFLOW` 是一个特殊元素——它引用的不是 C 函数，而是另一个 JSON 文件。

**属性:**

| 参数 | 类型 | 说明 |
|------|------|------|
| `file` | string | 子流程 JSON 文件名 |
| `on_exit` | string | 退出后跳回的状态节点 |
| `inherit_keys` | bool | 是否继承主流程的全局按键 (ONOFF/LOCK) |

**预置实例:**

```jsonc
"sub_timer":   { "type": "SUBFLOW", "params": { "file": "timer.json", "on_exit": "__CALLER__" } },
"sub_zone":    { "type": "SUBFLOW", "params": { "file": "zone.json", "on_exit": "__CALLER__" } },
"sub_boost":   { "type": "SUBFLOW", "params": { "file": "boost.json", "on_exit": "__CALLER__" } },
"sub_poweron": { "type": "SUBFLOW", "params": { "file": "poweron.json", "on_exit": "S2_STANDBY" } },
```

> `__CALLER__` = 回到调用子流程的那个节点。poweron 子流程例外，它退出后固定到 S2_STANDBY。

---

### 8.10 元素的使用：状态节点如何引用

状态节点不再手写 display 字段，而是引用元素名：

```jsonc
{
  "node_id": "S2_STANDBY",
  "elements": ["smg_dash_flash", "led_power_flash", "dp_off"],
  "events": [
    { "key": "KEY_ONOFF", "type": "LONG_PRESS",
      "do": ["act_power_on_init"],
      "next": "S3_HEAD_SELECT" }
  ]
},

{
  "node_id": "S7_TIMER_ACTIVE",
  "elements": ["smg_alt_power_time", "led_timer_on", "led_power_on", "dp_on"],
  "events": [
    { "key": "KEY_TIMER", "type": "PRESS",
      "do": ["act_timer_re_edit"],
      "next": "S6_TIMER_SETTING" }
  ],
  "timeout": {
    "guard": "guard_timer_remain_zero",
    "do": ["buzz_timer_done", "act_timer_expire"],
    "next": "S2_STANDBY"
  }
}
```

**规则:**
- `elements[]` — 进入该状态时激活的元素列表
- `events[].do[]` — 事件触发时执行的元素列表 (ACT/BUZZ/SUBFLOW)
- `timeout.guard` — 元素名，引用 GUARD 实例
- `timeout.do[]` — 超时触发时执行的元素列表

---

### 8.11 新增元素的步骤

```
以新增跑马灯为例:

Step 1 — 定义属性
  在 C 中扩展 smg_mode 枚举: SMG_MODE_MARQUEE
  smg_params 新增字段: speed_ms, text[]

Step 2 — 实现方法
  void smg_render_marquee(SmgParams_t *p);
  独立函数，不触碰 smg_render_constant/flash/alternate

Step 3 — 注册到 C 方法表
  { SMG_MODE_MARQUEE, smg_render_marquee },

Step 4 — JSON 新增实例
  "smg_marquee_intro": {
    "type": "SMG",
    "params": { "source": "text", "text": "HELLO", "mode": "marquee", "speed_ms": 300 }
  }

Step 5 — 状态节点引用
  "elements": ["smg_marquee_intro", ...]
```

> 已有元素完全不受影响。编译也只增量编译新增的 .c 文件。

---

## 九、状态机完整定义

```
[S0] POWER_ON        上电全显3秒
     display: all=0xFF, leds:all=on, DSP_CONSTANT
     timeout: 3s → S1_VERSION

[S1] VERSION         版本显示3秒
     display: smg_source="V", DSP_CONSTANT
     timeout: 3s → S2_STANDBY

[S2] STANDBY         待机态
     display: smg_source="dash", DSP_FLASH_500MS, leds:power=flash
     events:
       {key:ONOFF, type:LONG_PRESS, timeout_ms:1500, action:power_on_init, next:S3_HEAD_SELECT}
       {key:LOCK,  type:LONG_PRESS, timeout_ms:1500, action:child_lock_on, next:S10_CHILD_LOCK}

[S3] HEAD_SELECT     炉头选择态
     display: all=0, DSP_CONSTANT
     events:
       {key:HEAD_1..4,  type:PRESS, action:select_head, arg:0..3, next:S4_FLASHING}
       {key:ZONE,       type:PRESS, guard:no_existing_zone, action:zone_enter, next:S9_ZONE_SETTING}
       {key:ONOFF,      type:LONG_PRESS, action:power_off_all, next:S2_STANDBY}
       {key:LOCK,       type:LONG_PRESS, action:child_lock_on, next:S10_CHILD_LOCK}

[S4] FLASHING        选中闪烁态
     display: selected=DSP_FLASH_500MS, others=DSP_CONSTANT
     events:
       {key:POWER_0..8, type:PRESS, action:set_power_level, arg:0..8, guard:not_zero_when_0}
       {key:POWER_9,    type:PRESS,  action:set_power_level, arg:9}
       {key:POWER_9,    type:LONG_PRESS, action:boost_enter}
       {key:TIMER,      type:PRESS, action:timer_start_setting, next:S6_TIMER_SETTING}
       {key:HEAD_1..4,  type:PRESS, action:select_head, arg:0..3}  // stay in S4
       {key:PAUSE,      type:PRESS, action:pause_all, next:S11_PAUSE}
       {key:ONOFF,      type:LONG_PRESS, action:power_off_all, next:S2_STANDBY}
     timeout: {guard:idle_5s_and_power_zero, action:goto_standby, next:S2_STANDBY}
     timeout: {guard:idle_5s_and_power_active, action:stop_flash, next:S5_WORKING}

[S5] WORKING         工作态
     display: smg_source="power_level", DSP_CONSTANT
     events: (见 6.0 完整示例)

[S6] TIMER_SETTING   定时设置态
     display: smg_source="timer_total", DSP_FLASH_500MS, dp="timer"
     events:
       {key:PLUS,   type:PRESS, action:timer_adjust_plus}
       {key:MINUS,  type:PRESS, action:timer_adjust_minus}
       {key:TIMER,  type:PRESS, action:timer_confirm, next:S7_TIMER_ACTIVE}
       {key:TIMER,  type:LONG_PRESS, action:timer_cancel, next:S5_WORKING}
       {key:ONOFF,  type:LONG_PRESS, action:power_off_all, next:S2_STANDBY}
     timeout: {guard:idle_5s, action:timer_confirm, next:S7_TIMER_ACTIVE}

[S7] TIMER_ACTIVE    定时生效
     display: DSP_ALT_PWR_TIME, dp="timer"  (见 6.1)

[S8] BOOST           Boost态
     display: smg_source="P", DSP_CONSTANT  (见 6.2)

[S9] ZONE_SETTING    组合设置态
     display: smg_source="hbar_vbar", DSP_ALT_HBAR_VBAR, leds:zone=on
     events:
       {key:HEAD_1..4, type:PRESS, action:zone_toggle_head, arg:0..3}
       {key:ZONE,      type:PRESS, action:zone_confirm, next:S14_ZONE_ACTIVE}
       {key:ZONE,      type:LONG_PRESS, action:zone_cancel, next:S2_STANDBY}
     timeout: {guard:idle_5s, action:zone_confirm, next:S14_ZONE_ACTIVE}

[S10] CHILD_LOCK     童锁态
     display: smg_source="Loc", DSP_CONSTANT, leds:lock=on
     events:
       {key:LOCK,  type:LONG_PRESS, action:child_lock_off, next:"__PREV__"}
       {key:ONOFF, type:LONG_PRESS, action:power_off_all, next:S2_STANDBY}

[S11] PAUSE          暂停态
     display: working_heads="PA", DSP_FLASH_500MS; non_working=unchanged
     events:
       {key:PAUSE, type:PRESS, action:pause_resume, next:S5_WORKING}
       {key:ONOFF, type:LONG_PRESS, action:power_off_all, next:S2_STANDBY}

[S14] ZONE_ACTIVE    组合生效态
     display: normal digits, DSP_CONSTANT, leds:zone=on
     events:
       {key:HEAD_x,  type:PRESS, action:select_zone_group, next:S4_FLASHING}
       {key:ZONE,    type:PRESS, guard:is_full_zone, action:flash_hint}
       {key:ZONE,    type:LONG_PRESS, action:zone_cancel, next:S2_STANDBY}
```

---

## 九、Python 测试框架

```python
# test_framework.py —— 加载与 MCU 完全相同的 ui_rules.json
import json

class VirtualHead:
    def __init__(self, idx):
        self.idx = idx
        self.power_level = 0
        self.timer_total_sec = 0
        self.timer_remain_sec = 0
        self.boost_remain_sec = 0
        self.pre_boost_level = 0
        self.zone_group = 0

class VirtualUI:
    """完整模拟 ui_engine 行为，加载同一 JSON"""
    def __init__(self, rules_path="ui_rules.json"):
        with open(rules_path) as f:
            rules = json.load(f)
        self.nodes = {s["node_id"]: s for s in rules["states"]}
        self.heads = [VirtualHead(i) for i in range(4)]
        self.ctx = {"current_node": "S0_POWER_ON", "prev_node": None,
                     "selected_head": None, "child_lock": False,
                     "paused": False, "idle_timer_ms": 0}
        self.display = None
        self._enter_node("S0_POWER_ON")

    def _enter_node(self, node_id):
        """执行 entry_display → 生成 DisplayCmd_t"""
        node = self.nodes[node_id]
        self.ctx["current_node"] = node_id
        self.display = self._build_display_cmd(node["entry_display"])

    def _find_event(self, key, key_type):
        """在当前节点 events[] 中匹配 key + type，检查 guard"""
        for ev in self.nodes[self.ctx["current_node"]].get("events", []):
            if ev["key"] == key and ev["type"] == key_type:
                if ev.get("guard") and not self._eval_guard(ev["guard"]):
                    continue
                return ev
        return None

    def feed_key(self, key_name, key_type):
        """输入按键 → 匹配规则 → 执行 action → 跳转 → 生成显示"""
        ev = self._find_event(key_name, key_type)
        if ev is None:
            return False
        self._exec_action(ev["action"], ev.get("arg", 0))
        self.ctx["prev_node"] = self.ctx["current_node"]
        next_node = ev["next"]
        if next_node == "__PREV__":
            next_node = self.ctx["prev_node"]
        self._enter_node(next_node)
        return True

    def feed_tick_100ms(self):
        self.ctx["idle_timer_ms"] += 100
        node = self.nodes[self.ctx["current_node"]]
        to = node.get("timeout")
        if to and self._eval_guard(to.get("guard")):
            self._exec_action(to["action"])
            self._enter_node(to["next"])

    def feed_tick_1s(self):
        for h in self.heads:
            if h.timer_remain_sec > 0:
                h.timer_remain_sec -= 1
            if h.boost_remain_sec > 0:
                h.boost_remain_sec -= 1

    def _exec_action(self, name, arg=0):
        """模拟 C 侧 action 函数效果"""
        # ... 按名执行对应操作
        pass

    def _eval_guard(self, name):
        """模拟 C 侧 guard 函数"""
        # ... 按名执行对应判定
        pass
```

```python
# test_spec_states.py
def test_power_on_sequence():
    ui = VirtualUI("ui_rules.json")
    assert ui.ctx["current_node"] == "S0_POWER_ON"
    assert ui.display["special"] == "full_on"
    for _ in range(30): ui.feed_tick_100ms()
    assert ui.ctx["current_node"] == "S1_VERSION"
    for _ in range(30): ui.feed_tick_100ms()
    assert ui.ctx["current_node"] == "S2_STANDBY"

def test_full_main_loop():
    """S2→S3→S4→S5→S11→S5→S2 完整环路"""
    ui = VirtualUI("ui_rules.json")
    # skip S0,S1
    ui._enter_node("S2_STANDBY")

    assert ui.feed_key("KEY_ONOFF", "LONG_PRESS")  # → S3
    assert ui.ctx["current_node"] == "S3_HEAD_SELECT"

    assert ui.feed_key("KEY_HEAD_1", "PRESS")       # → S4
    assert ui.ctx["selected_head"] == 0

    assert ui.feed_key("KEY_POWER_5", "PRESS")      # 设5档
    assert ui.heads[0].power_level == 5

    for _ in range(50): ui.feed_tick_100ms()        # 5s超时
    assert ui.ctx["current_node"] == "S5_WORKING"

    assert ui.feed_key("KEY_PAUSE", "PRESS")         # 暂停
    assert ui.ctx["current_node"] == "S11_PAUSE"

    assert ui.feed_key("KEY_PAUSE", "PRESS")         # 恢复
    assert ui.ctx["current_node"] == "S5_WORKING"

    assert ui.feed_key("KEY_ONOFF", "LONG_PRESS")    # 关机
    assert ui.ctx["current_node"] == "S2_STANDBY"
```

---

## 十、JSON 方案评估

### 为什么 JSON 适合这个项目

| 理由               | 说明                                     |
| ---------------- | -------------------------------------- |
| **改型只需改数据**      | 产品定型后，改型需求（加模式/调显示/改变按键行为）只改 JSON，不动 C |
| **AI 改数据安全**     | JSON 是声明式描述，AI 不需要理解完整 C 上下文就能正确修改     |
| **状态机天然适合 JSON** | 状态+事件+转换 = 三个数组，和 JSON 结构一一对应          |
| **显示元素集中**       | 所有显示描述在一个文件，不会散落 C 各处                  |
| **250 种显示状态**    | DisplayState_t 枚举足够扩展                  |
| **PC 端可独立验证**    | Python 加载同一 JSON，不改 C 就能跑测试            |

### 风险与对策

| 风险                  | 对策                                             |
| ------------------- | ---------------------------------------------- |
| cJSON 占用 3-5KB SRAM | 16K SRAM 足够；解析后立即 cJSON_Delete 释放              |
| JSON 需烧录到 Flash     | json_to_c.py 将 JSON 转 C const char[]，作为只读数据段链接 |
| guard 条件涉及运行时值      | 用字符串引用 C 函数名，查表执行——边界清晰                        |
| 调试不可见               | UART3 trace: `"UI: S2→S3 via KEY_ONOFF"`       |
| 首次构建链路多一步           | json_to_c.py 加入 Makefile/Keil before-build     |
|                     |                                                |

### 和传统硬编码的对比

```
传统 C 硬编码:
  改按键行为 → 找 switch case → 改 C 逻辑 → 全量编译 → 烧录 → 测
  AI 需要理解前后 5000 行 C 上下文

JSON 驱动:
  改按键行为 → 改 JSON 一个字段 → json_to_c.py → 编译 → 烧录 → 测
  AI 只需理解 JSON schema 和当前节点的 10 行 JSON
```

---

## 十一、主循环槽位分配

```
10槽 × 1ms = 10ms 完整周期

0: 定时消息生成 (100ms/1s)
1: 显示刷新 (drv_display)
2: UI引擎 (ui_engine — JSON规则匹配)
3: 按键扫描 (drv_key)
4: 通讯处理 (MODBUS)
5: 烹饪逻辑 (cook — 定时/Boost超时)
6: 保护/故障检测
7-9: 预留
```

---

## 十二、实施步骤

1. **更新 drv_key** — 新增 KEY_HEAD_1~4, KEY_ZONE/PAUSE/TIMER/PLUS/MINUS；调整物理映射 (与规格书对齐)
2. **集成 cJSON** — 加入 lib/cJSON.c/h，验证编译
3. **创建 ui_engine.h** — 接口: Init / OnKey / Tick100ms / Tick1s / GetDisplay
4. **创建 ui_engine.c** — 通用 JSON 执行器: Parse → Match → Action → Display
5. **创建 ui_actions.c** — action + guard 函数库 (全部 ~25 个)
6. **创建 hal_display.h/c** — 8SEG+11COM IO直推扫描 (参考 LED_Drive.c)
7. **创建 drv_display.h/c** — DisplayCmd_t→Disp_DATA[4]→IO_Drive_Disp_BUFF[11]
8. **编写 ui_rules.json** — 主线状态: S0→S1→S2→S3→S4→S5→S8→S11
9. **编写 json_to_c.py** — JSON → C const char[]
10. **集成 main.c** — 新槽位分配 + ui_engine 调用
11. **编写 Python 测试** — 加载同一 JSON，跑全状态机
12. **端到端验证** — 上电→版本→待机→开关→选头→档位→Boost→定时→暂停

### 实施步骤（全局）

1. msg_def.h — 消息ID、结构体定义 ✅
2. msg_scheduler — 调度器实现 ✅
3. hal_timer + hal_uart + hal_gpio ✅
4. hal_comm (UART0 DMA) ✅
5. drv_key ✅ 需更新
6. **cJSON 集成** ← 下一步
7. **ui_engine + ui_actions**
8. **hal_display + drv_display**
9. **ui_rules.json + json_to_c.py**
10. proto_modbus
11. cook + actuator
12. main.c 整合

---

## 十三、JSON 组织方式探索（三种方案对比）

> 同一场景: S2(待机)→长按开关→S3(选头)→按头1→S4(闪烁)→按5档→S5(工作)→按定时→定时设置→确认→定时生效→到期→回待机

### 方案 A: 扁平状态机（基线）

所有状态平铺在一个数组里，定时逻辑内嵌在 S6/S7 节点中。

```jsonc
{
  "states": [
    {
      "node_id": "S2_STANDBY",
      "entry_display": { "smg": "dash", "disp": "DSP_FLASH_500MS", "led_power": "flash" },
      "events": [
        { "key": "KEY_ONOFF", "type": "LONG_PRESS", "timeout_ms": 1500,
          "action": "power_on_init", "next": "S3_HEAD_SELECT" }
      ]
    },
    {
      "node_id": "S5_WORKING",
      "entry_display": { "smg": "power_level", "disp": "DSP_CONSTANT" },
      "events": [
        { "key": "KEY_TIMER", "type": "PRESS",
          "action": "timer_start_setting", "next": "S6_TIMER_SETTING" },
        { "key": "KEY_PAUSE", "type": "PRESS",
          "action": "pause_all", "next": "S11_PAUSE" }
      ]
    },
    {
      "node_id": "S6_TIMER_SETTING",
      "entry_display": { "smg": "timer_total", "disp": "DSP_FLASH_500MS", "dp": "timer" },
      "events": [
        { "key": "KEY_PLUS",  "type": "PRESS", "action": "timer_adjust", "arg": 1 },
        { "key": "KEY_MINUS", "type": "PRESS", "action": "timer_adjust", "arg": -1 },
        { "key": "KEY_TIMER", "type": "PRESS",
          "action": "timer_confirm", "next": "S7_TIMER_ACTIVE" },
        { "key": "KEY_TIMER", "type": "LONG_PRESS",
          "action": "timer_cancel", "next": "S5_WORKING" }
      ],
      "timeout": { "ms": 5000, "action": "timer_confirm", "next": "S7_TIMER_ACTIVE" }
    },
    {
      "node_id": "S7_TIMER_ACTIVE",
      "entry_display": { "smg": "timer_remain", "disp": "DSP_ALT_PWR_TIME", "dp": "timer" },
      "events": [
        { "key": "KEY_TIMER", "type": "PRESS",
          "action": "timer_re_edit", "next": "S6_TIMER_SETTING" }
      ],
      "timeout": { "guard": "timer_remain_zero", "action": "timer_expire", "next": "S2_STANDBY" }
    }
  ]
}
```

**问题**: 定时逻辑(S6+S7)和主流程混在一起。S5_WORKING 需要知道定时完成后回哪。如果再有一个"预约"子流程，S5 的 events[] 又要膨胀。

---

### 方案 B: 主流程 + 子流程分文件

把定时、组合、Boost 抽成**独立 JSON 文件**。主流程不展开子流程内部状态，只写 `"subflow": "timer"`。

**主流程文件 `ui_rules.json`:**

```jsonc
{
  "states": [
    {
      "node_id": "S2_STANDBY",
      "entry_display": { "smg": "dash", "disp": "DSP_FLASH_500MS", "led_power": "flash" },
      "events": [
        { "key": "KEY_ONOFF", "type": "LONG_PRESS", "timeout_ms": 1500,
          "action": "power_on_init", "next": "S3_HEAD_SELECT" }
      ]
    },
    {
      "node_id": "S4_FLASHING",
      "entry_display": { "smg": "power_level", "disp": "DSP_FLASH_500MS" },
      "events": [
        { "key": "KEY_TIMER", "type": "PRESS",
          "action": "subflow_enter",
          "arg": "timer",                    // ← 只引用名字，不展开内部
          "next": "S6_TIMER_SETTING" }
      ]
    },
    {
      "node_id": "S6_TIMER_SETTING",
      "subflow": "timer",                    // ← 这个节点由 timer.json 接管
      "on_exit": "S5_WORKING"               // ← 子流程退出后回这里
    },
    {
      "node_id": "S5_WORKING",
      "entry_display": { "smg": "power_level", "disp": "DSP_CONSTANT" },
      "events": [
        { "key": "KEY_TIMER", "type": "PRESS",
          "action": "subflow_enter", "arg": "timer", "next": "S6_TIMER_SETTING" },
        { "key": "KEY_ZONE", "type": "PRESS",
          "action": "subflow_enter", "arg": "zone", "next": "S9_ZONE_SETTING" }
      ]
    }
  ]
}
```

**子流程文件 `timer.json`:**

```jsonc
{
  "subflow_id": "timer",
  "entry_node": "timer_setting",
  "nodes": {
    "timer_setting": {
      "display": { "smg": "timer_total", "disp": "DSP_FLASH_500MS", "dp": "timer" },
      "events": [
        { "key": "KEY_PLUS",  "type": "PRESS", "action": "timer_adjust", "arg": 1 },
        { "key": "KEY_MINUS", "type": "PRESS", "action": "timer_adjust", "arg": -1 },
        { "key": "KEY_TIMER", "type": "PRESS",  "goto": "timer_active" },
        { "key": "KEY_TIMER", "type": "LONG_PRESS", "action": "timer_cancel", "exit": true }
      ],
      "timeout": { "ms": 5000, "goto": "timer_active" }
    },
    "timer_active": {
      "display": { "smg": "timer_remain", "disp": "DSP_ALT_PWR_TIME", "dp": "timer" },
      "events": [
        { "key": "KEY_TIMER", "type": "PRESS", "goto": "timer_setting" },
        { "key": "KEY_ONOFF", "type": "LONG_PRESS", "action": "power_off_all", "exit": true }
      ],
      "timeout": { "guard": "timer_remain_zero", "action": "timer_expire", "exit": true }
    }
  }
}
```

**优点:**
- 主流程不被子流程细节污染
- 改定时行为只改 `timer.json`
- 子流程可以复用（不同炉头的定时是同一份 JSON）

**仍有问题:** `timer_adjust`、`timer_confirm` 这些 action 还是要写 C 函数。能不能把 action 也 JSON 化？

---

### 方案 C: 元素注册表 —— 函数/显示/子流程/控件 统一配置

**核心理念**: 所有东西都是 "元素"。SMG 显示是一种元素，LED 是一种元素，子流程是一种元素，action 函数也是一种元素。每个元素有 `type` + `params`。

**元素类型:**
| type | 说明 | 示例 |
|------|------|------|
| `smg` | 数码管渲染 | 段码生成、闪烁、交替 |
| `led` | LED 灯控制 | 开关、闪烁 |
| `buzzer` | 蜂鸣器 | 次数、间隔、时长 |
| `timer_op` | 定时器操作 | 加减、确认、取消 |
| `subflow` | 子流程引用 | 调用另一个 JSON 逻辑段 |
| `guard` | 条件判定 | 参数化判定 |

```jsonc
{
  "elements": {
    "disp_standby": {
      "type": "smg",
      "params": { "source": "dash", "mode": "flash", "on_ms": 500, "off_ms": 500 }
    },
    "disp_power_level": {
      "type": "smg",
      "params": { "source": "power_level", "mode": "constant" }
    },
    "disp_flash_selected": {
      "type": "smg",
      "params": { "source": "power_level", "mode": "flash", "on_ms": 500, "off_ms": 500 }
    },
    "disp_timer_setting": {
      "type": "smg",
      "params": { "source": "timer_total", "mode": "flash", "on_ms": 500, "off_ms": 500 }
    },
    "disp_timer_active": {
      "type": "smg",
      "params": { "source": "timer_remain", "mode": "alternate",
                  "alt_source": "power_level", "alt_sec": 5, "main_sec": 5 }
    },
    "disp_marquee_intro": {
      "type": "smg",
      "params": { "source": "text", "text": "HELLO", "mode": "marquee", "speed_ms": 300 }
    },
    "led_power_flash": {
      "type": "led",
      "params": { "led": "power", "mode": "flash", "on_ms": 500, "off_ms": 500 }
    },
    "led_power_on": {
      "type": "led",
      "params": { "led": "power", "mode": "on" }
    },
    "led_timer_on": {
      "type": "led",
      "params": { "led": "timer", "mode": "on" }
    },
    "beep_timer_done": {
      "type": "buzzer",
      "params": { "times": 1, "duration_ms": 1000, "gap_ms": 0 }
    },
    "beep_key_click": {
      "type": "buzzer",
      "params": { "times": 1, "duration_ms": 50, "gap_ms": 0 }
    },
    "action_power_on_init": {
      "type": "action",
      "params": { "func": "power_on_init" }
    },
    "action_set_power": {
      "type": "action",
      "params": { "func": "set_power_level", "arg": "{event.arg}" }
    },
    "action_timer_adjust": {
      "type": "action",
      "params": { "func": "timer_adjust", "arg": "{event.arg}" }
    },
    "action_timer_expire": {
      "type": "action",
      "params": { "func": "timer_expire" }
    },
    "subflow_timer": {
      "type": "subflow",
      "params": { "file": "timer.json", "on_exit": "{parent.next}" }
    }
  },

  "states": [
    {
      "node_id": "S2_STANDBY",
      "display": ["disp_standby", "led_power_flash"],
      "events": [
        { "key": "KEY_ONOFF", "type": "LONG_PRESS", "timeout_ms": 1500,
          "do": ["action_power_on_init"], "next": "S3_HEAD_SELECT" }
      ]
    },
    {
      "node_id": "S4_FLASHING",
      "display": ["disp_flash_selected"],
      "events": [
        { "key": "KEY_POWER_5", "type": "PRESS",
          "do": ["action_set_power"], "next": "S5_WORKING" },
        { "key": "KEY_TIMER", "type": "PRESS",
          "do": ["subflow_timer"], "next": "S6_TIMER_SETTING" }
      ]
    },
    {
      "node_id": "S7_TIMER_ACTIVE",
      "display": ["disp_timer_active", "led_timer_on"],
      "events": [
        { "key": "KEY_TIMER", "type": "PRESS", "next": "S6_TIMER_SETTING" }
      ],
      "timeout": { "guard": "timer_remain_zero",
                   "do": ["beep_timer_done", "action_timer_expire"],
                   "next": "S2_STANDBY" }
      }
    }
  ]
}
```

**优点:**
- 元素可任意组合：`"display": ["disp_standby", "led_power_flash", "beep_key_click"]`
- 新增跑马灯 = 在 elements 里加一个 `disp_marquee_intro`，代码零改动
- 新增蜂鸣模式 = 在 elements 里加一个 `beep_xxx`，代码零改动
- `{event.arg}` 语法让参数从事件传递到 action

**问题:**
- `{event.arg}` 这种模板变量引入了新的解析需求
- elements 平铺在一起，规模大了需要分组

---

### 三种方案对比

| 维度 | A: 扁平状态机 | B: 主流程+子文件 | C: 元素注册表 |
|------|-------------|----------------|-------------|
| 定时逻辑位置 | 内嵌在 S6/S7 | 独立 `timer.json` | 元素 + 独立 `timer.json` |
| 新增跑马灯 | 改 C 代码 | 改 C 代码 | elements 里加一条 |
| 新增蜂鸣模式 | 改 C 代码 | 改 C 代码 | elements 里加一条 |
| 组合显示元素 | 每个状态手写 | 每个状态手写 | 引用元素名即可 |
| 子流程复用 | 无 | 有 | 有 |
| 参数传递 | 无 | 通过 action arg | 模板变量 `{event.arg}` |
| JSON 文件数 | 1 | 1+N(子流程) | 1+N(子流程) |
| C 代码改动频率 | 每次改显示行为 | 每次改显示行为 | 仅新增元素类型时 |

### 推荐方向: B + C 混合

- 用 **C 的元素注册表** 管理显示模式、LED 模式、蜂鸣模式（这些是稳定的底层能力）
- 用 **B 的子文件分拆** 管理定时、组合、Boost 等子逻辑段（这些是可能变型的业务块）
- 主流程 JSON 只描述状态节点 + 引用的元素名 + 事件路由

下一轮可以深入这个方向，写出完整的元素类型定义和主流程 JSON。

---

## 十四、验证方式

- 每个 .c 文件零交叉 include
- Python 测试：加载 UI 同一 JSON，跑规格书全部状态转换
- UART3 trace：实时状态转换日志
- Keil 编译通过 + 开发板运行
