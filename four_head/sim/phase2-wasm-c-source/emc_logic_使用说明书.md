# emc_logic 使用说明书

**适用文件**：`emc_logic.h` + `emc_logic.c`（shared版 v2）  
**适用平台**：PC (DLL) / 8051 (KEIL C51)  
**基准文档**：`煮面炉逻辑层单向数据流文档.md` v1.3  
**版本**：V1.0  

---

## 一、快速开始

```c
#include "emc_logic.h"

void main(void) {
    /* 1. 初始化逻辑层 */
    emc_logic_init();

    /* 2. 注册回调函数（4个） */
    emc_logic_register_callbacks(my_disp_output,   /* 显示输出 */
                                 my_buzzer,         /* 蜂鸣器 */
                                 my_status_change,  /* 状态变化 */
                                 my_get_hw_inputs); /* 硬件输入 */

    /* 3. 启动10ms定时器，周期性调用 */
    while (1) {
        key_disp_cycle();    /* 主循环，由10ms定时器触发 */
        delay_10ms();        /* 伪代码：等待10ms */
    }
}

/* 按键中断中调用 */
void on_key_pressed(uint8_t key_code, uint8_t event) {
    key_input_callback(key_code, event);
}
```

---

## 二、公有API参考

### 2.1 初始化

#### `void emc_logic_init(void)`

- **作用**：清零内部状态、初始化转移表、进入上电自检
- **调用时机**：系统上电后调用一次
- **对应数据流**：架构总览→初始进入S_POWER_ON
- **注意**：必须先于 `key_disp_cycle()` 调用

#### `void emc_logic_register_callbacks(DispOutputCb_t disp_cb, BuzzerCb_t buzz_cb, StatusChangeCb_t status_cb, GetHardwareInputsCb_t hw_cb)`

- **作用**：注册4个外部回调函数
- **调用时机**：`emc_logic_init()` 之后，`key_disp_cycle()` 开始之前
- **回调可为NULL**：不必须全部实现，但 `disp_cb` 和 `hw_cb` 建议必须提供

### 2.2 周期主循环

#### `void key_disp_cycle(void)`

- **作用**：10ms周期主循环，驱动整个逻辑层运行
- **调用频率**：每10ms一次（由外部定时器驱动）
- **内部执行顺序（与数据流文档严格对齐）**：

| Step | 数据流对应 | 代码函数 | 说明 |
|------|-----------|---------|------|
| 0 | 环境输入 → HardwareInputs_t | `s_emc.callbacks.get_hw_inputs` | 统一拉取所有硬件输入 |
| 0.5 | — | 内联 | 水温自动解锁首次上电锁定 |
| 1 | 定时事件 → 系统时间 | `s_emc.system_time_ms += 10` | 更新10ms计数 |
| 2 | 按键驱动回调 → 键码缓冲 → 掩码表检查 → 锁定检查 → 状态机处理 | `is_key_valid()` → `check_lock_conditions()` → `process_key_event()` | 循环处理所有待处理按键 |
| 3 | 状态超时检测 | `check_state_timeout()` | POWER_ON/VERSION/FUNC_SELECT超时 |
| 4 | 临时提示超时检测 | `check_hint_timeout()` | 3秒后恢复原显示 |
| 5 | 烹饪倒计时 | `update_cooking_countdown()` | 出水/加热阶段秒级递减 |
| 6 | 故障检测 | `check_fault_conditions()` | 持续监测fault_code + 烹饪完成锅具检测 |
| 7 | 显示逻辑更新 | `update_display()` → `convert_to_segcode()` → `update_led_state()` | 填充DispBuffer → 转段码 → 设LED |
| 8 | 外部回调输出 | `s_emc.callbacks.disp_output()` | 调用显示输出回调 |

### 2.3 按键输入

#### `void key_input_callback(uint8_t key_code, uint8_t event)`

- **作用**：底层按键驱动检测到事件后调用
- **参数**：
  - `key_code`：按键码（`KeyCode_e`枚举值，0~13）
  - `event`：事件类型（`key_event_t`枚举值）
