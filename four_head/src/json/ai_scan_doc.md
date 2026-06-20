# 项目源码扫描 — AI 数据流分析辅助文档

共扫描 15 个 .c 文件

---

## ../src/app/app_comm_mgr.c

- **Group**: app
- **行数**: 297

### #include 依赖

  - `#include "core/std_module.h"`
  - `#include "app_comm_mgr.h"`
  - `#include <string.h>`
  - `#include <stddef.h>`

### 函数

| 函数名 | 声明 | 有函数体 |
|--------|------|----------|
| send_read_req | `static void send_read_req(uint8_t head_idx)` |   |
| handle_response | `static void handle_response(const uint8_t *rx_data, uint16_t frame_len)` |   |
| AppCommMgr_OnDataUpdate | `void AppCommMgr_OnDataUpdate(uint16_t param, void *data_ptr)` |   |
| AppCommMgr_OnTxDone | `void AppCommMgr_OnTxDone(uint16_t param, void *data_ptr)` |   |
| AppCommMgr_OnPowerCmd | `void AppCommMgr_OnPowerCmd(uint16_t param, void *data_ptr)` |   |
| Init | `static void Init(void)` |   |
| ProcessInput | `static void ProcessInput(void)` |   |
| App_CommMgr_Run | `void App_CommMgr_Run(void)` |   |
| App_CommMgr_Init | `void App_CommMgr_Init(void) { Constructor(); }` | ✓ |

### #define 常量

  - `PROTO_PARSE_OK` = 0      /* 解析成功 (与 PROTO_PARSE_OK 对齐) */
  - `PROTO_FUNC_READ` = 0x03u  /* 读寄存器 (协议知识, 仅本模块)       */
  - `COMM_REG_WORKSTATE` = 0x2014u /* 工作状态 bit0=总开关 bit1=风机 bit4=电压 */
  - `COMM_REG_TARGETPOWER` = 0x2016u /* 目标功率 = W */
  - `APP_COMM_SEND_BUF_SIZE` = 64u
  - `APP_COMM_RX_BUF_SIZE` = 256u
  - `COMM_S_IDLE` = 0u
  - `COMM_S_WAIT_TX` = 1u
  - `COMM_S_WAIT_RX` = 2u

---

## ../src/app/app_protect.c

- **Group**: app
- **行数**: 317

### #include 依赖

  - `#include "core/std_module.h"`
  - `#include "app_protect.h"`
  - `#include <stddef.h>`

### 函数

| 函数名 | 声明 | 有函数体 |
|--------|------|----------|
| check_igbt | `static void check_igbt(uint8_t idx)` |   |
| check_bot | `static void check_bot(uint8_t idx)` |   |
| check_comm | `static void check_comm(uint8_t idx)` |   |
| check_voltage | `static void check_voltage(void)` |   |
| check_hardware | `static void check_hardware(uint8_t idx)` |   |
| AppProtect_OnRegData | `void AppProtect_OnRegData(uint16_t param, void *data_ptr)` |   |
| ProcessInput | `static void ProcessInput(void) {}` | ✓ |
| Init | `static void Init(void)` |   |
| App_Protect_Run | `void App_Protect_Run(void)` |   |
| App_Protect_Init | `void App_Protect_Init(void) { Constructor(); }` | ✓ |

### #define 常量

  - `PROT_REG_COUNT` = 22u
  - `PROT_SLAVE_ADDR_BASE` = 5u
  - `PROT_SLAVE_ADDR_STEP` = 5u

---

## ../src/app/app_power.c

- **Group**: app
- **行数**: 274
- **说明**: 4炉头功率管理

### #include 依赖

  - `#include "core/std_module.h"`
  - `#include "app_power.h"`
  - `#include <string.h>`

### 函数

| 函数名 | 声明 | 有函数体 |
|--------|------|----------|
| igbt_derate | `static uint16_t igbt_derate(PowerCtx_t *ctx, uint16_t power)` |   |
| top_temp_stop | `static uint16_t top_temp_stop(PowerCtx_t *ctx, uint16_t power)` |   |
| soft_start | `static uint16_t soft_start(PowerCtx_t *ctx, uint16_t power)` |   |
| interrupted_heat | `static uint16_t interrupted_heat(PowerCtx_t *ctx, uint16_t power)` |   |
| level_to_watt | `static uint16_t level_to_watt(uint8_t level)` |   |
| ProcessInput | `static void ProcessInput(void)` |   |
| Init | `static void Init(void)` |   |
| AppPower_OnCookingData | `void AppPower_OnCookingData(Para_Grp_t *pOut)` |   |
| AppPower_OnProtectData | `void AppPower_OnProtectData(Para_Grp_t *pOut)` |   |
| AppPower_OnCommMgrData | `void AppPower_OnCommMgrData(Para_Grp_t *pOut)` |   |

