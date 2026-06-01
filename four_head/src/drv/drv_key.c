/**
 * drv_key.c —— 按键驱动层实现
 *
 * 依赖: msg_scheduler.h + hal_key.h + drv_key.h
 * 层级: DRV —— 不include其他业务模块
 *
 * 去抖: 连续2次读到相同键值确认为按下
 * 组合键: 相邻功率焊盘同时触摸(如TK12+TK13→KEY_POWER_1)
 * 键码映射: 提取自参考程序 Key_Driver.c
 */
#include "drv_key.h"
#include "../hal/hal_key.h"
#include <stddef.h>

/* __weak 回调: 多接收方广播, 链接器自动接线, interface_map.h 文档化 */
__weak void AppHmi_OnKey(uint16_t param, void *data_ptr)
{ (void)param; (void)data_ptr; }
__weak void AppCooking_OnKey(uint16_t param, void *data_ptr)
{ (void)param; (void)data_ptr; }
__weak void AppSegAlign_OnKey(uint16_t param, void *data_ptr)
{ (void)param; (void)data_ptr; }

/* ========== MCU触摸通道位掩码（与TKDriver.h MCU_TK定义一致）========== */
#define TK_CH(n)        (1UL << (n))  /* 通道n的位掩码                         */

/* PCB丝印 DF_TK1~DF_TK21 → MCU通道映射 */
#define DF_TK1          TK_CH(28)
#define DF_TK2          TK_CH(29)
#define DF_TK3          TK_CH(24)
#define DF_TK4          TK_CH(30)
#define DF_TK5          TK_CH(23)
#define DF_TK6          TK_CH(22)
#define DF_TK7          TK_CH(10)
#define DF_TK8          TK_CH(11)
#define DF_TK9          TK_CH(13)
#define DF_TK10         TK_CH(7)
#define DF_TK11         TK_CH(8)
#define DF_TK12         TK_CH(21)
#define DF_TK13         TK_CH(20)
#define DF_TK14         TK_CH(19)
#define DF_TK15         TK_CH(18)
#define DF_TK16         TK_CH(17)
#define DF_TK17         TK_CH(16)
#define DF_TK18         TK_CH(15)
#define DF_TK19         TK_CH(14)
#define DF_TK20         TK_CH(12)
#define DF_TK21         TK_CH(9)

/* ========== 物理→逻辑映射表 ========== */
/* 组合键必须排在单键前面，确保优先匹配 */
typedef struct {
    uint32_t  phy_mask;     /* 物理通道位掩码                               */
    uint8_t   key_code;     /* 逻辑键码 KeyCode_t                           */
} KeyMap_t;

static const KeyMap_t s_key_map[] = {
    /* 组合键（相邻功率键同时按下 → 中间的档位值） */
    { DF_TK12 + DF_TK13,    KEY_POWER_1 },
    { DF_TK13 + DF_TK14,    KEY_POWER_2 },
    { DF_TK14 + DF_TK15,    KEY_POWER_3 },
    { DF_TK15 + DF_TK16,    KEY_POWER_4 },
    { DF_TK16 + DF_TK17,    KEY_POWER_5 },
    { DF_TK17 + DF_TK18,    KEY_POWER_6 },
    { DF_TK18 + DF_TK19,    KEY_POWER_7 },
    { DF_TK19 + DF_TK20,    KEY_POWER_8 },
    { DF_TK20 + DF_TK21,    KEY_POWER_9 },

    /* 单键映射 */
    { DF_TK11,              KEY_ONOFF },
    { DF_TK5,               KEY_RIGHT_P_SET },
    { DF_TK3,               KEY_LEFT_P_SET },
    { DF_TK4,               KEY_RIGHT_P_SET_UP },
    { DF_TK2,               KEY_LEFT_P_SET_UP },
    { DF_TK1,               KEY_SUB },
    { DF_TK6,               KEY_ADD },
    { DF_TK7,               KEY_TIME_SET },
    { DF_TK8,               KEY_STOP },
    { DF_TK9,               KEY_BIND },
    { DF_TK10,              KEY_LOCK },
    { DF_TK12,              KEY_POWER_0 },
    { DF_TK13,              KEY_POWER_1 },
    { DF_TK14,              KEY_POWER_2 },
    { DF_TK15,              KEY_POWER_3 },
    { DF_TK16,              KEY_POWER_4 },
    { DF_TK17,              KEY_POWER_5 },
    { DF_TK18,              KEY_POWER_6 },
    { DF_TK19,              KEY_POWER_7 },
    { DF_TK20,              KEY_POWER_8 },
    { DF_TK21,              KEY_POWER_9 },
};
#define KEY_MAP_COUNT (sizeof(s_key_map) / sizeof(s_key_map[0]))

