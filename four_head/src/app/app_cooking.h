/**
 * app_cooking.h —— 4炉头烹饪状态机接口
 *
 * 依赖: msg_def.h（唯一）
 * 层级: APP —— 应用层烹饪管理
 *
 * 参考: 参考程序 User_Main/APP/Cooking_Menu.c + Key_dispose.c
 *
 * 设计:
 *   每炉头独立状态机: OFF → RUN → PAUSE/COMPLETE
 *   菜单由多个步骤组成, 支持: 功率/煮沸/恒温/循环
 *   自计时(数10ms调用次数), 不依赖外部定时器
 *
 * 消息:
 *   订阅: MSG_COOKING_CTRL, MSG_KEY_EVENT, MSG_REG_DATA_READY, MSG_TIMER_1S
 *   发布: MSG_POWER_CTRL, MSG_DISPLAY_REFRESH
 */
#ifndef APP_COOKING_H
#define APP_COOKING_H

#include <stdint.h>

/* ========== 配置常量 ========== */
#define COOK_HEAD_COUNT     4u
#define COOK_STEPS_MAX      8u
#define COOK_MENU_MAX       8u
#define COOK_POWER_LV_MAX   10u

/* ========== 烹饪状态 ========== */
typedef enum {
    COOK_STA_OFF      = 0,  /* 未激活 */
    COOK_STA_RUN      = 1,  /* 执行中 */
    COOK_STA_PAUSE    = 2,  /* 暂停 */
    COOK_STA_COMPLETE = 3   /* 完成,等待清除 */
} CookState_t;

/* ========== 步骤类型 ========== */
typedef enum {
    COOK_STEP_STOP      = 0,  /* 结束标记 */
    COOK_STEP_POWER     = 1,  /* 固定功率,计时结束 */
    COOK_STEP_BOIL      = 2,  /* 全功率烧开,检测沸腾 */
    COOK_STEP_TEMP_HOLD = 3,  /* 维持目标温度(滞后控制) */
    COOK_STEP_CYCLE     = 4   /* 循环开关加热 */
} CookStepType_t;

/* ========== 烹饪步骤定义(Flash常量) ========== */
typedef struct {
    uint8_t  type;           /* CookStepType_t */
    uint8_t  power_level;    /* 功率档位 0-10 */
    uint16_t duration_sec;   /* 持续时间(秒), 0=无限 */
    uint8_t  target_temp;    /* 目标温度(℃), 0=不使用 */
    uint8_t  reserved;       /* 对齐填充 */
} CookStep_t;

/* ========== 菜单定义(Flash常量) ========== */
typedef struct {
    uint8_t           step_count;
    const CookStep_t *steps;
} CookMenu_t;

/* ========== 烹饪命令编码 ========== */
#define COOK_CMD_NONE       0u
#define COOK_CMD_START      1u
#define COOK_CMD_STOP       2u
#define COOK_CMD_PAUSE      3u
#define COOK_CMD_RESUME     4u
#define COOK_CMD_SET_POWER  5u
#define COOK_CMD_SET_TEMP   6u

/* MSG_COOKING_CTRL param 编码:
 *   [15:12] menu_id  (0-7)
 *   [11:8]  reserved
 *   [7:4]   head_idx (0-3)
 *   [3:0]   cmd      (COOK_CMD_*) */
#define COOK_PARAM(head, cmd, menu) \
    ((uint16_t)(((menu) & 0xFu) << 12) | \
     (uint16_t)(((head) & 0xFu) << 4)  | \
     (uint16_t)((cmd) & 0xFu))

#define COOK_PARAM_GET_HEAD(p)  (((p) >> 4) & 0xFu)
#define COOK_PARAM_GET_CMD(p)   ((p) & 0xFu)
#define COOK_PARAM_GET_MENU(p)  (((p) >> 12) & 0xFu)

/* ========== 预定义菜单ID ========== */
#define COOK_MENU_KEEPWARM  1u
#define COOK_MENU_HOTPOT    2u
#define COOK_MENU_BOIL_SIMMER 3u

/* ========== 公共接口 ========== */
void App_Cooking_Init(void);
void App_Cooking_Run(void);  /* 每10ms槽位6调用 */

#endif /* APP_COOKING_H */
