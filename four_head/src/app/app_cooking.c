/**
 * app_cooking.c —— 4炉头烹饪状态机实现
 *
 * 依赖: app_cooking.h + core/msg_scheduler.h
 * 层级: APP —— 应用层烹饪管理
 * 零交叉include: 不include任何其他APP模块头文件
 *
 * 参考: 参考程序 User_Main/APP/Cooking_Menu.c + Key_dispose.c
 *
 * 简化说明:
 *   参考程序使用预配置的大量菜单(4457行)和闭源库(PID/斜率),
 *   本模块用简化滞后控制+沸腾检测替代,自包含且可扩展。
 */
#include "app_cooking.h"
#include <stddef.h>

/* __weak 回调: 发送方定义的空壳, 接收方定义强符号, 链接器自动接线 */
__weak void AppPower_OnPowerCtrl(uint16_t param, void *data_ptr)
{ (void)param; (void)data_ptr; }
__weak void DrvDisplay_OnRefresh(uint16_t param, void *data_ptr)
{ (void)param; (void)data_ptr; }

/* ========== 功率等级表 (与app_power.h的s_power_table保持同步) ========== */
static const uint16_t s_power_table[] = {
    0,    200,  300,  400,  500,
    1000, 1100, 1200, 1300, 1500,
    2000
};

/* ========== 本地功率控制结构(与app_power.h的PowerCtrl_t布局一致) ========== */
typedef struct {
    uint8_t  head_index;
    uint8_t  onoff;
    uint16_t target_power;
    uint8_t  power_level;
    uint8_t  work_mode;
    uint16_t target_temp;
} LocalPowerCtrl_t;

/* ========== 炉头烹饪上下文 ========== */
typedef struct {
    CookState_t state;
    uint8_t     menu_id;
    uint8_t     step_index;
    uint16_t    step_timer;
    uint8_t     current_temp;
    uint8_t     user_power_lv;
    uint8_t     boil_samples;
    uint16_t    boil_temp_acc;
} CookCtx_t;

/* ========== Flash菜单定义 ========== */

/* 菜单1: 保温 65°C */
static const CookStep_t s_steps_keepwarm[] = {
    {COOK_STEP_TEMP_HOLD, 3, 0, 65, 0}
};

/* 菜单2: 火锅 (默认档位5=1000W, 用户可调) */
static const CookStep_t s_steps_hotpot[] = {
    {COOK_STEP_POWER, 5, 0, 0, 0}
};

/* 菜单3: 煮沸后慢炖 (全功率烧开→300W慢炖30分钟) */
static const CookStep_t s_steps_boil[] = {
    {COOK_STEP_BOIL,  10, 0,    0, 0},
    {COOK_STEP_POWER, 2,  1800, 0, 0}
};

static const CookMenu_t s_menus[COOK_MENU_MAX] = {
    {0, NULL},              /* 索引0: 空 */
    {1, s_steps_keepwarm},  /* 索引1: 保温 */
    {1, s_steps_hotpot},    /* 索引2: 火锅 */
    {2, s_steps_boil},      /* 索引3: 煮沸后慢炖 */
};

/* ========== 全局状态 ========== */
static CookCtx_t        s_ctx[COOK_HEAD_COUNT];
static uint16_t         s_tick_10ms;
static LocalPowerCtrl_t s_power_cmd;  /* static: Msg_Post传递指针 */
static uint8_t          s_selected_head;  /* 当前选中的炉头 */

/* ========== 内部: 沸腾检测 ========== */
static uint8_t detect_boil(CookCtx_t *ctx)
{
    if (ctx->current_temp < 90u) {
        ctx->boil_samples  = 0u;
        ctx->boil_temp_acc = 0u;
        return 0u;
    }

    ctx->boil_samples++;
    ctx->boil_temp_acc += (uint16_t)ctx->current_temp;

    if (ctx->boil_samples >= 3u) {
        uint8_t avg = (uint8_t)(ctx->boil_temp_acc / ctx->boil_samples);
        int16_t diff = (int16_t)ctx->current_temp - (int16_t)avg;
        if (diff < 0) diff = -diff;
        if (diff < 2) {
            ctx->boil_samples  = 0u;
            ctx->boil_temp_acc = 0u;
            return 1u;  /* 沸腾确认 */
        }
    }
    return 0u;
}

/* ========== 内部: 温度滞后控制 ========== */
static uint16_t temp_hysteresis(CookCtx_t *ctx, uint8_t target, uint8_t power_lv)
{
    uint8_t temp = ctx->current_temp;

    if (temp < target - 3u) {
        return s_power_table[power_lv];  /* 低于目标,全功率加热 */
    }
    if (temp > target) {
        return 0u;  /* 超过目标,停止加热 */
    }
    /* 在目标±3℃区间内,维持1/3功率 */
    if (power_lv > 2u) {
        return s_power_table[power_lv - 2u];
    }
    return s_power_table[1];  /* 最低200W */
}

