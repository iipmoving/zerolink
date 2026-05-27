/**
 * drv_display.c —— 显示驱动层实现
 *
 * 依赖: drv_display.h + SMG_Disp_General_Lib.h + hal_display.h
 * 层级: DRV —— SMG库封装 + IO缓冲管理
 *
 * 缓冲布局 (与参考程序 Disp_data_Exchange_Hardware 一致):
 *   IO[0..1] = Z1 SMG (上左)  ← Disp_Upper[0..1]
 *   IO[2..3] = Z2 SMG (上右)  ← Disp_Upper[2..3]
 *   IO[4..5] = Z3 SMG (下左)  ← Disp_Lower[0..1]
 *   IO[6..7] = Z4 SMG (下右)  ← Disp_Lower[2..3]
 *   IO[8..10]= LED组
 *
 * 双缓冲:
 *   s_io_work[] — Update写入(10ms槽位)
 *   s_io_buff[] — Scan读出(每1ms)
 *   提交: Scan发现dirty标志 → memcpy work→buff → 清dirty
 *
 * 测试模式:
 *   上4位 = 按键码(hex)
 *   下4位 = 按键类型字符串
 */
#include "drv_display.h"
#include "../hal/hal_smg.h"
#include "../hal/hal_display.h"
#include <string.h>
#include <stddef.h>

/* 独立声明的显示帧类型 — 与 APP 层 HmiDisplayCache_t 布局一致，
 * 通过 MSG_DISPLAY_REFRESH 的 void* 传递。
 * 两份声明互不 include，由 interface_map.h 文档保证一致性。 */
typedef struct {
    char    seg_chars[8];
    uint8_t seg_blink[4];
    uint8_t seg_mode;
    int8_t  hot_head_idx;
    uint8_t leds_power;
    uint8_t leds_timer;
    uint8_t leds_pause;
    uint8_t leds_child_lock;
    uint8_t leds_head_select[4];
    uint8_t leds_power_level[10];
} DisplayFrame_t;

/* ========== 显示缓冲 — 双缓冲 ========== */
static unsigned char s_disp_upper[4];   /* 上4位 SMG缓冲 */
static unsigned char s_disp_lower[4];   /* 下4位 SMG缓冲 */
static unsigned char s_io_buff[11];     /* 扫描缓冲 (Scan只读) */
static unsigned char s_io_work[11];     /* 工作缓冲 (Update写入) */
static uint8_t       s_io_dirty;        /* 工作缓冲有新数据,待提交 */
static uint8_t       s_blink_mask[8];   /* 0xFF=全数字闪烁, 0x80=仅DP闪烁 */
static unsigned char  s_disp_upper_clean[4]; /* 无闪烁的干净段码(上) */
static unsigned char  s_disp_lower_clean[4]; /* 无闪烁的干净段码(下) */

/* ========== 按键显示缓存 ========== */
static uint8_t s_cur_key_code;
static uint8_t s_cur_key_state;
static uint8_t s_disp_dirty;            /* 有新的按键数据待刷新 */

/* ========== 扫描状态 ========== */
static uint8_t s_scan_com;              /* 当前扫描COM位 0..10 */
static uint8_t s_prev_com;              /* 上一COM位 (0xFF=首次) */
static uint8_t s_flash_toggle;          /* 闪烁相位 0/1 */
static uint8_t s_flash_period_10ms;     /* 闪烁半周期, 单位10ms (从hmi_cfg读取) */
static uint8_t s_update_cnt;            /* 自计数: 10次×10ms=100ms */
static uint8_t s_flash_cnt;             /* 自计数: 按 s_flash_period_10ms 翻转 */

/* ========== HMI LED 位映射: 逻辑LED → IO[8..10] 物理位 ========== */
/*                                                                   */
/* 物理布局 (COM9~11, bit0~7):                                       */
/*   IO[8]  (COM9):  b0..b7                                          */
/*   IO[9]  (COM10): b0..b7                                          */
/*   IO[10] (COM11): b0..b7                                          */
/*                                                                   */
/* 验证: #define HMI_DEBUG_KEYS → 长按开关进入LED测试                 */
/*   上屏=序号(0~15), 下屏=物理位(如C9b0), +/-切灯                    */

