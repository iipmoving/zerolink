// ===== [AI GENERATED] 范式接入+骨架, 可被PY替换 =====
#include "../include_io/app_cooking_io.h"

static void Init(void);
MODULE_SKELETON(AppCooking);

/* 管道就绪标志: 每 BIT 代表一个管道的 ST_NEW 状态 */
typedef union {
    uint8_t all;
    struct {
        uint8_t drvkey   : 1;  /* DrvKey 数据就绪 */
        uint8_t appcommmgr   : 1;  /* AppCommMgr 数据就绪 */
    } bits;
} AppCooking_PipeFlags_t;

/* ---- 数据实体（模块私有）---- */
static MODULE_INPUT(AppCooking)*   s_inPara;    // 输入参数实体在ADC， 这里只调用不修改
static MODULE_OUTPUT(AppCooking)  s_outPara;   // 输出参数缓冲区

/* ---- 消费者 last_seq — seq 有效性比对 (空闲SLOT幂等) ---- */
static uint8_t s_last_seq_DrvKey = 0xFF;  /* DrvKey→AppCooking */
static uint8_t s_last_seq_AppCommMgr = 0xFF;  /* AppCommMgr→AppCooking */

/* ---- 内部 OUTPUT_LINK + PARAMS 实例 ---- */
static MODULE_OUTPUT_PARAMS(AppCooking, AppPower)  s_AppCookingToAppPowerParams;
static MODULE_OUTPUT_LINK(AppCooking, AppPower)  s_AppCookingToAppPowerLink;


/* 用户业务入口: flags.bits 指示哪些管道有新数据 (seq 比对通过)
 * 实际定义在用户代码区 (可引用用户变量/函数) */
static void user_Process(MODULE_INPUT(AppCooking) *in, MODULE_OUTPUT(AppCooking) *out, AppCooking_PipeFlags_t flags);

static void ProcessInput(void)
{
    MODULE_INPUT(AppCooking) *in  = (MODULE_INPUT(AppCooking)*)g_input.para;
    MODULE_OUTPUT(AppCooking) *out = (MODULE_OUTPUT(AppCooking)*)g_output.para;

    /* === 输入段: seq 有效性比对 === */
    AppCooking_PipeFlags_t flags = {0};
    {
        uint8_t cur_seq = in->DrvKey_params->seq;
        if (cur_seq != s_last_seq_DrvKey) {
            flags.bits.drvkey = 1;
            s_last_seq_DrvKey = cur_seq;
        }
    }
    {
        uint8_t cur_seq = in->AppCommMgr_params->seq;
        if (cur_seq != s_last_seq_AppCommMgr) {
            flags.bits.appcommmgr = 1;
            s_last_seq_AppCommMgr = cur_seq;
        }
    }

    /* === 计算段: 用户业务 === */
    user_Process(in, out, flags);
}

MODULE_EXPORT(AppCooking);

// ===== [END AI GENERATED] =====
#include "core/std_module.h"
#include "../include_io/app_cooking_io.h"
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

#if 0  /* DEDUP */
#if 0  /* DEDUP */
MODULE_SKELETON(AppCooking);
#endif  /* DEDUP */
#endif  /* DEDUP */

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
}

static void out_disp(uint8_t h, uint8_t cmd)
{
    OutData_t *o = (OutData_t *)g_output.para;
    o->has_display = 1;
    o->disp_head = h;  o->disp_cmd = cmd;
    g_output.info.route = g_input.info.route;
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

/* ========== route 分发: 替代 V1 OnKey/OnRegData/OnTimer1s 回调 ========== */
static void user_Process(MODULE_INPUT(AppCooking) *in, MODULE_OUTPUT(AppCooking) *out, AppCooking_PipeFlags_t flags)
{
    if (flags.bits.drvkey) {
        MODULE_INPUT_PARAMS(DrvKey, AppCooking) *p = in->DrvKey_params->params;
        uint8_t kc = p->key_code, ks = p->key_state;
        if (kc == 1u && ks == 1u) s_selected_head = (s_selected_head + 1u) % COOK_HEAD_COUNT;
        if (kc == 2u && ks == 1u) on_cmd(s_selected_head, 2u, 0u);
        if (kc == 4u && ks == 1u) on_cmd(s_selected_head, 1u, 1u);
        if (kc >= 0x10u && kc <= 0x19u && ks == 1u) on_cmd(s_selected_head, 5u, kc - 0x10u);
    }
    if (flags.bits.appcommmgr) {
        MODULE_INPUT_PARAMS(AppCommMgr, AppCooking) *p = in->AppCommMgr_params->params;
        if (p->head_index < COOK_HEAD_COUNT) {
            s_ctx[p->head_index].current_temp = (uint8_t)(p->regs[3] & 0xFFu);
        }
    }
    (void)out;
}

/* =================================================================
 * Init
 * ================================================================= */

static void Init(void)
{
    memset(s_ctx, 0, sizeof(s_ctx));
    for (uint8_t i = 0; i < COOK_HEAD_COUNT; i++) s_ctx[i].current_temp = 25;
    s_selected_head = 0;
    g_input.para  = &s_inPara;
    g_output.para = &s_outPara;
    memset(&s_outPara, 0, sizeof(s_outPara));
    s_AppCookingToAppPowerLink.params = &s_AppCookingToAppPowerParams;
    s_outPara.AppPower_params         = &s_AppCookingToAppPowerLink;
}

/* =================================================================
 * 导出
 * ================================================================= */

#if 0  /* DEDUP */
#if 0  /* DEDUP */
MODULE_EXPORT(AppCooking);
#endif  /* DEDUP */
#endif  /* DEDUP */


