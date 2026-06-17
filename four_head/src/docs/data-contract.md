# four_head 数据契约 (v2.3)

> 本文档定义模块间的数据交互接口，采用 INPUT_LINK / OUTPUT_LINK 配对管道模式。

---

## 核心概念

```
INPUT  = 你想从别的模块得到的数据
OUTPUT = 你给别的模块提供的数据

INPUT_LINK 与 OUTPUT_LINK 是配对管道：
  - 布局完全一致（字段顺序、类型、大小相同）
  - INPUT_LINK 用 MODULE_INPUT_PARAMS，OUTPUT_LINK 用 MODULE_OUTPUT_PARAMS
  - 通过 Switcher 指针直穿打通（零拷贝）
```

---

## 模块数据流汇总

| 模块 | INPUT 来源 | OUTPUT 去向 |
|------|-----------|-------------|
| **DrvKey** | 无 | → AppHmi, AppCooking, AppSegAlign |
| **AppCommMgr** | 无 | → AppPower, AppCooking, AppProtect |
| **AppProtect** | ← AppCommMgr | → AppPower |
| **AppCooking** | ← DrvKey, AppCommMgr | → AppPower |
| **AppPower** | ← AppCommMgr, AppProtect, AppCooking | → DrvCommMgr, AppHmi |
| **AppHmi** | ← DrvKey, AppPower | → DrvDisplay, DrvBuzzer (@OUTPUT_CALLBACK) |
| **AppSegAlign** | ← DrvKey | 无 |
| **DrvDisplay** | ← AppHmi | 无 |
| **DrvCommMgr** | ← AppPower | 无 |

---

## 数据管道配对表

### DrvKey → AppHmi / AppCooking / AppSegAlign

| 管道 | Producer | Consumer | 数据内容 |
|------|----------|----------|----------|
| `Key_to_Hmi` | DrvKey | AppHmi | 按键事件 (key_code, key_state, head_index) |
| `Key_to_Cooking` | DrvKey | AppCooking | 按键事件 |
| `Key_to_SegAlign` | DrvKey | AppSegAlign | 按键事件 |

**PARAMS 结构**：
```c
typedef struct {
    uint8_t  key_code;      /* KeyCode_t */
    uint8_t  key_state;     /* KEY_STATE_* */
    uint8_t  head_index;    /* 炉头索引 */
    uint8_t  res[1];
} Key_to_Hmi_Params;  /* sizeof = 4 */
```

---

### AppCommMgr → AppPower / AppCooking / AppProtect

| 管道 | Producer | Consumer | 数据内容 |
|------|----------|----------|----------|
| `CommMgr_to_Power` | AppCommMgr | AppPower | 寄存器数据 (regs[22]) |
| `CommMgr_to_Cooking` | AppCommMgr | AppCooking | 寄存器数据 |
| `CommMgr_to_Protect` | AppCommMgr | AppProtect | 寄存器数据 |

**PARAMS 结构**：
```c
typedef struct {
    uint8_t  head_index;      /* 炉头索引 0-3 */
    uint8_t  slave_addr;      /* MODBUS 站号 */
    uint8_t  online;          /* 是否在线 */
    uint8_t  res[1];
    uint16_t regs[22];        /* 寄存器值 */
} CommMgr_to_Power_Params;  /* sizeof = 48 */
```

---

### AppProtect → AppPower

| 管道 | Producer | Consumer | 数据内容 |
|------|----------|----------|----------|
| `Protect_to_Power` | AppProtect | AppPower | 故障数据 (fault) |

**PARAMS 结构**：
```c
typedef struct {
    uint8_t  head_index;      /* 炉头索引 */
    uint8_t  slave_addr;      /* MODBUS 站号 */
    uint16_t fault;           /* ProtectFault_t */
    uint8_t  res[2];
} Protect_to_Power_Params;  /* sizeof = 8 */
```

---

### AppCooking → AppPower

| 管道 | Producer | Consumer | 数据内容 |
|------|----------|----------|----------|
| `Cooking_to_Power` | AppCooking | AppPower | 烹饪状态 (state, power, time) |

**PARAMS 结构**：
```c
typedef struct {
    uint8_t  head_index;      /* 炉头索引 */
    uint8_t  cooking_state;   /* COOKING_STATE_* */
    uint8_t  power_level;     /* 功率档位 0-9 */
    uint8_t  res[1];
    uint16_t target_power;    /* 目标功率 W */
    uint16_t actual_power;    /* 实际功率 W */
    uint32_t cooking_time;    /* 烹饪时间 ms */
} Cooking_to_Power_Params;  /* sizeof = 16 */
```

---

### AppPower → DrvCommMgr / AppHmi