/* ========== 一、模式灯枚举 → 物理位映射 ========== */
/* 4个独立状态指示灯, 各对应一个物理LED */
typedef enum {
    LED_MODE_POWER      = 0,
    LED_MODE_TIMER      = 1,
    LED_MODE_PAUSE      = 2,
    LED_MODE_CHILD_LOCK = 3,
    LED_MODE_GROUP      = 4,
    LED_MODE_COUNT
} LedMode_t;

static const struct { uint8_t io; uint8_t mask; } s_mode_map[LED_MODE_COUNT] = {
    { 10, 0x08 },  /* POWER:      COM11 bit3 */
    { 10, 0x10 },  /* TIMER:      COM11 bit4 */
    { 10, 0x01 },  /* PAUSE:      COM11 bit0 */
    { 10, 0x80 },  /* CHILD_LOCK: COM11 bit7 */
    { 10, 0x02 },  /* GROUP:      COM11 bit1 (预留, 常清) */
};

/* ========== 二、档位灯 → 物理位映射 ========== */
#define LED_LVL_COUNT 10

static const struct { uint8_t io; uint8_t mask; } s_lvl_map[LED_LVL_COUNT] = {
    {  9, 0x20 },  /* L0: COM10 bit5 */
    {  9, 0x02 },  /* L1: COM10 bit1 */
    {  9, 0x01 },  /* L2: COM10 bit0 */
    {  9, 0x10 },  /* L3: COM10 bit4 */
    {  8, 0x10 },  /* L4: COM9  bit4 */
    {  8, 0x01 },  /* L5: COM9  bit0 */
    {  8, 0x02 },  /* L6: COM9  bit1 */
    {  8, 0x20 },  /* L7: COM9  bit5 */
    {  8, 0x80 },  /* L8: COM9  bit7 */
    {  8, 0x08 },  /* L9: COM9  bit3 */
};

/* ========== 三、炉头选中灯枚举 → 物理位映射 ========== */
#define LED_HS_COUNT 4

static const struct { uint8_t io; uint8_t mask; } s_hs_map[LED_HS_COUNT] = {
    {  0, 0x00 },  /* Z1: 无物理LED */
    {  0, 0x00 },  /* Z2: 无物理LED */
    {  0, 0x00 },  /* Z3: 无物理LED */
    {  0, 0x00 },  /* Z4: 无物理LED */
};

/* ========== 直接设置 LED IO 字节 (绕过HMI缓存, 用于硬件验证) ========== */
void Drv_Display_SetRawLEDs(uint8_t io8, uint8_t io9, uint8_t io10)
{
    s_io_work[8]  = io8;
    s_io_work[9]  = io9;
    s_io_work[10] = io10;
    s_io_dirty = 1u;
}

/* SMG直接更新 (绕过HMI消息, 仅写s_io_work[0..7], 不动LED) */
void Drv_Display_ShowRawSMG(const char *upper, const char *lower)
{
    unsigned char seg[4];

    HAL_SMG_FontASCII(seg, upper);
    s_io_work[0] = s_disp_upper[0] = seg[0];
    s_io_work[1] = s_disp_upper[1] = seg[1];
    s_io_work[2] = s_disp_upper[2] = seg[2];
    s_io_work[3] = s_disp_upper[3] = seg[3];

    HAL_SMG_FontASCII(seg, lower);
    s_io_work[4] = s_disp_lower[0] = seg[0];
    s_io_work[5] = s_disp_lower[1] = seg[1];
    s_io_work[6] = s_disp_lower[2] = seg[2];
    s_io_work[7] = s_disp_lower[3] = seg[3];

    s_io_dirty = 1u;
}

