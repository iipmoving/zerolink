/**
 * app_protect.h —— 4炉头故障检测与保护接口
 *
 * 依赖: msg_def.h + app_comm_mgr.h (RegData_t)
 * 层级: APP —— 应用层故障检测
 *
 * 参考: 参考程序 User_Main/APP/Err_Check.c
 *
 * 调度:
 *   每10ms槽位5调用 Run()
 *   内部自计数: 100ms检测一次各炉头
 *
 * 检测项目:
 *   IGBT NTC: 开路/短路/超温 (AD阈值判断)
 *   BOT NTC:  开路/短路/超温
 *   电压:     高压>260V / 低压<150V
 *   通讯:     超时断开
 *   传感器失效: AD值长时间不变
 *
 * 故障状态通过 MSG_SYSTEM_ERROR 广播
 */
#ifndef APP_PROTECT_H
//#define APP_PROTECT_H

#include <stdint.h>

/* ========== 炉头数量 ========== */
#define PROTECT_HEAD_COUNT   4u

/* ========== 寄存器索引 (对应0x1000起始的22个寄存器) ========== */
#define PROTECT_REG_STATUS   0u   /* 0x1000: 系统状态 bit7=init_ok */
#define PROTECT_REG_ERR      1u   /* 0x1001: 电源板故障标志 */
#define PROTECT_REG_IGBT_AD  2u   /* 0x1002: IGBT NTC AD值 */
#define PROTECT_REG_BOT_AD   3u   /* 0x1003: 炉面 NTC AD值 */
#define PROTECT_REG_VOL_AD   4u   /* 0x1004: 电压 AD值 */
#define PROTECT_REG_CUR_AD   5u   /* 0x1005: 电流 AD值 */
#define PROTECT_REG_FAN      6u   /* 0x1006: 风扇转速 */

/* ========== NTC AD 阈值 (参考 Err_Check.c 配置) ========== */
#define PROTECT_IGBT_OPEN_AD    3u    /* IGBT开路: AD<3 (约-30℃) */
#define PROTECT_IGBT_SHORT_AD   230u  /* IGBT短路: AD>230 (约296℃) */
#define PROTECT_IGBT_HIGH_AD    184u  /* IGBT超温: AD>184 (约95℃) */
#define PROTECT_IGBT_RECOVER_AD 138u  /* IGBT恢复: AD<138 (约71℃) */

#define PROTECT_BOT_OPEN_AD     3u    /* 炉面开路: AD<3 */
#define PROTECT_BOT_SHORT_AD    252u  /* 炉面短路: AD>252 */
#define PROTECT_BOT_HIGH_AD     235u  /* 炉面超温: AD>235 (约240℃) */
#define PROTECT_BOT_RECOVER_AD  236u  /* 炉面恢复: 高于超温值=不可自动恢复 */

/* ========== 电压阈值 (AD值, 对应参考程序 Vol_HL_Configuration) ========== */
#define PROTECT_VOL_HIGH_V      260u  /* 高压阈值 (V) */
#define PROTECT_VOL_LOW_V       150u  /* 低压阈值 (V) */

/* ========== 时序参数 ========== */
#define PROTECT_CHECK_PERIOD_100MS  10u  /* 100ms检测周期 = 10×10ms */
#define PROTECT_ERR_DELAY_100MS     30u  /* 故障延迟: 3秒 = 30×100ms */
#define PROTECT_RECOVER_DELAY_100MS 30u  /* 恢复延迟: 3秒 = 30×100ms */
#define PROTECT_COMM_TIMEOUT_100MS  50u  /* 通讯超时: 5秒 = 50×100ms */

/* ========== 故障位定义 (参考 Err_Check.h bit位) ========== */
typedef union {
    uint16_t byte;
    struct {
        uint16_t igbt_short   : 1;  /* bit0: IGBT散热片短路 */
        uint16_t igbt_open    : 1;  /* bit1: IGBT散热片开路 */
        uint16_t igbt_high    : 1;  /* bit2: IGBT散热片超温 */
        uint16_t comm_open    : 1;  /* bit3: 通讯断开 */
        uint16_t bot_short    : 1;  /* bit4: 炉面传感器短路 */
        uint16_t bot_open     : 1;  /* bit5: 炉面传感器开路 */
        uint16_t bot_high     : 1;  /* bit6: 炉面传感器超温 */
        uint16_t bot_sx       : 1;  /* bit7: 炉面传感器失效 */
        uint16_t vol_high     : 1;  /* bit8: 电压过高 */
        uint16_t vol_low      : 1;  /* bit9: 电压过低 */
        uint16_t fan_open     : 1;  /* bit10: 风扇断开 */
        uint16_t water_open   : 1;  /* bit11: 进水异常 */
        uint16_t reserved1    : 1;  /* bit12 */
        uint16_t reserved2    : 1;  /* bit13 */
        uint16_t hardware     : 1;  /* bit14: 硬件故障 */
        uint16_t reserved3    : 1;  /* bit15 */
    } bits;
} ProtectFault_t;

/* ========== MSG_SYSTEM_ERROR 携带的数据 ========== */
typedef struct {
    uint8_t        head_index;  /* 炉头索引 0-3 */
    uint8_t        slave_addr;  /* MODBUS站号 */
    ProtectFault_t fault;       /* 当前故障位集合 */
} ProtectEvent_t;

/* ========== 公共接口 ========== */
void App_Protect_Init(void);
void App_Protect_Run(void);  /* 每10ms槽位5调用 */

#endif /* APP_PROTECT_H */