---

## ../src/app/app_cooking.c

- **Group**: app
- **行数**: 283
- **说明**: 4炉头烹饪状态机

### #include 依赖

  - `#include "core/std_module.h"`
  - `#include "app_cooking.h"`
  - `#include <string.h>`

### 函数

| 函数名 | 声明 | 有函数体 |
|--------|------|----------|
| detect_boil | `static uint8_t detect_boil(CookCtx_t *ctx)` |   |
| temp_hysteresis | `static uint16_t temp_hysteresis(CookCtx_t *ctx, uint8_t target, uint8_t lv)` |   |
| out_power | `static void out_power(uint8_t h, uint8_t on, uint16_t w, uint8_t lv, uint8_t md, uint16_t tmp)` |   |
| out_disp | `static void out_disp(uint8_t h, uint8_t cmd)` |   |
| on_cmd | `static void on_cmd(uint8_t head, uint8_t cmd, uint8_t menu)` |   |
| cook_tick | `static void cook_tick(void)` |   |
| ProcessInput | `static void ProcessInput(void)` |   |
| Init | `static void Init(void)` |   |
| AppCooking_OnKey | `void AppCooking_OnKey(uint16_t param, void *data_ptr)` |   |
| AppCooking_OnRegData | `void AppCooking_OnRegData(uint16_t param, void *data_ptr)` |   |
| AppCooking_OnTimer1s | `void AppCooking_OnTimer1s(uint16_t param, void *data_ptr)` |   |

---

## ../src/app/app_hmi.c

- **Group**: app
- **行数**: 1888

### #include 依赖

  - `#include "core/std_module.h"`
  - `#include "app_hmi.h"`
  - `#include <string.h>`
  - `#include <stddef.h>`
  - `#include <stdint.h>`

### 函数

| 函数名 | 声明 | 有函数体 |
|--------|------|----------|
| DrvDisplay_OnRefresh | `void DrvDisplay_OnRefresh(uint16_t param, void *data_ptr);` |   |
| head_key_to_index | `static int8_t head_key_to_index(uint8_t key);` |   |
| head_index_to_key | `static uint8_t head_index_to_key(uint8_t idx);` |   |
| digit_key_to_level | `static int8_t digit_key_to_level(uint8_t key);` |   |
| is_head_key | `static uint8_t is_head_key(uint8_t key);` |   |
| AppHmi_OnKey | `void AppHmi_OnKey(uint16_t param, void *data_ptr);` |   |
| AppHmi_OnTimer100ms | `void AppHmi_OnTimer100ms(uint16_t param, void *data_ptr);` |   |
| AppHmi_OnTimer1s | `void AppHmi_OnTimer1s(uint16_t param, void *data_ptr);` |   |
| hmi_match_dynamic_route | `static uint8_t hmi_match_dynamic_route(const HmiRoute_t *route,` |   |
| hmi_execute_action | `static void hmi_execute_action(HmiAction_t action, int8_t param,` |   |
| hmi_apply_actions | `static void hmi_apply_actions(const HmiAction_t *actions,` |   |
| select_head | `static void select_head(uint8_t index);` |   |
| confirm_select | `static void confirm_select(int8_t head_idx);` |   |
| handle_power_key | `static void handle_power_key(uint8_t level);` |   |
| enter_boost | `static void enter_boost(void);` |   |
| exit_boost | `static void exit_boost(int8_t idx);` |   |
| exit_boost_set_power | `static void exit_boost_set_power(uint8_t level);` |   |
| enter_timer_setting | `static void enter_timer_setting(int8_t keep_value);` |   |
| confirm_timer | `static void confirm_timer(int8_t head_idx);` |   |
| cancel_timer_setting | `static void cancel_timer_setting(void);` |   |
| cancel_timer_active | `static void cancel_timer_active(void);` |   |
| handle_timer_adjust | `static void handle_timer_adjust(int8_t delta);` |   |
| go_working | `static void go_working(void);` |   |
| go_powered_off | `static void go_powered_off(void);` |   |
| enter_deep_sleep | `static void enter_deep_sleep(void);` |   |
| toggle_pause | `static void toggle_pause(void);` |   |
| toggle_child_lock | `static void toggle_child_lock(void);` |   |
| resolve_target | `static int8_t resolve_target(void);` |   |
| find_timer_head | `static int8_t find_timer_head(void);` |   |
| push_to_stack | `static void push_to_stack(uint8_t idx);` |   |
| remove_from_stack | `static void remove_from_stack(uint8_t idx);` |   |
| get_stack_top | `static int8_t get_stack_top(void);` |   |
| reassign_hot_head | `static void reassign_hot_head(uint8_t old_idx);` |   |
| clear_all_heads | `static void clear_all_heads(void);` |   |
| reset_idle_timer | `static void reset_idle_timer(void);` |   |
| post_display | `static void post_display(void);` |   |
| derive_seg_mode | `static void derive_seg_mode(void);` |   |
| derive_seg_blink | `static void derive_seg_blink(void);` |   |
| update_head_display | `static void update_head_display(uint8_t idx);` |   |
| update_all_displays | `static void update_all_displays(void);` |   |