/* ========== Blink: 闪烁掩码+相位 → SMG字节 ========== */
/* s_blink_mask[i] == 0xFF: 数字闪烁, DP保留 (off-phase 清零bits0-6)  */
/* s_blink_mask[i] == 0x80: 仅DP闪烁 (off-phase 清除 bit7)           */
static void apply_blink_dp(void)
{
    uint8_t i;
    for (i = 0; i < 4; i++) {
        if (s_blink_mask[i] == 0xFF) {
            if (s_flash_toggle) {
                s_disp_upper[i] = s_disp_upper_clean[i] & 0x80u;
            } else {
                s_disp_upper[i] = s_disp_upper_clean[i];
            }
        } else if (s_blink_mask[i] == 0x80) {
            if (s_flash_toggle) {
                s_disp_upper[i] &= 0x7Fu;
            } else {
                s_disp_upper[i] |= 0x80u;
            }
        }
    }
    for (i = 0; i < 4; i++) {
        if (s_blink_mask[i + 4] == 0xFF) {
            if (s_flash_toggle) {
                s_disp_lower[i] = s_disp_lower_clean[i] & 0x80u;
            } else {
                s_disp_lower[i] = s_disp_lower_clean[i];
            }
        } else if (s_blink_mask[i + 4] == 0x80) {
            if (s_flash_toggle) {
                s_disp_lower[i] &= 0x7Fu;
            } else {
                s_disp_lower[i] |= 0x80u;
            }
        }
    }
}

/* ========== 接收 HMI 显示缓存 → 更新内部缓冲 ========== */
static void sync_hmi_display(const DisplayFrame_t *disp)
{
    char upper_str[5];
    char lower_str[5];
    uint8_t i;

    /* --- 段码: seg_chars → 物理SMG缓冲 --- */
    /* Upper panel = Z1(seg[0,1]) + Z2(seg[2,3]) */
    upper_str[0] = disp->seg_chars[0];
    upper_str[1] = disp->seg_chars[1];
    upper_str[2] = disp->seg_chars[2];
    upper_str[3] = disp->seg_chars[3];
    upper_str[4] = '\0';

    /* Lower panel = Z3(seg[4,5]) + Z4(seg[6,7]) */
    lower_str[0] = disp->seg_chars[4];
    lower_str[1] = disp->seg_chars[5];
    lower_str[2] = disp->seg_chars[6];
    lower_str[3] = disp->seg_chars[7];
    lower_str[4] = '\0';

    /* ASCII → SMG段码 (bits 0-6; bit7留给DP) */
    HAL_SMG_FontASCII(s_disp_upper, upper_str);
    HAL_SMG_FontASCII(s_disp_lower, lower_str);

    /* 清除旧DP位后重算 */
    for (i = 0; i < 4; i++) {
        s_disp_upper[i] &= 0x7Fu;
        s_disp_lower[i] &= 0x7Fu;
    }

    /* 保存无闪烁的干净段码 (供 blink on-phase 恢复) */
    for (i = 0; i < 4; i++) {
        s_disp_upper_clean[i] = s_disp_upper[i];
        s_disp_lower_clean[i] = s_disp_lower[i];
    }

    /* 热点炉头 DP 指示器: 热点炉头的两个 DP 常亮 */
    /* 同时写入 clean data (供 blink on-phase) 和显示缓冲 (供非闪烁位) */
    if (disp->hot_head_idx >= 0 && disp->hot_head_idx < 4) {
        uint8_t hh = (uint8_t)disp->hot_head_idx;
        uint8_t p0 = hh * 2u;
        uint8_t p1 = hh * 2u + 1u;
        if (p0 < 4u) {
            s_disp_upper_clean[p0] |= 0x80u;
            s_disp_upper[p0]       |= 0x80u;
        } else {
            s_disp_lower_clean[p0 - 4u] |= 0x80u;
            s_disp_lower[p0 - 4u]       |= 0x80u;
        }
        if (p1 < 4u) {
            s_disp_upper_clean[p1] |= 0x80u;
            s_disp_upper[p1]       |= 0x80u;
        } else {
            s_disp_lower_clean[p1 - 4u] |= 0x80u;
            s_disp_lower[p1 - 4u]       |= 0x80u;
        }
    }

    /* seg_blink → 全数字闪烁 (0xFF), head_select 用物理LED */
    for (i = 0; i < 4; i++) {
        uint8_t m = disp->seg_blink[i] ? 0xFFu : 0u;
        s_blink_mask[i * 2]     = m;
        s_blink_mask[i * 2 + 1] = m;
    }

    /* 首次应用 blink */
    apply_blink_dp();

    /* --- LED: 逻辑状态 → IO[8..10] 物理位 --- */
    s_io_work[8]  = 0u;
    s_io_work[9]  = 0u;
    s_io_work[10] = 0u;

    /* 模式灯: 查表映射 */
    {
        uint8_t mode_on[LED_MODE_COUNT];
        mode_on[LED_MODE_POWER]      = disp->leds_power;
        mode_on[LED_MODE_TIMER]      = disp->leds_timer;
        mode_on[LED_MODE_PAUSE]      = disp->leds_pause;
        mode_on[LED_MODE_CHILD_LOCK] = disp->leds_child_lock;
        mode_on[LED_MODE_GROUP]      = 0;  /* 预留, 常清 */
        for (i = 0; i < LED_MODE_COUNT; i++) {
            if (mode_on[i] && s_mode_map[i].io >= 8u) {
                s_io_work[s_mode_map[i].io] |= s_mode_map[i].mask;
            }
        }
    }

    /* 档位灯: 查表映射 */
    {
        for (i = 0; i < LED_LVL_COUNT; i++) {
            if (disp->leds_power_level[i] && s_lvl_map[i].io >= 8u) {
                s_io_work[s_lvl_map[i].io] |= s_lvl_map[i].mask;
            }
        }
    }

    /* 炉头选中灯: 查表映射 */
    {
        for (i = 0; i < 4; i++) {
            if (disp->leds_head_select[i] && s_hs_map[i].io >= 8u) {
                s_io_work[s_hs_map[i].io] |= s_hs_map[i].mask;
            }
        }
    }

    /* SMG缓冲 → IO工作缓冲 (与LED同时提交, 避免撕裂) */
    s_io_work[0] = s_disp_upper[0];
    s_io_work[1] = s_disp_upper[1];
    s_io_work[2] = s_disp_upper[2];
    s_io_work[3] = s_disp_upper[3];
    s_io_work[4] = s_disp_lower[0];
    s_io_work[5] = s_disp_lower[1];
    s_io_work[6] = s_disp_lower[2];
    s_io_work[7] = s_disp_lower[3];

    s_io_dirty = 1u;
}