- **行为**：仅写入循环缓冲，不做状态判断（解耦关键点）
- **调用时机**：按键中断或轮询检测到事件时
- **对应数据流**：输入层→第2.1节 key_input_callback

### 2.4 调试接口

#### `const EmcCtrl_t *emc_logic_get_ctrl(void)`

- **作用**：获取内部状态结构体只读指针
- **用途**：测试/调试时查看当前状态、参数、显示内容
- **注意**：返回指针指向内部静态变量，仅供读取

---

## 三、回调函数实现指南

### 3.1 回调类型总览

| 回调 | 类型定义 | 提供方 | 调用频率 | 说明 |
|------|---------|--------|---------|------|
| `get_hw_inputs` | `GetHardwareInputsCb_t` | **用户必须实现** | 每10ms | 统一获取水温/水位/锅具/故障码 |
| `disp_output` | `DispOutputCb_t` | **用户必须实现** | 每10ms | 输出段码+LED到物理显示 |
| `buzzer` | `BuzzerCb_t` | 可选 | 按需 | 播放蜂鸣器音效 |
| `status_change` | `StatusChangeCb_t` | 可选 | 状态变化时 | 通知外部业务模块 |

### 3.2 硬件输入回调（环境输入→逻辑层）

**对应数据流文档第1部分：【环境输入】**

```c
void my_get_hw_inputs(HardwareInputs_t *p_hw) {
    p_hw->water_temp     = read_temperature();     // 水温 0.1°C
    p_hw->water_level_ok = read_water_level();     // 水位 1=正常
    p_hw->pot_present    = read_pot_sensor();      // 锅具 1=在位
    p_hw->fault_code     = read_fault_register();  // 故障码
}
```

`HardwareInputs_t` 各字段读取时机：
- `water_temp`：S_STANDBY时显示水温用；首次上电锁定判断用
- `water_level_ok`：启动烹饪时检查用（SAF-05）
- `pot_present`：烹饪完成End阶段检测锅具移开用
- `fault_code`：每个周期持续监测

### 3.3 显示输出回调（处理层→输出层）

**对应数据流文档第5~6部分：显示数据流 + 外部回调输出**

```c
void my_disp_output(SegCode_t *p_seg, LedState_t *p_led) {
    /* p_seg->seg[0..3]:   4位数码管段码 */
    /* p_seg->dp_mask:     小数点控制 */
    /* p_seg->colon_mask:  时钟点控制 */
    
    /* p_led->power_led:     电源指示灯 */
    /* p_led->func_leds:     功能灯M1-M10（10bit） */
    /* p_led->add_water_led: 加水指示灯 */
    /* p_led->add_time_led:  加时间指示灯 */
    /* p_led->water_disp_led:水量指示灯 */
    /* p_led->time_disp_led: 时间指示灯 */
    
    write_segments(p_seg->seg, p_seg->dp_mask, p_seg->colon_mask);
    write_leds(p_led);
}
```

**特殊模式处理**（控件层需支持）：
- `special_mode == 1`：跑马灯效果（S_DEMO）
- `special_mode == 2`：闪烁效果（S_SHUTDOWN），500ms周期切换显示/空白
- `dp_mask & 0x04`：DIG2位小数点表示温度度数符号（S_STANDBY）

### 3.4 蜂鸣器回调

```c
void my_buzzer(uint8_t cmd) {
    switch (cmd) {
    case BUZZ_CLICK:  beep(4000, 150);  break;  /* 确认反馈 */
    case BUZZ_DOUBLE: beep_twice();     break;  /* 警告 */
    case BUZZ_LONG:   beep(1000, 500);  break;  /* 错误 */
    case BUZZ_CHORD:  beep_chord();     break;  /* 启动和弦 */
    case BUZZ_ERROR:  beep(1000, 1000); break;  /* 错误长音 */
    }
}
```

蜂鸣器触发时机：

