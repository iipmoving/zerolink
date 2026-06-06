/**
 * app_seg_align.c —— 通用数码管段位对齐模块实现
 *
 * 依赖: app_seg_align.h, <string.h>, <stddef.h>
 * 层级: APP —— 对齐/校准工具
 *
 * 串口函数通过 __weak 桩对接 HAL 层, 不直接 include hal_uart.h
 *
 * 状态机:
 *   IDLE → SEG_ALIGN(12步) → COM_ALIGN(8步) → DONE → IDLE
 *
 * 入口:
 *   - 组合键: 关机态长按 SEG_ALIGN_KEY_ENTER 3秒
 *   - 串口:   "##ALIGN" 命令 (需 SEG_ALIGN_SERIAL_ENABLE=1)
 *
 * 操作:
 *   - CONFIRM键: 正确 → 下一步
 *   - REJECT键:  不正确 → 自动调整映射
 *   - ENTER键长按3秒: 强制退出(不保存)
 */

#include "core/std_module.h"
#include "app_seg_align.h"
#include <string.h>
#include <stddef.h>

typedef struct { uint8_t dummy; } InData_t;
typedef struct { uint8_t dummy; } OutData_t;
static InData_t  s_in;
static OutData_t s_out;
MODULE_SKELETON();

/* ================================================================
 * 串口函数 __weak 桩 — 不直接 include hal_uart.h
 * ================================================================ */
#if SEG_ALIGN_SERIAL_ENABLE
__weak void HAL_UART_Debug_Print(const char *str)       { (void)str; }
__weak void HAL_UART_Debug_RX_Enable(void)              { }
__weak uint8_t HAL_UART_Debug_GetChar(uint8_t *ch)      { (void)ch; return 0u; }
#endif

/* ========== 段码常量 (逻辑段 -> 标准位布局) ========== */
#define SEG_BIT_A     0u
#define SEG_BIT_B     1u
#define SEG_BIT_C     2u
#define SEG_BIT_D     3u
#define SEG_BIT_E     4u
#define SEG_BIT_F     5u
#define SEG_BIT_G     6u
#define SEG_BIT_DP    7u

#define SEG_MASK_A    (1u << SEG_BIT_A)    /* 0x01 */
#define SEG_MASK_B    (1u << SEG_BIT_B)    /* 0x02 */
#define SEG_MASK_C    (1u << SEG_BIT_C)    /* 0x04 */
#define SEG_MASK_D    (1u << SEG_BIT_D)    /* 0x08 */
#define SEG_MASK_E    (1u << SEG_BIT_E)    /* 0x10 */
#define SEG_MASK_F    (1u << SEG_BIT_F)    /* 0x20 */
#define SEG_MASK_G    (1u << SEG_BIT_G)    /* 0x40 */
#define SEG_MASK_DP   (1u << SEG_BIT_DP)   /* 0x80 */

/* 数字段码组合 */
#define SEG_DIGIT_0  (SEG_MASK_A|SEG_MASK_B|SEG_MASK_C|SEG_MASK_D|SEG_MASK_E|SEG_MASK_F)
#define SEG_DIGIT_1  (SEG_MASK_B|SEG_MASK_C)
#define SEG_DIGIT_8  (SEG_MASK_A|SEG_MASK_B|SEG_MASK_C|SEG_MASK_D|SEG_MASK_E|SEG_MASK_F|SEG_MASK_G)

/* ========== __weak 桩: DRV/存储层可覆盖 ========== */
__weak void DrvSegAlign_WriteCom(uint8_t com, uint8_t seg_mask)
{ (void)com; (void)seg_mask; }

__weak void DrvSegAlign_BlockHmi(uint8_t block)
{ (void)block; }

__weak void SegAlign_OnSave(const SegAlignMap_t *map)
{ (void)map; }

__weak uint8_t SegAlign_OnLoad(SegAlignMap_t *map)
{ (void)map; return 0u; }

/* 进入门禁: 默认允许, HMI 模块可覆盖强符号限制仅 POWERED_OFF 进入 */
__weak uint8_t AppSegAlign_CanEnter(void) { return 1u; }

/* ========== 对齐状态机 ========== */
#define ALIGN_STATE_IDLE        0u
#define ALIGN_STATE_SEG         1u   /* SEG 段位对齐 (12步) */
#define ALIGN_STATE_COM         2u   /* COM 位选对齐 (8步)  */
#define ALIGN_STATE_DONE        3u