### #define 常量

  - `TEST_LED_COUNT` = 16u

---

## ../src/app/app_seg_align.c

- **Group**: app
- **行数**: 530

### #include 依赖

  - `#include "core/std_module.h"`
  - `#include "app_seg_align.h"`
  - `#include <string.h>`
  - `#include <stddef.h>`

### 函数

| 函数名 | 声明 | 有函数体 |
|--------|------|----------|
| calc_crc8 | `static uint8_t calc_crc8(const uint8_t *data, uint8_t len)` |   |
| update_map_crc | `static void update_map_crc(void)` |   |
| logical_to_physical | `static uint8_t logical_to_physical(uint8_t logical_mask)` |   |
| get_step_seg_mask | `static uint8_t get_step_seg_mask(void)` |   |
| adjust_current_seg | `static void adjust_current_seg(void)` |   |
| adjust_current_com | `static void adjust_current_com(void)` |   |
| refresh_alignment_display | `static void refresh_alignment_display(void)` |   |
| serial_putc | `static void serial_putc(char c)` |   |
| serial_putd | `static void serial_putd(uint8_t n)` |   |
| serial_report_status | `static void serial_report_status(void)` |   |
| serial_report_mapping | `static void serial_report_mapping(void)` |   |
| serial_parse_cmd | `static void serial_parse_cmd(const char *cmd)` |   |
| advance_step | `static void advance_step(void)` |   |
| AppSegAlign_IsActive | `uint8_t AppSegAlign_IsActive(void)` |   |
| ProcessInput | `static void ProcessInput(void) {}` | ✓ |
| Init | `static void Init(void)` |   |
| AppSegAlign_Init | `void AppSegAlign_Init(void) { Constructor(); }` | ✓ |
| AppSegAlign_Run | `void AppSegAlign_Run(void)` |   |
| AppSegAlign_OnKey | `void AppSegAlign_OnKey(uint16_t param, void *data_ptr)` |   |

### #define 常量

  - `SEG_BIT_A` = 0u
  - `SEG_BIT_B` = 1u
  - `SEG_BIT_C` = 2u
  - `SEG_BIT_D` = 3u
  - `SEG_BIT_E` = 4u
  - `SEG_BIT_F` = 5u
  - `SEG_BIT_G` = 6u
  - `SEG_BIT_DP` = 7u
  - `SEG_MASK_A` = (1u << SEG_BIT_A)    /* 0x01 */
  - `SEG_MASK_B` = (1u << SEG_BIT_B)    /* 0x02 */
  - `SEG_MASK_C` = (1u << SEG_BIT_C)    /* 0x04 */
  - `SEG_MASK_D` = (1u << SEG_BIT_D)    /* 0x08 */
  - `SEG_MASK_E` = (1u << SEG_BIT_E)    /* 0x10 */
  - `SEG_MASK_F` = (1u << SEG_BIT_F)    /* 0x20 */
  - `SEG_MASK_G` = (1u << SEG_BIT_G)    /* 0x40 */
  - `SEG_MASK_DP` = (1u << SEG_BIT_DP)   /* 0x80 */
  - `SEG_DIGIT_0` = (SEG_MASK_A|SEG_MASK_B|SEG_MASK_C|SEG_MASK_D|SEG_MASK_E|SEG_MASK_F)
  - `SEG_DIGIT_1` = (SEG_MASK_B|SEG_MASK_C)
  - `SEG_DIGIT_8` = (SEG_MASK_A|SEG_MASK_B|SEG_MASK_C|SEG_MASK_D|SEG_MASK_E|SEG_MASK_F|SEG_MASK_G)
  - `ALIGN_STATE_IDLE` = 0u

