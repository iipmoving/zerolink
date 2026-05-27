# LED 物理测试模式归档

> 归档日期：2026-05-26
> 会话范围：`Claude/drv/drv_display.c`, `Claude/drv/drv_display.h`, `Claude/app/app_hmi.c`
> 验证依据：用户硬件实测

## 一、做了什么

### 1. LED 物理测试模式重写（`app_hmi.c`）

原有 `HMI_DEBUG_KEYS` 测试模式将按键码和事件类型显示在数码管上，但不涉及 LED。重写后：

- **LED 物理扫描表** `s_phys_leds[16]`：按 COM9→COM10→COM11, bit0→bit7 顺序列出 16 个已确认存在的物理 LED 位
- **LED 列表**（16 条）：
  - COM9 (IO[8]): b0, b1, b2, b3, b4, b5, b7（b6 不存在）
  - COM10 (IO[9]): b0, b1, b4, b5（b2,b3,b6,b7 不存在）
  - COM11 (IO[10]): b0, b1, b3, b4, b7（b2,b5,b6 不存在）
- **操作**：长按开关进入，+/- 切换序号(0-15)，再次长按退出
- **界面**：上排序号，下排物理位标识（如 `C9b0`）
- **IO 操作用 `Drv_Display_SetRawLEDs` 直写**，不经过 HMI 映射层

### 2. 新增 `Drv_Display_ShowRawSMG`（`drv_display.c/h`）

- 写入 SMG 段码到 `s_io_work[0..7]`，不碰 LED 字节
- 同步更新 `s_disp_upper/lower`，防止 `Drv_Display_Update` 100ms 周期用旧值覆盖

### 3. 新增 `Drv_Display_SetRawLEDs`（`drv_display.c/h`）

- 直接设置 `s_io_work[8..10]`

### 4. `s_mode_map` 更新（硬件验证确认）

| LED | IO | Mask | 物理位 |
|-----|-----|------|--------|
| POWER | 10 | 0x08 | COM11 bit3 |
| TIMER | 10 | 0x10 | COM11 bit4 |
| PAUSE | 10 | 0x01 | COM11 bit0 |
| CHILD_LOCK | 10 | 0x80 | COM11 bit7 |
| GROUP | 10 | 0x02 | COM11 bit1（预留，sync 中常清）|

### 5. `s_lvl_map` 更新（硬件验证确认）

| 档位 | IO | Mask | 物理位 |
|------|----|------|--------|
| L0 | 9 | 0x20 | COM10 bit5 |
| L1 | 9 | 0x02 | COM10 bit1 |
| L2 | 9 | 0x01 | COM10 bit0 |
| L3 | 9 | 0x10 | COM10 bit4 |
| L4 | 8 | 0x10 | COM9 bit4 |
| L5 | 8 | 0x01 | COM9 bit0 |
| L6 | 8 | 0x02 | COM9 bit1 |
| L7 | 8 | 0x20 | COM9 bit5 |
| L8 | 8 | 0x80 | COM9 bit7 |
| L9 | 8 | 0x08 | COM9 bit3 |

### 6. 隐式声明修正

`app_hmi.c` 添加 `#include "../drv/drv_display.h"`

### 7. IO 越界保护

`sync_hmi_display` 中 LED 查表加 `io >= 8u` 判断，防止 LED 写入 SMG 字节

## 二、文件状态

| 文件 | 状态 |
|------|------|
| `Claude/drv/drv_display.h` | 稳定，SetRawLEDs/ShowRawSMG 声明 |
| `Claude/drv/drv_display.c` | 稳定，s_mode_map/s_lvl_map 注释对齐 |
| `Claude/app/app_hmi.c` | 稳定，HMI_DEBUG_KEYS LED 扫描测试 |

## 三、遗留问题

1. GROUP LED `sync_hmi_display` 中强制置 0，功能预留待后续扩展
2. 测试模式退出后显示 `00 non` 而非全灭（低优先级）
3. 若后续硬件改版需更新三张映射表：`s_mode_map`, `s_lvl_map`, `s_hs_map`
4. 测试模式由 `#define HMI_DEBUG_KEYS` 控制，发布时注释即可移除