#define ALIGN_STEP_SEG_COUNT    12u
#define ALIGN_STEP_COM_COUNT    SEG_ALIGN_DIGIT_COUNT  /* 仅 SMG 位, 不含 LED */

/* 测试图案描述 */
typedef struct {
    uint8_t      mask;       /* 目标段码 (逻辑段), 0=特殊处理 */
    const char  *name;       /* 步骤名称 (串口输出) */
} SegTestStep_t;

static const SegTestStep_t s_seg_steps[ALIGN_STEP_SEG_COUNT] = {
    { 0xFF,         "ALL_ON" },  /* 0: 全亮         */
    { SEG_MASK_A,   "SEG_A"  },  /* 1: 仅A段        */
    { SEG_MASK_B,   "SEG_B"  },  /* 2: 仅B段        */
    { SEG_MASK_C,   "SEG_C"  },  /* 3: 仅C段        */
    { SEG_MASK_D,   "SEG_D"  },  /* 4: 仅D段        */
    { SEG_MASK_E,   "SEG_E"  },  /* 5: 仅E段        */
    { SEG_MASK_F,   "SEG_F"  },  /* 6: 仅F段        */
    { SEG_MASK_G,   "SEG_G"  },  /* 7: 仅G段        */
    { SEG_MASK_DP,  "SEG_DP" },  /* 8: 仅小数点     */
    { 0,            "DIGIT_0"},  /* 9: 显示0(特殊)  */
    { 0,            "DIGIT_1"},  /*10: 显示1(特殊)  */
    { 0,            "DIGIT_8"},  /*11: 显示8(特殊)  */
};

/* ========== 运行时状态 ========== */
static SegAlignMap_t s_map;              /* 当前映射表                 */
static uint8_t       s_align_state;      /* IDLE/SEG/COM/DONE         */
static uint8_t       s_cur_step;         /* 当前步骤 0..N-1           */
static uint8_t       s_active;           /* 对齐模式激活标志          */
static uint8_t       s_enter_hold;       /* 进入键长按计数            */
static uint8_t       s_exit_hold;        /* 退出键长按计数            */
static uint8_t       s_seg_tried[8];     /* 每段已尝试的物理位掩码    */
static uint8_t       s_com_tried[11];    /* 每COM已尝试的物理位掩码   */

#if SEG_ALIGN_SERIAL_ENABLE
/* ========== 串口命令缓冲 ========== */
#define SERIAL_BUF_SIZE  32u
static char    s_serial_buf[SERIAL_BUF_SIZE];
static uint8_t s_serial_pos;
#endif

/* ========== 内部: 计算 CRC8 (XOR) ========== */
static uint8_t calc_crc8(const uint8_t *data, uint8_t len)
{
    uint8_t i, crc = 0u;
    for (i = 0u; i < len; i++) { crc ^= data[i]; }
    return crc;
}

/* ========== 内部: 更新映射 CRC ========== */
static void update_map_crc(void)
{
    s_map.crc8 = calc_crc8((const uint8_t *)&s_map, sizeof(SegAlignMap_t) - 1u);
}

/* ========== 内部: 将逻辑段码转为物理段码 ========== */
static uint8_t logical_to_physical(uint8_t logical_mask)
{
    uint8_t i, physical = 0u;
    for (i = 0u; i < 8u; i++) {
        if (logical_mask & (1u << i)) {
            physical |= (uint8_t)(1u << s_map.seg_order[i]);
        }
    }
    return physical;
}

/* ========== 内部: 获取当前步骤的目标段码(逻辑) ========== */
static uint8_t get_step_seg_mask(void)
{
    switch (s_cur_step) {
    case 9u:  return SEG_DIGIT_0;
    case 10u: return SEG_DIGIT_1;
    case 11u: return SEG_DIGIT_8;
    default:  return s_seg_steps[s_cur_step].mask;
    }
}

/* ========== 内部: 调整当前段的物理位映射 ========== */
static void adjust_current_seg(void)
{
    uint8_t seg_idx, tried, bit;

    if (s_cur_step >= 1u && s_cur_step <= 8u) {
        /* 单段测试: 尝试当前逻辑段的不同物理位 */
        seg_idx = s_cur_step - 1u;
        tried   = s_seg_tried[seg_idx];

        for (bit = 0u; bit < 8u; bit++) {
            if (!(tried & (1u << bit))) {
                s_map.seg_order[seg_idx] = bit;
                s_seg_tried[seg_idx] |= (uint8_t)(1u << bit);
                update_map_crc();
                return;
            }
        }
        /* 全部试完: 重置为默认, 清尝试记录 */
        s_map.seg_order[seg_idx] = seg_idx;
        s_seg_tried[seg_idx] = 0u;
    } else {
        /* 全亮/数字测试: 尝试反转 + 偏移 */
        s_map.seg_order[0] = (s_map.seg_order[0] + 1u) & 0x07u;
        if (s_map.seg_order[0] == 0u) {
            s_map.common_type ^= 1u;  /* 切换共阴/共阳 */
        }
    }
    update_map_crc();
}