---

## ../src/cfg/hmi_data.c

- **Group**: cfg
- **行数**: 276

### #include 依赖

  - `#include "../app/app_hmi.h"`

---

## ../src/drv/drv_key.c

- **Group**: drv
- **行数**: 217

### #include 依赖

  - `#include "core/std_module.h"`
  - `#include "drv_key.h"`
  - `#include "../hal/hal_key.h"`
  - `#include <stddef.h>`

### 函数

| 函数名 | 声明 | 有函数体 |
|--------|------|----------|
| Key_Lookup | `static uint8_t Key_Lookup(uint32_t phy_mask)` |   |
| ProcessInput | `static void ProcessInput(void) {}` | ✓ |
| Init | `static void Init(void)` |   |
| Drv_Key_Init | `void Drv_Key_Init(void) { Constructor(); }` | ✓ |
| Key_PostEvent | `static void Key_PostEvent(uint8_t key_code, uint8_t key_state)` |   |
| Key_Release | `static void Key_Release(void)` |   |
| Drv_Key_Scan | `void Drv_Key_Scan(void)` |   |

### #define 常量

  - `TK_CH(n)` = (1UL << (n))  /* 通道n的位掩码                         */
  - `DF_TK1` = TK_CH(28)
  - `DF_TK2` = TK_CH(29)
  - `DF_TK3` = TK_CH(24)
  - `DF_TK4` = TK_CH(30)
  - `DF_TK5` = TK_CH(23)
  - `DF_TK6` = TK_CH(22)
  - `DF_TK7` = TK_CH(10)
  - `DF_TK8` = TK_CH(11)
  - `DF_TK9` = TK_CH(13)
  - `DF_TK10` = TK_CH(7)
  - `DF_TK11` = TK_CH(8)
  - `DF_TK12` = TK_CH(21)
  - `DF_TK13` = TK_CH(20)
  - `DF_TK14` = TK_CH(19)
  - `DF_TK15` = TK_CH(18)
  - `DF_TK16` = TK_CH(17)
  - `DF_TK17` = TK_CH(16)
  - `DF_TK18` = TK_CH(15)
  - `DF_TK19` = TK_CH(14)

---

## ../src/drv/drv_display.c

- **Group**: drv
- **行数**: 500

### #include 依赖

  - `#include "core/std_module.h"`
  - `#include "drv_display.h"`
  - `#include "../hal/hal_smg.h"`
  - `#include "../hal/hal_display.h"`
  - `#include <string.h>`
  - `#include <stddef.h>`

### 函数

| 函数名 | 声明 | 有函数体 |
|--------|------|----------|
| Drv_Display_SetRawLEDs | `void Drv_Display_SetRawLEDs(uint8_t io8, uint8_t io9, uint8_t io10)` |   |
| Drv_Display_ShowRawSMG | `void Drv_Display_ShowRawSMG(const char *upper, const char *lower)` |   |
| apply_blink_dp | `static void apply_blink_dp(void)` |   |
| sync_hmi_display | `static void sync_hmi_display(const DisplayFrame_t *disp)` |   |
| DrvDisplay_OnRefresh | `void DrvDisplay_OnRefresh(uint16_t param, void *data_ptr)` |   |
| DrvSegAlign_BlockHmi | `void DrvSegAlign_BlockHmi(uint8_t block)` |   |
| DrvSegAlign_WriteCom | `void DrvSegAlign_WriteCom(uint8_t com, uint8_t seg_mask)` |   |
| ProcessInput | `static void ProcessInput(void) {}` | ✓ |
| Init | `static void Init(void)` |   |
| flash_sync | `static void flash_sync(void)` |   |
| Drv_Display_ShowKey | `void Drv_Display_ShowKey(uint8_t key_code, uint8_t key_state)` |   |
| refresh_display | `static void refresh_display(void)` |   |
| Drv_Display_Update | `void Drv_Display_Update(void)` |   |
| Drv_Display_Scan | `void Drv_Display_Scan(void)` |   |
| Drv_Display_Init | `void Drv_Display_Init(void) { Constructor(); }` | ✓ |