| 管道 | Producer | Consumer | 数据内容 |
|------|----------|----------|----------|
| `Power_to_DrvCommMgr` | AppPower | DrvCommMgr | 功率命令 (target, actual) |
| `Power_to_Hmi` | AppPower | AppHmi | 功率状态 (on, level, actual) |

**PARAMS 结构**：
```c
/* Power_to_DrvCommMgr_Params */
typedef struct {
    uint8_t  head_index;
    uint8_t  slave_addr;
    uint8_t  power_on;
    uint8_t  res[1];
    uint16_t target_power;
    uint16_t actual_power;
} Power_to_DrvCommMgr_Params;  /* sizeof = 12 */

/* Power_to_Hmi_Params */
typedef struct {
    uint8_t  head_index;
    uint8_t  power_on;
    uint8_t  power_level;
    uint8_t  res[1];
    uint16_t actual_power;
} Power_to_Hmi_Params;  /* sizeof = 8 */
```

---

### AppHmi → DrvDisplay / DrvBuzzer

| 管道 | Producer | Consumer | 数据内容 |
|------|----------|----------|----------|
| `Hmi_to_Display` | AppHmi | DrvDisplay | 显示数据 (seg_chars, led_bits) |
| `Hmi_to_Buzzer` | AppHmi | DrvBuzzer | 蜂鸣控制 (@OUTPUT_CALLBACK 例外) |

**PARAMS 结构**：
```c
/* Hmi_to_Display_Params */
typedef struct {
    uint8_t  head_index;
    char     seg_chars[8];
    uint8_t  seg_mode;
    uint8_t  led_bits;
    uint8_t  res[1];
} Hmi_to_Display_Params;  /* sizeof = 16 */

/* Hmi_to_Buzzer_Params */
typedef struct {
    uint8_t  head_index;
    uint8_t  buzz_type;
    uint8_t  buzz_count;
    uint8_t  res[1];
} Hmi_to_Buzzer_Params;  /* sizeof = 4 */
```

---

## io.h 文件清单

| 文件 | 模块 | INPUT | OUTPUT |
|------|------|-------|--------|
| [drv_key_io.h](file:///D:/OBSIDIAN/MOVING%20IH/ZEROLINK/four_head/src/include/drv_key_io.h) | DrvKey | 无 | Key_to_Hmi/Cooking/SegAlign |
| [app_comm_mgr_io.h](file:///D:/OBSIDIAN/MOVING%20IH/ZEROLINK/four_head/src/include/app_comm_mgr_io.h) | AppCommMgr | 无 | CommMgr_to_Power/Cooking/Protect |
| [app_protect_io.h](file:///D:/OBSIDIAN/MOVING%20IH/ZEROLINK/four_head/src/include/app_protect_io.h) | AppProtect | CommMgr_to_Protect | Protect_to_Power |
| [app_cooking_io.h](file:///D:/OBSIDIAN/MOVING%20IH/ZEROLINK/four_head/src/include/app_cooking_io.h) | AppCooking | Key_to_Cooking, CommMgr_to_Cooking | Cooking_to_Power |
| [app_power_io.h](file:///D:/OBSIDIAN/MOVING%20IH/ZEROLINK/four_head/src/include/app_power_io.h) | AppPower | CommMgr/Protect/Cooking_to_Power | Power_to_DrvCommMgr/Hmi |
| [app_hmi_io.h](file:///D:/OBSIDIAN/MOVING%20IH/ZEROLINK/four_head/src/include/app_hmi_io.h) | AppHmi | Key_to_Hmi, Power_to_Hmi | Hmi_to_Display/Buzzer |
| [app_seg_align_io.h](file:///D:/OBSIDIAN/MOVING%20IH/ZEROLINK/four_head/src/include/app_seg_align_io.h) | AppSegAlign | Key_to_SegAlign | 无 |
| [drv_display_io.h](file:///D:/OBSIDIAN/MOVING%20IH/ZEROLINK/four_head/src/include/drv_display_io.h) | DrvDisplay | Hmi_to_Display | 无 |
| [drv_comm_mgr_io.h](file:///D:/OBSIDIAN/MOVING%20IH/ZEROLINK/four_head/src/include/drv_comm_mgr_io.h) | DrvCommMgr | Power_to_DrvCommMgr | 无 |

---

## SKILL 更新记录

已在 [modify-module/SKILL.md](file:///D:/OBSIDIAN/MOVING%20IH/ZEROLINK/.claude/skills/modify-module/SKILL.md) 中添加：
- INPUT_LINK 与 OUTPUT_LINK 配对管道概念
- INPUT = 你想从别的模块得到的数据
- OUTPUT = 你给别的模块提供的数据
- INPUT_LINK 里用 MODULE_INPUT_PARAMS，不是 OUTPUT_PARAMS