/* ========== 内部: 调整当前 COM 映射 ========== */
static void adjust_current_com(void)
{
    uint8_t tried, bit;

    tried = s_com_tried[s_cur_step];
    for (bit = 0u; bit < SEG_ALIGN_DIGIT_COUNT; bit++) {
        if (!(tried & (1u << bit))) {
            s_map.com_order[s_cur_step] = bit;
            s_com_tried[s_cur_step] |= (uint8_t)(1u << bit);
            update_map_crc();
            return;
        }
    }
    s_map.com_order[s_cur_step] = s_cur_step;
    s_com_tried[s_cur_step] = 0u;
    update_map_crc();
}

/* ========== 内部: 刷新显示(逐COM写段码到DRV) ========== */
static void refresh_alignment_display(void)
{
    uint8_t i, logical_mask, physical_mask;

    /* 清零所有 COM */
    for (i = 0u; i < SEG_ALIGN_COM_COUNT; i++) {
        DrvSegAlign_WriteCom(i, 0x00);
    }

    if (s_align_state == ALIGN_STATE_SEG) {
        logical_mask  = get_step_seg_mask();
        physical_mask = logical_to_physical(logical_mask);
        if (s_map.common_type != 0u) {
            physical_mask = (uint8_t)(~physical_mask);
        }
        for (i = 0u; i < SEG_ALIGN_DIGIT_COUNT; i++) {
            DrvSegAlign_WriteCom(s_map.com_order[i], physical_mask);
        }
    } else if (s_align_state == ALIGN_STATE_COM) {
        physical_mask = (s_map.common_type == 0u) ? 0xFFu : 0x00u;
        DrvSegAlign_WriteCom(s_map.com_order[s_cur_step], physical_mask);
    }
}

#if SEG_ALIGN_SERIAL_ENABLE
/* ========== 内部: 串口输出单字符 ========== */
static void serial_putc(char c)
{
    char s[2]; s[0] = c; s[1] = '\0';
    HAL_UART_Debug_Print(s);
}

/* ========== 内部: 串口输出数字 ========== */
static void serial_putd(uint8_t n)
{
    if (n >= 10u) serial_putc((char)('0' + n / 10u));
    serial_putc((char)('0' + n % 10u));
}

/* ========== 内部: 串口输出对齐状态 ========== */
static void serial_report_status(void)
{
    if (s_align_state == ALIGN_STATE_IDLE) {
        HAL_UART_Debug_Print("[ALIGN] Idle\r\n");
        return;
    }
    if (s_align_state == ALIGN_STATE_DONE) {
        HAL_UART_Debug_Print("[ALIGN] Done, mapping saved\r\n");
        return;
    }
    HAL_UART_Debug_Print("[ALIGN] Phase=");
    HAL_UART_Debug_Print((s_align_state == ALIGN_STATE_SEG) ? "SEG" : "COM");
    HAL_UART_Debug_Print(" Step=");
    serial_putd(s_cur_step + 1u);
    HAL_UART_Debug_Print(" Name=");
    HAL_UART_Debug_Print((s_align_state == ALIGN_STATE_SEG)
        ? s_seg_steps[s_cur_step].name : "COM");
    HAL_UART_Debug_Print("\r\n");
}

static void serial_report_mapping(void)
{
    uint8_t i;
    HAL_UART_Debug_Print("[ALIGN] seg_order: ");
    for (i = 0u; i < 8u; i++) {
        serial_putd(s_map.seg_order[i]);
        serial_putc(' ');
    }
    HAL_UART_Debug_Print(s_map.common_type ? "CA\r\n" : "CC\r\n");
}