/* ========== __weak 接收: 由 app_hmi / app_cooking 直调 ========== */
void DrvDisplay_OnRefresh(uint16_t param, void *data_ptr)
{
    const DisplayFrame_t *disp;
    (void)param;
    if (data_ptr == NULL) return;
    disp = (const DisplayFrame_t *)data_ptr;
    sync_hmi_display(disp);
}

/* ========== 初始化 ========== */
void Drv_Display_Init(void)
{
    /* 初始化SMG库: 2组独立显示(上下各4位数码管) */
    HAL_SMG_Init();

    /* 清缓冲 */
    memset(s_disp_upper, 0x00, sizeof(s_disp_upper));
    memset(s_disp_lower, 0x00, sizeof(s_disp_lower));
    memset(s_io_buff,     0x00, sizeof(s_io_buff));
    memset(s_io_work,     0x00, sizeof(s_io_work));
    memset(s_blink_mask,  0x00, sizeof(s_blink_mask));
    s_io_dirty = 0u;

    s_scan_com         = 0u;
    s_prev_com         = 0xFFu;
    s_flash_toggle     = 0u;
    s_update_cnt       = 0u;
    s_flash_cnt        = 0u;
    s_flash_period_10ms = (uint8_t)(DISPLAY_BLINK_PHASE_MS / 10u);
    s_disp_dirty    = 0u;
    s_cur_key_code  = 0u;
    s_cur_key_state = 0u;

    /* 初始化硬件 GPIO */
    HAL_Display_Init();

    /* 启动画面: 全显测试 (写工作缓冲并提交) */
    memset(s_io_work, 0xFF, sizeof(s_io_work));
    s_io_dirty = 1u;
}