/* ========== 去抖状态 ========== */
static uint32_t s_debounce_mask;    /* 上一次读到的物理键值                     */
static uint8_t  s_debounce_cnt;     /* 连续相同次数                             */
static uint8_t  s_last_key;         /* 当前按下的逻辑键码(KEY_NONE=空闲)        */
static uint16_t s_hold_cnt;         /* 按住期间的扫描次数(每次10ms)             */
static uint8_t  s_long_sent;        /* 已发送LONG标志                           */
static uint8_t  s_release_cnt;      /* 松手去抖计数 (连续读到0的次数)            */

/* ========== 映射查找 ========== */
static uint8_t Key_Lookup(uint32_t phy_mask)
{
    uint8_t i;
    for (i = 0u; i < (uint8_t)KEY_MAP_COUNT; i++) {
        if (s_key_map[i].phy_mask == phy_mask) {
            return s_key_map[i].key_code;
        }
    }
    return (uint8_t)KEY_NONE;
}

/* ========== 初始化 ========== */
void Drv_Key_Init(void)
{
    HAL_Key_Init();
    s_debounce_mask = 0u;
    s_debounce_cnt  = 0u;
    s_last_key      = (uint8_t)KEY_NONE;
    s_hold_cnt      = 0u;
    s_long_sent     = 0u;
    s_release_cnt   = 0u;
}

/* ========== 按键事件发送 ========== */
static void Key_PostEvent(uint8_t key_code, uint8_t key_state)
{
    uint16_t param = (uint16_t)key_code | ((uint16_t)key_state << 8);
    AppHmi_OnKey(param, NULL);
    AppCooking_OnKey(param, NULL);
    AppSegAlign_OnKey(param, NULL);
}

/* ========== 处理按键释放（切键时先释放旧键） ========== */
static void Key_Release(void)
{
    if (s_last_key != (uint8_t)KEY_NONE) {
        if (!s_long_sent) {
            Key_PostEvent(s_last_key, KEY_STATE_TAP);
        }
        Key_PostEvent(s_last_key, KEY_STATE_RELEASE);
        s_last_key  = (uint8_t)KEY_NONE;
    }
    s_hold_cnt  = 0u;
    s_long_sent = 0u;
}

/* ========== 扫描入口（每10ms调用一次）========== */
void Drv_Key_Scan(void)
{
    uint32_t raw_mask;
    uint8_t  key;
    raw_mask = HAL_Key_Scan();
    if (raw_mask == KEY_SCAN_NOT_READY) return;

    if (raw_mask != 0u) {
        /* 有触摸 → 清零松手去抖 */
        s_release_cnt = 0u;
        if (raw_mask == s_debounce_mask) {
            s_debounce_cnt++;
            if (s_debounce_cnt >= KEY_DEBOUNCE_CNT) {
                /* 去抖通过 */
                key = Key_Lookup(raw_mask);
                if (key == (uint8_t)KEY_NONE) {
                    /* 未映射的物理组合 → 释放 */
                    Key_Release();
                    return;
                }

                if (key != s_last_key) {
                    /* 新键按下（或切键）→ 先释放旧键 */
                    Key_Release();
                    s_last_key  = key;
                    s_hold_cnt  = 0u;
                    s_long_sent = 0u;
                    Key_PostEvent(key, KEY_STATE_PRESS);
                } else {
                    /* 同一键持续按住 → 计数 */
                    s_hold_cnt++;
                    if (!s_long_sent && s_hold_cnt >= KEY_LONG_THRESH) {
                        s_long_sent = 1u;
                        Key_PostEvent(key, KEY_STATE_LONG);
                    } else if (s_long_sent) {
                        if (((s_hold_cnt - KEY_LONG_THRESH) % KEY_REPEAT_PERIOD) == 0u) {
                            Key_PostEvent(key, KEY_STATE_REPEAT);
                        }
                    }
                }
            }
        } else {
            /* 触摸模式变化 → 释放旧键，开始新去抖 */
            Key_Release();
            s_debounce_mask = raw_mask;
            s_debounce_cnt  = 1u;
        }
    } else {
        /* 无触摸 → 松手去抖 (连续N次读0才确认松手, 避免噪声误释放) */
        s_release_cnt++;
        if (s_release_cnt >= KEY_RELEASE_DB_CNT) {
            Key_Release();
            s_debounce_mask = 0u;
            s_debounce_cnt  = 0u;
            s_release_cnt   = 0u;
        }
    }
}