/* ========== 内部: 串口命令解析 ========== */
static void serial_parse_cmd(const char *cmd)
{
    if (strncmp(cmd, "##ALIGN", 7u) != 0) return;

    if (cmd[7u] == '\0' || cmd[7u] == '\r') {
        if (!s_active) {
            s_align_state  = ALIGN_STATE_SEG;
            s_cur_step     = 0u;
            s_active       = 1u;
            s_enter_hold   = 0u;
            s_exit_hold    = 0u;
            DrvSegAlign_BlockHmi(1u);
            memset(s_seg_tried, 0x00, sizeof(s_seg_tried));
            memset(s_com_tried, 0x00, sizeof(s_com_tried));
            refresh_alignment_display();
            HAL_UART_Debug_Print("[ALIGN] Entered SEG phase\r\n");
        }
    } else if (cmd[7u] == ':') {
        if (strncmp(&cmd[8u], "OK", 2u) == 0) {
            AppSegAlign_OnKey(
                (uint16_t)SEG_ALIGN_KEY_CONFIRM
                | ((uint16_t)SEG_ALIGN_KEY_TAP << 8), NULL);
        } else if (strncmp(&cmd[8u], "NG", 2u) == 0) {
            AppSegAlign_OnKey(
                (uint16_t)SEG_ALIGN_KEY_REJECT
                | ((uint16_t)SEG_ALIGN_KEY_TAP << 8), NULL);
        } else if (cmd[8u] == '?') {
            serial_report_status();
            serial_report_mapping();
        } else if (strncmp(&cmd[8u], "EXIT", 4u) == 0) {
            s_active       = 0u;
            s_align_state  = ALIGN_STATE_IDLE;
            s_cur_step     = 0u;
            DrvSegAlign_BlockHmi(0u);
            HAL_UART_Debug_Print("[ALIGN] Exited\r\n");
        }
    }
}
#endif /* SEG_ALIGN_SERIAL_ENABLE */

/* ========== 内部: 推进到下一步 ========== */
static void advance_step(void)
{
    if (s_align_state == ALIGN_STATE_SEG) {
        s_cur_step++;
        if (s_cur_step >= ALIGN_STEP_SEG_COUNT) {
            s_align_state = ALIGN_STATE_COM;
            s_cur_step    = 0u;
            memset(s_com_tried, 0x00, sizeof(s_com_tried));
#if SEG_ALIGN_SERIAL_ENABLE
            HAL_UART_Debug_Print("[ALIGN] SEG done -> COM phase\r\n");
#endif
        }
        refresh_alignment_display();
        return;
    }

    if (s_align_state == ALIGN_STATE_COM) {
        s_cur_step++;
        if (s_cur_step >= ALIGN_STEP_COM_COUNT) {
            s_align_state = ALIGN_STATE_DONE;
            s_active      = 0u;
            DrvSegAlign_BlockHmi(0u);
            update_map_crc();
            SegAlign_OnSave(&s_map);
#if SEG_ALIGN_SERIAL_ENABLE
            HAL_UART_Debug_Print("[ALIGN] All done, mapping saved\r\n");
            serial_report_mapping();
#endif
            return;
        }
        refresh_alignment_display();
        return;
    }
}

/* ========== 公开: AppSegAlign_IsActive ========== */
uint8_t AppSegAlign_IsActive(void)
{
    return s_active;
}

static void ProcessInput(void) {}

/* ========== Init ========== */
static void Init(void)
{
    uint8_t i;

    for (i = 0u; i < 8u; i++)  { s_map.seg_order[i] = i; }
    for (i = 0u; i < 11u; i++) { s_map.com_order[i] = i; }
    s_map.common_type = 0u;
    s_map.crc8        = 0u;

    s_align_state = ALIGN_STATE_SEG;  /* TODO: 改为 IDLE, 按键进入 */
    s_cur_step    = 0u;
    s_active      = 1u;              /* TODO: 改为 0u, 按键激活 */

    DrvSegAlign_BlockHmi(1u);        /* 阻塞 HMI 刷新, 防止覆盖对齐画面 */

    /* 调试: 确认初始化被执行 */
    HAL_UART_Debug_Print("[ALIGN] Init: active=");
    {
        char _a[2]; _a[0] = (char)('0' + s_active); _a[1] = '\0';
        HAL_UART_Debug_Print(_a);
    }
    HAL_UART_Debug_Print(" state=");
    {
        char _b[2]; _b[0] = (char)('0' + s_align_state); _b[1] = '\0';
        HAL_UART_Debug_Print(_b);
    }
    HAL_UART_Debug_Print("\r\n");
    s_enter_hold  = 0u;
    s_exit_hold   = 0u;
    memset(s_seg_tried, 0x00, sizeof(s_seg_tried));
    memset(s_com_tried, 0x00, sizeof(s_com_tried));

#if SEG_ALIGN_SERIAL_ENABLE
    s_serial_pos = 0u;
    memset(s_serial_buf, 0x00, sizeof(s_serial_buf));
    HAL_UART_Debug_RX_Enable();
#endif

    (void)SegAlign_OnLoad(&s_map);

    /* 强制进入: 立即显示第一个测试图案 */
    refresh_alignment_display();

    g_input.para  = &s_in;
    g_output.para = &s_out;
}