### #define 常量

  - `LED_LVL_COUNT` = 10
  - `LED_HS_COUNT` = 4

---

## ../src/drv/drv_buzzer.c

- **Group**: drv
- **行数**: 433

### #include 依赖

  - `#include "core/std_module.h"`
  - `#include "drv_buzzer.h"`
  - `#include "../hal/hal_buzzer.h"`
  - `#include <stddef.h>`

### 函数

| 函数名 | 声明 | 有函数体 |
|--------|------|----------|
| Buzz_MY_SetupNote | `static void Buzz_MY_SetupNote(void)` |   |
| Buzz_Mode_MY | `static void Buzz_Mode_MY(uint8_t mode)` |   |
| Buzz_Dispose_MY | `static void Buzz_Dispose_MY(void)` |   |
| Buzz_Dispose_Set | `static void Buzz_Dispose_Set(uint8_t count, uint8_t hz_timer, uint8_t jiange)` |   |
| Buzz_Mode | `static void Buzz_Mode(uint8_t mode)` |   |
| Buzz_Dispose | `static void Buzz_Dispose(void)` |   |
| DrvBuzzer_OnCtrl | `void DrvBuzzer_OnCtrl(uint16_t param, void *data_ptr)` |   |
| ProcessInput | `static void ProcessInput(void) {}` | ✓ |
| Init | `static void Init(void)` |   |
| Drv_Buzzer_Select | `void Drv_Buzzer_Select(uint8_t out_sel, uint8_t mode)` |   |
| Drv_Buzzer_Timer_1ms | `void Drv_Buzzer_Timer_1ms(void)` |   |
| Drv_Buzzer_Init | `void Drv_Buzzer_Init(void) { Constructor(); }` | ✓ |

---

## ../src/drv/drv_comm_mgr.c

- **Group**: drv
- **行数**: 122

### #include 依赖

  - `#include "core/std_module.h"`
  - `#include "drv_comm_mgr.h"`
  - `#include "drv_comm.h"`
  - `#include <string.h>`
  - `#include <stddef.h>`

### 函数

| 函数名 | 声明 | 有函数体 |
|--------|------|----------|
| AppCommMgr_OnTxDone | `void AppCommMgr_OnTxDone(uint16_t param, void *data_ptr);` |   |
| AppCommMgr_OnDataUpdate | `void AppCommMgr_OnDataUpdate(uint16_t param, void *data_ptr);` |   |
| DrvCommMgr_OnSendReq | `void DrvCommMgr_OnSendReq(uint16_t param, void *data_ptr)` |   |
| ProcessInput | `static void ProcessInput(void) {}` | ✓ |
| Init | `static void Init(void)` |   |
| Drv_CommMgr_Update | `void Drv_CommMgr_Update(void)` |   |
| Drv_CommMgr_Init | `void Drv_CommMgr_Init(void) { Constructor(); }` | ✓ |

### #define 常量

  - `DRV_COMM_SEND_BUF_SIZE` = 64u
  - `DRV_COMM_RX_BUF_SIZE` = 256u

---

## ../src/drv/drv_comm.c

- **Group**: drv
- **行数**: 117

### #include 依赖

  - `#include "drv_comm.h"`
  - `#include "../hal/hal_comm.h"`
  - `#include "../hal/hal_uart.h"`
  - `#include <stdarg.h>`
  - `#include <stdio.h>`

### 函数

