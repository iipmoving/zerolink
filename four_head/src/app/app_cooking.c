/**
 * @file    app_cooking.c
 * @brief   4炉头烹饪状态机
 * @layer   APP
 *
 * 输入: Key(按键→事件驱动), RegData(温度→遥测), Timer1s(秒脉冲→计时)
 * 输出: PowerCtrl(→app_power), DisplayCmd(→drv_display)
 */
#include "core/std_module.h"
#include "app_cooking.h"
#include <string.h>

/* =================================================================
 * 数据结构
 * ================================================================= */

typedef struct {
    uint8_t  key_code;
    uint8_t  key_state;
    uint8_t  key_valid;
    uint8_t  reg_head;
    uint8_t  reg_temp;
    uint8_t  reg_valid;
    uint8_t  timer_valid;
} InData_t;

typedef struct {
    uint8_t  has_power;
    uint8_t  has_display;
    uint8_t  power_head;
    uint8_t  power_onoff;
    uint16_t power_watt;
    uint8_t  power_level;
    uint8_t  power_mode;
    uint16_t power_temp;
    uint8_t  disp_head;
    uint8_t  disp_cmd;
} OutData_t;

/* 炉头上下文 */
typedef struct {
    uint8_t  state;       /* CookState_t */
    uint8_t  menu_id;
    uint8_t  step_index;
    uint16_t step_timer;
    uint8_t  current_temp;
    uint8_t  user_power_lv;
    uint8_t  boil_samples;
    uint16_t boil_temp_acc;
} CookCtx_t;

/* =================================================================
 * 静态存储
 * ================================================================= */

static InData_t  s_in;
static OutData_t s_out;
static CookCtx_t        s_ctx[COOK_HEAD_COUNT];
static uint8_t          s_selected_head;
static uint16_t         s_power_table[] = {
    0,200,300,400,500,1000,1100,1200,1300,1500,2000};

/* 菜单定义 */
static const CookStep_t s_steps_keepwarm[] = {{3,3,0,65,0}};
static const CookStep_t s_steps_hotpot[]   = {{1,5,0,0,0}};
static const CookStep_t s_steps_boil[]     = {{2,10,0,0,0},{1,2,1800,0,0}};
static const CookMenu_t s_menus[COOK_MENU_MAX] = {
    {0,NULL}, {1,s_steps_keepwarm}, {1,s_steps_hotpot}, {2,s_steps_boil}};

/* =================================================================
 * 骨架
 * ================================================================= */

MODULE_SKELETON();

/* =================================================================
 * 内部函数
 * ================================================================= */

static uint8_t detect_boil(CookCtx_t *ctx)
{
    if (ctx->current_temp < 90) { ctx->boil_samples = 0; ctx->boil_temp_acc = 0; return 0; }
    ctx->boil_samples++;
    ctx->boil_temp_acc += ctx->current_temp;
    if (ctx->boil_samples >= 3) {
        uint8_t avg = (uint8_t)(ctx->boil_temp_acc / ctx->boil_samples);
        int16_t d = (int16_t)ctx->current_temp - (int16_t)avg;
        if (d < 0) d = -d;
        if (d < 2) { ctx->boil_samples = 0; ctx->boil_temp_acc = 0; return 1; }
    }
    return 0;
}

static uint16_t temp_hysteresis(CookCtx_t *ctx, uint8_t target, uint8_t lv)
{
    uint8_t t = ctx->current_temp;
    if (t < target - 3)  return s_power_table[lv];
    if (t > target)       return 0;
    return (lv > 2) ? s_power_table[lv-2] : s_power_table[1];
}

static void out_power(uint8_t h, uint8_t on, uint16_t w, uint8_t lv, uint8_t md, uint16_t tmp)
{
    OutData_t *o = (OutData_t *)g_output.para;
    o->has_power = 1;
    o->power_head  = h;  o->power_onoff = on;  o->power_watt = w;
    o->power_level = lv; o->power_mode  = md;  o->power_temp = tmp;
    g_output.info.route = g_input.info.route;
    g_output.info.status |= ST_OUT;
}

static void out_disp(uint8_t h, uint8_t cmd)
{
    OutData_t *o = (OutData_t *)g_output.para;
    o->has_display = 1;
    o->disp_head = h;  o->disp_cmd = cmd;
    g_output.info.route = g_input.info.route;
    g_output.info.status |= ST_OUT;
}

/* 烹饪控制命令处理 */
static void on_cmd(uint8_t head, uint8_t cmd, uint8_t menu)
{
    if (head >= COOK_HEAD_COUNT) return;
    CookCtx_t *ctx = &s_ctx[head];
    switch (cmd) {
    case 1: /* START */
        if (menu == 0 || menu >= COOK_MENU_MAX || s_menus[menu].step_count == 0) return;
        ctx->menu_id = menu; ctx->step_index = 0;
        ctx->step_timer = s_menus[menu].steps[0].duration_sec;
        ctx->boil_samples = 0; ctx->boil_temp_acc = 0;
        ctx->state = 1; /* RUN */
        out_disp(head, 1); break;
    case 2: /* STOP */
        ctx->state = 0; ctx->menu_id = 0; /* OFF */
        out_power(head, 0,0,0,0,0); out_disp(head, 0); break;
    case 3: /* PAUSE */
        if (ctx->state == 1) { ctx->state = 2; out_power(head, 0,0,0,0,0); } break;
    case 4: /* RESUME */
        if (ctx->state == 2) ctx->state = 1; break;
    case 5: /* SET_POWER */
        if (menu <= 10) ctx->user_power_lv = menu; break;
    }
}