void AppSegAlign_Init(void) { Constructor(); }
void AppSegAlign_Run(void)
{
#if SEG_ALIGN_SERIAL_ENABLE
    uint8_t ch;

    while (HAL_UART_Debug_GetChar(&ch)) {
        if (ch == '\r' || ch == '\n') {
            if (s_serial_pos > 0u) {
                s_serial_buf[s_serial_pos] = '\0';
                serial_parse_cmd(s_serial_buf);
                s_serial_pos = 0u;
            }
        } else if (s_serial_pos < SERIAL_BUF_SIZE - 1u) {
            s_serial_buf[s_serial_pos++] = (char)ch;
        }
    }
#else
    (void)0;
#endif
}

/* ========== 公开: AppSegAlign_OnKey ========== */
void AppSegAlign_OnKey(uint16_t param, void *data_ptr)
{
    uint8_t key_code, key_state;
    (void)data_ptr;

    key_code  = (uint8_t)(param & 0xFFu);
    key_state = (uint8_t)((param >> 8) & 0xFFu);

    /* --- 进入键长按检测 (在所有状态下) --- */
    if (key_code == (uint8_t)SEG_ALIGN_KEY_ENTER) {
        if (key_state == SEG_ALIGN_KEY_LONG
            || key_state == SEG_ALIGN_KEY_REPEAT) {
            s_enter_hold++;
        } else if (key_state == SEG_ALIGN_KEY_RELEASE
                   || key_state == SEG_ALIGN_KEY_TAP) {
            s_enter_hold = 0u;
        }

        if (!s_active && s_enter_hold >= SEG_ALIGN_HOLD_REPEAT) {
            /* TODO: 门禁条件待定, 当前强制进入测试 */
            s_align_state  = ALIGN_STATE_SEG;
            s_cur_step     = 0u;
            s_active       = 1u;
            s_enter_hold   = 0u;
            s_exit_hold    = 0u;
            DrvSegAlign_BlockHmi(1u);
            memset(s_seg_tried, 0x00, sizeof(s_seg_tried));
            memset(s_com_tried, 0x00, sizeof(s_com_tried));
            refresh_alignment_display();
#if SEG_ALIGN_SERIAL_ENABLE
            HAL_UART_Debug_Print("[ALIGN] Key entry -> SEG phase\r\n");
#endif
            return;
        }
        return;
    }

    /* 退出键: 长按强制退出 */
    if (s_active && key_code == (uint8_t)SEG_ALIGN_KEY_EXIT) {
        if (key_state == SEG_ALIGN_KEY_LONG
            || key_state == SEG_ALIGN_KEY_REPEAT) {
            s_exit_hold++;
            if (s_exit_hold >= SEG_ALIGN_HOLD_REPEAT) {
                s_active       = 0u;
                s_align_state  = ALIGN_STATE_IDLE;
                s_cur_step     = 0u;
                s_exit_hold    = 0u;
                DrvSegAlign_BlockHmi(0u);
#if SEG_ALIGN_SERIAL_ENABLE
                HAL_UART_Debug_Print("[ALIGN] Key exit\r\n");
#endif
            }
        } else {
            s_exit_hold = 0u;
        }
        return;
    }

    /* 非对齐模式: 忽略所有按键 */
    if (!s_active) return;

    /* 仅响应 TAP (短按确认/报错) */
    if (key_state != SEG_ALIGN_KEY_TAP) return;

    if (key_code == (uint8_t)SEG_ALIGN_KEY_CONFIRM) {
#if SEG_ALIGN_SERIAL_ENABLE
        HAL_UART_Debug_Print("[ALIGN] Step OK\r\n");
#endif
        advance_step();
    } else if (key_code == (uint8_t)SEG_ALIGN_KEY_REJECT) {
#if SEG_ALIGN_SERIAL_ENABLE
        HAL_UART_Debug_Print("[ALIGN] Step NG, adjusting\r\n");
#endif
        if (s_align_state == ALIGN_STATE_SEG) {
            adjust_current_seg();
        } else if (s_align_state == ALIGN_STATE_COM) {
            adjust_current_com();
        }
        refresh_alignment_display();
#if SEG_ALIGN_SERIAL_ENABLE
        serial_report_mapping();
#endif
    }
}

MODULE_EXPORT(AppSegAlign);