| 函数名 | 声明 | 有函数体 |
|--------|------|----------|
| Drv_Comm_Init | `void Drv_Comm_Init(void)` |   |
| Drv_Comm_Send | `uint8_t Drv_Comm_Send(const uint8_t *data, uint16_t len)` |   |
| Drv_Comm_Available | `uint16_t Drv_Comm_Available(void)` |   |
| Drv_Comm_Read | `uint16_t Drv_Comm_Read(uint8_t *buf, uint16_t max_len)` |   |
| Drv_Comm_Flush | `void Drv_Comm_Flush(void)` |   |
| Drv_Comm_TxDone | `uint8_t Drv_Comm_TxDone(void)` |   |
| Drv_Comm_RecvPoll | `uint16_t Drv_Comm_RecvPoll(void)` |   |
| Drv_Comm_UART_ISR | `void Drv_Comm_UART_ISR(void)` |   |
| Drv_Comm_DMA_ISR | `void Drv_Comm_DMA_ISR(void)` |   |
| Drv_Comm_Debug_Init | `void Drv_Comm_Debug_Init(void)` |   |
| Drv_Comm_Debug_PutChar | `void Drv_Comm_Debug_PutChar(uint8_t ch)` |   |
| Drv_Comm_Debug_Print | `void Drv_Comm_Debug_Print(const char *str)` |   |
| Drv_Comm_Debug_HexDump | `void Drv_Comm_Debug_HexDump(const uint8_t *data, uint16_t len)` |   |
| Drv_Comm_Debug_Printf | `int Drv_Comm_Debug_Printf(const char *fmt, ...)` |   |
| Drv_Comm_Debug_Print_NB | `void Drv_Comm_Debug_Print_NB(const char *str)` |   |
| Drv_Comm_Debug_TxBusy | `uint8_t Drv_Comm_Debug_TxBusy(void)` |   |
| Drv_Comm_Debug_Flush | `void Drv_Comm_Debug_Flush(void)` |   |
| Drv_Comm_Debug_ISR | `void Drv_Comm_Debug_ISR(void)` |   |

---

## ../src/proto/proto_modbus.c

- **Group**: proto
- **行数**: 258

### #include 依赖

  - `#include "proto_modbus.h"`

### 函数

| 函数名 | 声明 | 有函数体 |
|--------|------|----------|
| Proto_Modbus_CRC16 | `uint16_t Proto_Modbus_CRC16(const uint8_t *data, uint16_t len)` |   |
| append_crc | `static uint16_t append_crc(uint8_t *buf, uint16_t pos)` |   |
| Proto_Modbus_BuildRead | `uint16_t Proto_Modbus_BuildRead(uint8_t slave_addr, uint16_t reg_addr,` |   |
| Proto_Modbus_BuildWriteSingle | `uint16_t Proto_Modbus_BuildWriteSingle(uint8_t slave_addr, uint16_t reg_addr,` |   |
| Proto_Modbus_BuildWriteMulti | `uint16_t Proto_Modbus_BuildWriteMulti(uint8_t slave_addr, uint16_t reg_addr,` |   |
| Proto_Modbus_Parse | `int8_t Proto_Modbus_Parse(const uint8_t *rx_buf, uint16_t rx_len,` |   |
| Proto_BuildRead | `uint16_t Proto_BuildRead(uint8_t slave_addr, uint16_t reg_addr,` |   |
| Proto_Parse | `int8_t Proto_Parse(const uint8_t *rx_buf, uint16_t rx_len,` |   |
| Proto_BuildWriteSingle | `uint16_t Proto_BuildWriteSingle(uint8_t slave_addr, uint16_t reg_addr,` |   |

---

## ../src/main.c

- **Group**: src
- **行数**: 195

### #include 依赖

  - `#include "sc32_conf.h"`
  - `#include "system_sc32f1xxx.h"`
  - `#include "hal/hal_timer.h"`
  - `#include "hal/hal_uart.h"`
  - `#include "hal/hal_gpio.h"`
  - `#include "hal/hal_buzzer.h"`
  - `#include <stddef.h>`
  - `#include "drv/drv_key.h"`
  - `#include "drv/drv_display.h"`
  - `#include "drv/drv_buzzer.h"`
  - `#include "drv/drv_comm_mgr.h"`
  - `#include "app/app_comm_mgr.h"`
  - `#include "app/app_protect.h"`
  - `#include "app/app_power.h"`
  - `#include "app/app_cooking.h"`
  - `#include "app/app_hmi.h"`
  - `#include "app/app_seg_align.h"`
  - `#include "core/data_switcher.h"`

### 函数