| 场景 | 命令 | 触发位置 |
|------|------|---------|
| 按键确认（开/关机、功能选择） | `BUZZ_CLICK` | `change_state()` 进入各状态时 |
| 参数调整（加水/时间） | `BUZZ_CLICK` | `execute_action()` ACTION_ADJUST_WATER/TIME |
| 水位不足警告 | `BUZZ_DOUBLE` | `execute_action()` ACTION_TRY_START_COOK |
| 启动烹饪 | `BUZZ_CHORD` | `execute_action()` ACTION_TRY_START_COOK |
| 烹饪完成 | `BUZZ_CHORD` | `execute_action()` ACTION_SHOW_END |

### 3.5 状态变化回调

```c
void my_status_change(uint8_t type, void *p_value) {
    switch (type) {
    case STATUS_STATE_CHANGE: /* 状态改变，p_value指向SystemState_e */ break;
    case STATUS_FUNC_SELECT:  /* 功能选择变化，p_value指向func_id */   break;
    case STATUS_COOK_START:   /* 烹饪开始 */                          break;
    case STATUS_COOK_END:     /* 烹饪结束 */                          break;
    case STATUS_FAULT:        /* 故障发生，p_value指向fault_code */    break;
    case STATUS_PARAM_CHANGE: /* 参数变化 */                          break;
    }
}
```

---

## 四、单向数据流 vs 代码位置对照

### 4.1 整体架构对齐

```
数据流文档                        C代码对应
────────────────────────────────────────────────────
【输入层】
  key_input_callback()          → emc_logic.c L260 key_input_callback()
  键码缓冲队列(KeyBuffer_t)      → emc_logic.c L297 s_key_buffer
  get_hw_inputs_cb()            → emc_logic.c L286 s_emc.callbacks.get_hw_inputs
  HardwareInputs_t              → emc_logic.c L328 s_emc.hw

【处理层】key_disp_cycle()每10ms
  ① 拉取硬件输入                → emc_logic.c L284~294
  ② 掩码表查表(is_key_valid)     → emc_logic.c L506 is_key_valid()
  ③ 锁定检查(check_lock)        → emc_logic.c L537 check_lock_conditions()
  ④ 事件映射(map_key)           → emc_logic.c L589 map_key_to_transition_event()
  ⑤ 状态转移表查表               → emc_logic.c L596 s_state_trans_table[][]
  ⑥ 执行动作(execute_action)     → emc_logic.c L612 execute_action()
  ⑦ 状态切换(change_state)       → emc_logic.c L889 change_state()
  ⑧ 显示更新(update_display)     → emc_logic.c L1047 update_display()
  ⑨ 超时检测(check_timeout)      → emc_logic.c L924 check_state_timeout()

【输出层】
  DispBuffer_t                  → emc_logic.c L326 s_emc.disp
  SegCode_t → convert_to_segcode → emc_logic.c L1161 convert_to_segcode()
  LedState_t → update_led_state  → emc_logic.c L1176 update_led_state()
  disp_output_cb()              → emc_logic.c L363 s_emc.callbacks.disp_output
  buzzer_cb()                   → emc_logic.c 内联调用 request_buzzer()
  status_change_cb()            → emc_logic.c 内联调用 notify_status_change()
```

### 4.2 数据结构 vs 规格书节号

```
C代码结构体                    规格书对应章节
────────────────────────────────────────────────
SystemState_e                 第1.1节  状态定义(9个状态)
KeyCode_e                     第2.1节  按键定义(14个按键)
HardwareInputs_t              第4.3节  硬件输入参数
FuncConfig_t                  第10.1节 功能配置表
KeyMask_t / s_state_key_masks 第2.2节  按键锁定规则
DispBuffer_t / SegCode_t      第3.1节  数码管显示映射
LedState_t                    第3.3节  指示灯行为映射
EmcFlags_t                    编程规范 位域结构体
```

### 4.3 状态进入函数 vs 状态

```
状态        进入函数          文件位置
────────────────────────────────────
S_POWER_ON   enter_power_on()  emc_logic.c L705
S_VERSION    enter_version()   emc_logic.c L729
S_SHUTDOWN   enter_shutdown()  emc_logic.c L746
S_DEMO       enter_demo()      emc_logic.c L770
S_STANDBY    enter_standby()   emc_logic.c L787
S_FUNC_SELECT enter_func_select() L802
S_COOKING    enter_cooking()   emc_logic.c L821
S_PAUSE      enter_pause()     emc_logic.c L838
S_FAULT      enter_fault()     emc_logic.c L854
```