/* ========== 内部: 发送功率命令 ========== */
static void post_power_cmd(uint8_t head_idx, uint8_t onoff,
                           uint16_t power, uint8_t level, uint8_t mode,
                           uint16_t temp)
{
    s_power_cmd.head_index   = head_idx;
    s_power_cmd.onoff        = onoff;
    s_power_cmd.target_power = power;
    s_power_cmd.power_level  = level;
    s_power_cmd.work_mode    = mode;
    s_power_cmd.target_temp  = temp;
    AppPower_OnPowerCtrl((uint16_t)head_idx, &s_power_cmd);
}

/* ========== 内部: 发送显示刷新 ========== */
static void post_display(uint8_t head_idx, uint8_t disp_cmd)
{
    DrvDisplay_OnRefresh(
        (uint16_t)(((uint16_t)head_idx << 8) | disp_cmd),
        NULL);
}

/* ========== 内部: 烹饪控制(原自收MSG, 现直调) ========== */
static void on_cooking_ctrl(uint16_t param, void *data_ptr)
{
    uint8_t  head_idx;
    uint8_t  cmd;
    uint8_t  menu_id;
    CookCtx_t *ctx;
    (void)data_ptr;

    head_idx = (uint8_t)COOK_PARAM_GET_HEAD(param);
    cmd      = (uint8_t)COOK_PARAM_GET_CMD(param);
    menu_id  = (uint8_t)COOK_PARAM_GET_MENU(param);

    if (head_idx >= COOK_HEAD_COUNT) return;

    ctx = &s_ctx[head_idx];

    switch (cmd) {
    case COOK_CMD_START:
        if (menu_id == 0u || menu_id >= COOK_MENU_MAX) return;
        if (s_menus[menu_id].step_count == 0u) return;
        ctx->menu_id    = menu_id;
        ctx->step_index = 0u;
        ctx->step_timer = s_menus[menu_id].steps[0].duration_sec;
        ctx->boil_samples  = 0u;
        ctx->boil_temp_acc = 0u;
        ctx->state = COOK_STA_RUN;
        post_display(head_idx, 1u);  /* 烹饪中 */
        break;

    case COOK_CMD_STOP:
        ctx->state = COOK_STA_OFF;
        ctx->menu_id = 0u;
        post_power_cmd(head_idx, 0u, 0u, 0u, 0u, 0u);
        post_display(head_idx, 0u);  /* 关机 */
        break;

    case COOK_CMD_PAUSE:
        if (ctx->state == COOK_STA_RUN) {
            ctx->state = COOK_STA_PAUSE;
            post_power_cmd(head_idx, 0u, 0u, 0u, 0u, 0u);
        }
        break;

    case COOK_CMD_RESUME:
        if (ctx->state == COOK_STA_PAUSE) {
            ctx->state = COOK_STA_RUN;
        }
        break;

    case COOK_CMD_SET_POWER:
        if (menu_id <= COOK_POWER_LV_MAX) {
            ctx->user_power_lv = menu_id;  /* menu_id字段复用为power_level */
        }
        break;

    default:
        break;
    }
}

/* ========== __weak 接收: 由 drv_key 直调 ========== */
void AppCooking_OnKey(uint16_t param, void *data_ptr)
{
    uint8_t key_code;
    uint8_t key_state;
    (void)data_ptr;

    key_code  = (uint8_t)(param & 0xFFu);
    key_state = (uint8_t)((param >> 8) & 0xFFu);

    /* KEY_ONOFF短按: 切换炉头选择 */
    if (key_code == 0x01u && key_state == 1u) {  /* KEY_STATE_PRESS */
        s_selected_head = (s_selected_head + 1u) % COOK_HEAD_COUNT;
    }

    /* KEY_STOP: 停止当前选中炉头 */
    if (key_code == 0x02u && key_state == 1u) {
        on_cooking_ctrl(
            COOK_PARAM(s_selected_head, COOK_CMD_STOP, 0), NULL);
    }

    /* KEY_MENU: 启动保温菜单 */
    if (key_code == 0x04u && key_state == 1u) {
        on_cooking_ctrl(
            COOK_PARAM(s_selected_head, COOK_CMD_START, COOK_MENU_KEEPWARM),
            NULL);
    }

    /* KEY_POWER_0~9: 功率调节 */
    if (key_code >= 0x10u && key_code <= 0x19u && key_state == 1u) {
        uint8_t lv = key_code - 0x10u;
        on_cooking_ctrl(
            COOK_PARAM(s_selected_head, COOK_CMD_SET_POWER, lv),
            NULL);
    }
}