| 函数名 | 声明 | 有函数体 |
|--------|------|----------|
| Key_OnEvent | `static void Key_OnEvent(MsgId_t id, uint16_t param, void *data_ptr)` |   |
| Heartbeat_OnTimer100ms | `static void Heartbeat_OnTimer100ms(MsgId_t id, uint16_t param, void *data_ptr)` |   |
| Slot_TimerTick | `static void Slot_TimerTick(void)` |   |
| ExecSlot_Run | `static void ExecSlot_Run(void)` |   |
| main | `int main(void)` |   |

---

## ../src/core/data_switcher.c

- **Group**: src
- **行数**: 203
- **说明**: Data Switcher — PULL 路由调度器

### #include 依赖

  - `#include "core/std_module.h"`
  - `#include "data_switcher.h"`

### 函数

| 函数名 | 声明 | 有函数体 |
|--------|------|----------|
| AppPower_GetIO | `void AppPower_GetIO(Para_Grp_t **ppIn, Para_Grp_t **ppOut, void (**ppDoWork)(void));` |   |
| AppHmi_GetIO | `void AppHmi_GetIO(Para_Grp_t **ppIn, Para_Grp_t **ppOut, void (**ppDoWork)(void));` |   |
| AppCommMgr_GetIO | `void AppCommMgr_GetIO(Para_Grp_t **ppIn, Para_Grp_t **ppOut, void (**ppDoWork)(void));` |   |
| AppProtect_GetIO | `void AppProtect_GetIO(Para_Grp_t **ppIn, Para_Grp_t **ppOut, void (**ppDoWork)(void));` |   |
| AppSegAlign_GetIO | `void AppSegAlign_GetIO(Para_Grp_t **ppIn, Para_Grp_t **ppOut, void (**ppDoWork)(void));` |   |
| DrvKey_GetIO | `void DrvKey_GetIO(Para_Grp_t **ppIn, Para_Grp_t **ppOut, void (**ppDoWork)(void));` |   |
| DrvCommMgr_GetIO | `void DrvCommMgr_GetIO(Para_Grp_t **ppIn, Para_Grp_t **ppOut, void (**ppDoWork)(void));` |   |
| DrvBuzzer_GetIO | `void DrvBuzzer_GetIO(Para_Grp_t **ppIn, Para_Grp_t **ppOut, void (**ppDoWork)(void));` |   |
| DrvDisplay_GetIO | `void DrvDisplay_GetIO(Para_Grp_t **ppIn, Para_Grp_t **ppOut, void (**ppDoWork)(void));` |   |
| AppPower_OnCookingData | `void AppPower_OnCookingData(Para_Grp_t *pOut);` |   |
| AppPower_OnProtectData | `void AppPower_OnProtectData(Para_Grp_t *pOut);` |   |
| AppPower_OnCommMgrData | `void AppPower_OnCommMgrData(Para_Grp_t *pOut);` |   |
| AppCooking_OnRegData | `void AppCooking_OnRegData(uint16_t param, void *data_ptr);` |   |
| AppProtect_OnRegData | `void AppProtect_OnRegData(uint16_t param, void *data_ptr);` |   |
| AppHmi_OnKey | `void AppHmi_OnKey(uint16_t param, void *data_ptr);` |   |
| AppCooking_OnKey | `void AppCooking_OnKey(uint16_t param, void *data_ptr);` |   |
| AppSegAlign_OnKey | `void AppSegAlign_OnKey(uint16_t param, void *data_ptr);` |   |
| DrvDisplay_OnRefresh | `void DrvDisplay_OnRefresh(uint16_t param, void *data_ptr);` |   |
| DrvBuzzer_OnCtrl | `void DrvBuzzer_OnCtrl(uint16_t param, void *data_ptr);` |   |
| _register | `static void _register(uint8_t idx, void (*pDoWork)(void), Para_Grp_t *pOut)` |   |
| Switcher_Init | `void Switcher_Init(void)` |   |
| _route_comm_mgr | `static void _route_comm_mgr(void)` |   |
| _route_protect | `static void _route_protect(void)` |   |
| _route_key | `static void _route_key(void)` |   |
| AppHmi_OnOutput | `void AppHmi_OnOutput(Para_Grp_t *pOut)` |   |
| _route_hmi | `static void _route_hmi(void)` |   |
| Switcher_Run | `void Switcher_Run(void)` |   |

### #define 常量

  - `MAX_MODULES` = 16

---