### 4.4 数据流关键路径跟踪

**场景：待机→选择功能→调整水量→启动烹饪**

```
用户按键 M1
  → key_input_callback(KEY_M1, KEY_EVENT_SHORT)    [输入层]
  → key_disp_cycle() Step2:
    → is_key_valid(M1, SHORT) 查掩码表 [S_STANDBY].short_mask 含 MASK_KEY_M1
    → check_lock_conditions() 检查首次上电/扫码锁定
    → process_key_event():
      → map_key_to_transition_event() → EV_KEY_M1_SHORT
      → s_state_trans_table[S_STANDBY][EV_KEY_M1_SHORT]
        → next_state=S_FUNC_SELECT, action=ACTION_ENTER_FUNC_SELECT
      → execute_action(ACTION_ENTER_FUNC_SELECT):
        → enter_func_select()  [加载默认参数，调用disp_output刷新显示]
      → change_state(S_FUNC_SELECT)
        → notify_status_change(STATUS_STATE_CHANGE)
```

---

## 五、配置选项

### 5.1 硬件配置宏

在 `emc_logic.h` 中修改（编译前定义）：

```c
/* 是否有独立电源键 */
#define EMC_HAS_POWER_KEY    1    /* 1=独立, 0=START_PAUSE兼作电源键(默认1) */
```

`EMC_HAS_POWER_KEY = 0` 时行为变化：

| 操作 | 有独立电源键(默认) | 无独立电源键 |
|------|-----------------|-------------|
| S_SHUTDOWN 开机 | KEY_POWER 短按 | KEY_START_PAUSE 短按 |
| S_STANDBY 关机 | KEY_POWER 长按 | KEY_START_PAUSE 长按 |
| S_VERSION 快速开机 | KEY_POWER 短按 | KEY_START_PAUSE 短按 |
| S_DEMO 退出 | KEY_POWER 短按 | KEY_START_PAUSE 短按 |

### 5.2 内存优化宏

在编译选项中定义：

```c
/* 将594字节的状态转移表放入Flash(CODE区)，节省RAM */
#define EMC_OPT_TRANS_TABLE_IN_CODE

/* 平台自动检测：__C51__定义时自动启用__xdata */
/* PC编译(DLL)：EMC_XDATA 为空，使用普通内存 */
/* 8051编译：EMC_XDATA = __xdata，使用外部RAM */
```

---

## 六、典型使用场景

### 6.1 PC环境（DLL + Python）

```
Python: 虚拟按键 GUI  →  key_input_callback()
                        ↓
Python: 10ms定时器     →  key_disp_cycle()
                        ↓
                Python注册的回调:
                my_get_hw_inputs   → 虚拟温度/水位/锅具
                my_disp_output     → 虚拟数码管绘制
                my_status_change   → 日志记录
```

### 6.2 8051环境（KEIL C51）

```
硬件按键中断       →  key_input_callback()
硬件定时器(Timer0) →  key_disp_cycle() 每10ms
硬件驱动层实现:
  my_get_hw_inputs → ADC读温度 / GPIO读水位/锅具
  my_disp_output   → 段码写入数码管 / LED控制
  my_buzzer        → PWM输出到蜂鸣器
```

---

## 七、注意事项

1. **初始化顺序不可颠倒**：`emc_logic_init()` → `emc_logic_register_callbacks()` → `key_disp_cycle()`
2. **`key_disp_cycle()` 必须在10ms定时器中调用**，周期误差不超过±1ms
3. **`key_input_callback()` 不能阻塞**，必须在中断上下文中快速返回
4. **回调函数中的 `disp_output` 和 `buzzer` 应尽快返回**，它们在主循环中同步调用
5. **`get_hw_inputs` 回调每10ms调用一次**，实现必须轻量
6. 首次编译 KEIL C51 时，确认 `Keil Options > C51 > Misc Controls` 中添加 `--c99` 以支持指定初始化器