/* ========== __weak 接收: 由 app_comm_mgr 直调 ========== */
void AppCooking_OnRegData(uint16_t param, void *data_ptr)
{
    uint8_t  head_idx;
    uint8_t *reg_data;
    CookCtx_t *ctx;

    if (data_ptr == NULL) return;

    head_idx = (uint8_t)(param & 0xFFu);
    if (head_idx >= COOK_HEAD_COUNT) return;

    ctx = &s_ctx[head_idx];

    /* RegData_t: head_index(1) + slave_addr(1) + online(1) + padding(1) + regs[22](44) */
    /* current_temp from regs[3] (BOT AD), offset = 4 + 3*2 = 10 */
    reg_data = (uint8_t *)data_ptr;
    ctx->current_temp = reg_data[10];  /* regs[3]的低字节(AD值低8位) */
}

/* ========== __weak 接收: 由 main 直调 ========== */
void AppCooking_OnTimer1s(uint16_t param, void *data_ptr)
{
    uint8_t i;
    (void)param;
    (void)data_ptr;

    for (i = 0u; i < COOK_HEAD_COUNT; i++) {
        CookCtx_t *ctx = &s_ctx[i];

        if (ctx->state != COOK_STA_RUN) continue;

        if (ctx->menu_id == 0u || ctx->menu_id >= COOK_MENU_MAX) continue;

        {
            const CookMenu_t *menu = &s_menus[ctx->menu_id];
            const CookStep_t *step;
            uint8_t next_step;

            if (ctx->step_index >= menu->step_count) continue;

            step = &menu->steps[ctx->step_index];

            /* 步骤计时 */
            if (ctx->step_timer > 0u) {
                ctx->step_timer--;
            }

            next_step = 0u;  /* 默认不推进步骤 */

            switch (step->type) {
            case COOK_STEP_POWER: {
                uint8_t lv = step->power_level;
                if (ctx->user_power_lv > 0u) {
                    lv = ctx->user_power_lv;
                }
                post_power_cmd(i, 1u, s_power_table[lv], lv, 0u, 0u);
                if (ctx->step_timer == 0u && step->duration_sec > 0u) {
                    next_step = 1u;
                }
                break;
            }

            case COOK_STEP_BOIL: {
                post_power_cmd(i, 1u,
                    s_power_table[step->power_level],
                    step->power_level, 0u, 0u);
                if (detect_boil(ctx)) {
                    next_step = 1u;
                }
                break;
            }

            case COOK_STEP_TEMP_HOLD: {
                uint16_t power;
                uint8_t lv = step->power_level;
                if (ctx->user_power_lv > 0u) {
                    lv = ctx->user_power_lv;
                }
                power = temp_hysteresis(ctx, step->target_temp, lv);
                post_power_cmd(i, (power > 0u) ? 1u : 0u, power, lv, 1u,
                              (uint16_t)step->target_temp);
                /* TEMP_HOLD: 不自动推进, duration_sec=0=无限 */
                if (step->duration_sec > 0u && ctx->step_timer == 0u) {
                    next_step = 1u;
                }
                break;
            }

            case COOK_STEP_CYCLE: {
                uint16_t power;
                if ((ctx->step_timer % 6u) < 3u) {
                    power = s_power_table[step->power_level];
                } else {
                    power = 0u;
                }
                post_power_cmd(i, (power > 0u) ? 1u : 0u, power,
                    step->power_level, 0u, 0u);
                if (ctx->step_timer == 0u && step->duration_sec > 0u) {
                    next_step = 1u;
                }
                break;
            }

            case COOK_STEP_STOP:
            default:
                next_step = 1u;
                break;
            }

            /* 推进到下一步 */
            if (next_step) {
                ctx->step_index++;
                if (ctx->step_index >= menu->step_count) {
                    /* 所有步骤完成 */
                    ctx->state = COOK_STA_COMPLETE;
                    post_power_cmd(i, 0u, 0u, 0u, 0u, 0u);
                    post_display(i, 2u);  /* 完成 */
                } else {
                    /* 加载下一步 */
                    ctx->step_timer = menu->steps[ctx->step_index].duration_sec;
                    ctx->boil_samples  = 0u;
                    ctx->boil_temp_acc = 0u;
                }
            }
        }
    }
}

/* ========== 初始化 ========== */
void App_Cooking_Init(void)
{
    uint8_t i;

    for (i = 0u; i < COOK_HEAD_COUNT; i++) {
        s_ctx[i].state          = COOK_STA_OFF;
        s_ctx[i].menu_id        = 0u;
        s_ctx[i].step_index     = 0u;
        s_ctx[i].step_timer     = 0u;
        s_ctx[i].current_temp   = 25u;
        s_ctx[i].user_power_lv  = 0u;
        s_ctx[i].boil_samples   = 0u;
        s_ctx[i].boil_temp_acc  = 0u;
    }
    s_tick_10ms      = 0u;
    s_selected_head  = 0u;
}

/* ========== 每10ms槽位6调用 ========== */
void App_Cooking_Run(void)
{
    /* 烹饪逻辑全部在 MSG_TIMER_1S 回调中执行,
       本函数仅做占位和未来扩展预留 */
    (void)s_tick_10ms;
}
