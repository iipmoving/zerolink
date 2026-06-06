/**
 * app_power.h —— 4炉头功率管理与输出控制接口
 *
 * 依赖: msg_scheduler.h
 * 层级: APP —— 应用层功率管理
 *
 * 参考: 参考程序 User_Main/APP/work_power_out.c
 *
 * 功能:
 *   接收 COOK_MSG_POWER_OUT 设定目标功率
 *   响应 PROT_MSG_ERROR_OUT 调整/关断输出
 *   应用保护策略: IGBT高温降功率、炉面高温停止、间断加热
 *   输出抽象功率值(不关心传输协议)
 *
 * 调度:
 *   每10ms槽位2调用 Run()
 *   内部100ms节拍执行功率输出逻辑
 *
 * 功率等级表 (参考 DEF_POWER_LV_PCL):
 *   0W, 200W, 300W, 400W, 500W, 1000W, 1100W,
 *   1200W, 1300W, 1500W, 2000W
 */
#ifndef APP_POWER_H
//#define APP_POWER_H

#include <stdint.h>

/* ========== 炉头数量 ========== */
#define POWER_HEAD_COUNT    4u

/* ========== 工作状态 ========== */
typedef enum {
    POWER_STA_OFF,        /* 关机 */
    POWER_STA_IDLE,       /* 待机(在线但未加热) */
    POWER_STA_RUN,        /* 正常运行 */
    POWER_STA_PROTECT,    /* 保护降功率 */
    POWER_STA_ERROR       /* 故障停止 */
} PowerState_t;

/* ========== MSG_POWER_CTRL 携带的数据 ========== */
typedef struct {
    uint8_t  head_index;     /* 炉头索引 0-3 */
    uint8_t  onoff;          /* 1=开机加热, 0=关机 */
    uint16_t target_power;   /* 目标功率(W), 0=停止 */
    uint8_t  power_level;    /* 功率档位 0-10 */
    uint8_t  work_mode;      /* 工作模式: 0=功率, 1=温度 */
    uint16_t target_temp;    /* 目标温度(℃), 温度模式有效 */
} PowerCtrl_t;

/* ========== 功率等级表 ========== */
#define POWER_LV_MAX         10u
#define POWER_LV_MAX_WATT    2000u

/* ========== 时序参数 ========== */
#define POWER_RUN_PERIOD_100MS  10u  /* 100ms执行周期 = 10×10ms */

/* ========== 保护参数 ========== */
#define POWER_IGBT_DOWN_START  80u  /* IGBT开始降功率温度(℃) */
#define POWER_IGBT_LIMIT_TEMP  92u  /* IGBT极限保护温度(℃) */
#define POWER_TOP_STOP_TEMP    190u /* 炉面停止加热温度(℃) */
#define POWER_TOP_RESUME_TEMP  180u /* 炉面恢复加热温度(℃) */
#define POWER_HIGH_SOFT_START  2000u /* 高功率软启动阈值(W) */
#define POWER_SOFT_START_TIME  60u  /* 软启动时间(×100ms=6秒) */

/* ========== 公共接口 ========== */
/* v2.0: Switcher 入口（data_switcher.h 声明 GetIO + DoWork）*/
void AppPower_DoWork(void);

#endif /* APP_POWER_H */