/* ========== 500ms 闪烁同步（内部）========== */
static void flash_sync(void)
{
    s_flash_toggle ^= 1u;
    HAL_SMG_FlashSync(s_flash_toggle);
    /* DRV层自行管理DP闪烁: 按新相位应用/清除bit7 */
    apply_blink_dp();
    s_io_dirty = 1u;
}

/* ========== 按键显示 ========== */
void Drv_Display_ShowKey(uint8_t key_code, uint8_t key_state)
{
    s_cur_key_code  = key_code;
    s_cur_key_state = key_state;
    s_disp_dirty    = 1u;
}

/* 根据 key_state 返回类型字符串 */
static const char *key_type_str(uint8_t state)
{
    if (state & 0x01u) return "tAP";   /* PRESS  → "tAP"   */
    if (state & 0x02u) return "Lon";   /* LONG   → "Lon"   */
    if (state & 0x04u) return "rEL";   /* RELEASE→ "rEL"   */
    if (state & 0x08u) return "HOL";   /* REPEAT → "HOL"   */
    if (state & 0x10u) return " dOn";  /* TAP    → " dOn"  */
    return " non";                       /* none   → " non" */
}

/* 刷新显示缓冲: 按键码→上, 按键类型→下 */
static void refresh_display(void)
{
    unsigned char key_hex_h, key_hex_l;
    char type_buf[5];

    /* 上4位: 按键码的16进制显示 */
    key_hex_h = (s_cur_key_code >> 4) & 0x0Fu;
    key_hex_l = s_cur_key_code & 0x0Fu;
    HAL_SMG_HexHL(s_disp_upper, key_hex_h, key_hex_l);

    /* 下4位: 按键类型字符串 */
    {
        const char *src;
        uint8_t i;
        src = key_type_str(s_cur_key_state);
        for (i = 0u; i < 4u && src[i] != '\0'; i++) {
            type_buf[i] = src[i];
        }
        for (; i < 4u; i++) {
            type_buf[i] = ' ';
        }
        type_buf[4] = '\0';
    }
    HAL_SMG_FontASCII(s_disp_lower, type_buf);

    s_disp_dirty = 0u;
}

/* ========== 内容刷新（每10ms调用，自计时100ms刷新+500ms闪烁）========== */
void Drv_Display_Update(void)
{
    /* 闪烁同步 (周期由 hmi_cfg.elements->blink_phase_ms 决定) */
    s_flash_cnt++;
    if (s_flash_cnt >= s_flash_period_10ms) {
        s_flash_cnt = 0u;
        flash_sync();
    }

    /* 100ms 内容刷新 → 写入工作缓冲 */
    s_update_cnt++;
    if (s_update_cnt >= DISPLAY_UPDATE_PERIOD_10MS) {
        s_update_cnt = 0u;

        if (s_disp_dirty != 0u) {
            refresh_display();
        }

        /* SMG缓冲 → 工作缓冲(不碰扫描缓冲) */
        s_io_work[0] = s_disp_upper[0];
        s_io_work[1] = s_disp_upper[1];
        s_io_work[2] = s_disp_upper[2];
        s_io_work[3] = s_disp_upper[3];
        s_io_work[4] = s_disp_lower[0];
        s_io_work[5] = s_disp_lower[1];
        s_io_work[6] = s_disp_lower[2];
        s_io_work[7] = s_disp_lower[3];

        s_io_dirty = 1u;
    }
}

/* ========== 硬件扫描（每1ms）========== */
void Drv_Display_Scan(void)
{
    /* 工作缓冲有新数据 → 提交到扫描缓冲 (解耦Update写入) */
    if (s_io_dirty != 0u) {
        memcpy(s_io_buff, s_io_work, sizeof(s_io_buff));
        s_io_dirty = 0u;
    }

    /* 扫描当前COM位 */
    HAL_Display_Scan(s_scan_com, s_io_buff[s_scan_com], s_prev_com);
    s_prev_com = s_scan_com;

    /* 推进COM指针: 11ms完成一轮 */
    s_scan_com++;
    if (s_scan_com >= DISP_COM_COUNT) {
        s_scan_com = 0u;
    }
}