/* 1秒定时烹饪推进 */
static void cook_tick(void)
{
    for (uint8_t i = 0; i < COOK_HEAD_COUNT; i++) {
        CookCtx_t *ctx = &s_ctx[i];
        if (ctx->state != 1 || ctx->menu_id == 0 || ctx->menu_id >= COOK_MENU_MAX) continue;

        const CookMenu_t *menu = &s_menus[ctx->menu_id];
        if (ctx->step_index >= menu->step_count) continue;

        const CookStep_t *step = &menu->steps[ctx->step_index];
        if (ctx->step_timer > 0) ctx->step_timer--;

        uint8_t next = 0;
        switch (step->type) {
        case 1: { /* POWER */
            uint8_t lv = ctx->user_power_lv ? ctx->user_power_lv : step->power_level;
            out_power(i, 1, s_power_table[lv], lv, 0, 0);
            if (ctx->step_timer == 0 && step->duration_sec > 0) next = 1;
            break;
        }
        case 2: { /* BOIL */
            out_power(i, 1, s_power_table[step->power_level], step->power_level, 0, 0);
            if (detect_boil(ctx)) next = 1;
            break;
        }
        case 3: { /* TEMP_HOLD */
            uint8_t lv = ctx->user_power_lv ? ctx->user_power_lv : step->power_level;
            uint16_t p = temp_hysteresis(ctx, step->target_temp, lv);
            out_power(i, p ? 1 : 0, p, lv, 1, step->target_temp);
            if (step->duration_sec > 0 && ctx->step_timer == 0) next = 1;
            break;
        }
        case 4: { /* CYCLE */
            uint16_t p = ((ctx->step_timer % 6) < 3) ? s_power_table[step->power_level] : 0;
            out_power(i, p ? 1 : 0, p, step->power_level, 0, 0);
            if (ctx->step_timer == 0 && step->duration_sec > 0) next = 1;
            break;
        }
        default: next = 1; break;
        }

        if (next) {
            ctx->step_index++;
            if (ctx->step_index >= menu->step_count) {
                ctx->state = 3; /* COMPLETE */
                out_power(i, 0,0,0,0,0); out_disp(i, 2);
            } else {
                ctx->step_timer = menu->steps[ctx->step_index].duration_sec;
                ctx->boil_samples = 0; ctx->boil_temp_acc = 0;
            }
        }
    }
}

/* =================================================================
 * ProcessInput — 每帧调用
 * ================================================================= */

static void ProcessInput(void)
{
    InData_t *in = (InData_t *)g_input.para;

    /* ====== 输入段 ====== */
    if (g_input.info.status & ST_NEW) {
        if (in->key_valid) {
            /* 按键直接触发命令（旧 AppCooking_OnKey 的逻辑）*/
            uint8_t kc = in->key_code, ks = in->key_state;
            if (kc == 1 && ks == 1) s_selected_head = (s_selected_head + 1) % COOK_HEAD_COUNT;
            if (kc == 2 && ks == 1) on_cmd(s_selected_head, 2, 0);
            if (kc == 4 && ks == 1) on_cmd(s_selected_head, 1, 1);
            if (kc >= 0x10 && kc <= 0x19 && ks == 1) on_cmd(s_selected_head, 5, kc - 0x10);
            in->key_valid = 0;
        }
        if (in->reg_valid) {
            uint8_t idx = in->reg_head;
            if (idx < COOK_HEAD_COUNT) s_ctx[idx].current_temp = in->reg_temp;
            in->reg_valid = 0;
        }
        if (in->timer_valid) {
            cook_tick();
            in->timer_valid = 0;
        }
        g_input.info.status &= ~ST_NEW;
    }
}

/* =================================================================
 * Init
 * ================================================================= */

static void Init(void)
{
    memset(s_ctx, 0, sizeof(s_ctx));
    for (uint8_t i = 0; i < COOK_HEAD_COUNT; i++) s_ctx[i].current_temp = 25;
    s_selected_head = 0;
    memset(&s_in, 0, sizeof(s_in));
    memset(&s_out, 0, sizeof(s_out));
    g_input.para  = &s_in;
    g_output.para = &s_out;
}

/* =================================================================
 * 导出
 * ================================================================= */

MODULE_EXPORT(AppCooking);

/* =================================================================
 * 强符号桥接: 旧调用方 → 写入 g_input
 * ================================================================= */

void AppCooking_OnKey(uint16_t param, void *data_ptr)
{
    (void)data_ptr;
    InData_t *in = (InData_t *)g_input.para;
    in->key_code  = (uint8_t)(param & 0xFF);
    in->key_state = (uint8_t)((param >> 8) & 0xFF);
    in->key_valid = 1;
    g_input.info.status |= ST_NEW;
}

void AppCooking_OnRegData(uint16_t param, void *data_ptr)
{
    if (!data_ptr) return;
    InData_t *in = (InData_t *)g_input.para;
    in->reg_head = (uint8_t)(param & 0xFF);
    in->reg_temp = ((uint8_t *)data_ptr)[10];  /* regs[3] low byte */
    in->reg_valid = 1;
    g_input.info.status |= ST_NEW;
}

void AppCooking_OnTimer1s(uint16_t param, void *data_ptr)
{
    (void)param; (void)data_ptr;
    InData_t *in = (InData_t *)g_input.para;
    in->timer_valid = 1;
    g_input.info.status |= ST_NEW;
}
