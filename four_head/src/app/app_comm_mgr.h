/**
 * app_comm_mgr.h —— 4炉头 MODBUS 轮询调度器接口
 *
 * 依赖: msg_scheduler.h + proto_modbus.h + hal_comm.h
 * 层级: APP —— 应用层通信管理
 *
 * 调度:
 *   每10ms槽位调用 Run()，内部100ms节拍触发一轮轮询
 *   4炉头按 5→10→15→20 循环，每炉头间隔100ms
 *
 * 寄存器缓存:
 *   每炉头保存 0x1000-0x1015 共22个寄存器
 *   数据更新时发送 MSG_REG_DATA_READY
 */
#ifndef APP_COMM_MGR_H
#define APP_COMM_MGR_H

#include <stdint.h>

/* ========== 炉头数量与站号 ========== */
#define COMM_HEAD_COUNT          4u
#define COMM_SLAVE_ADDR_BASE     5u
#define COMM_SLAVE_ADDR_STEP     5u

/* ========== 寄存器定义 ========== */
#define COMM_REG_STATUS          0x1000u  /* 系统状态     bit7=初始化完成 */
#define COMM_REG_COUNT           22u      /* 每炉头寄存器数 0x1000-0x1015 */

/* ========== 时序参数 ========== */
#define COMM_POLL_PERIOD_10MS    10u      /* 100ms轮询周期 = 10×10ms        */
#define COMM_RETRY_MAX           3u       /* 最大重试次数                     */
#define COMM_TIMEOUT_10MS        5u       /* 响应超时 50ms = 5×10ms          */

/* ========== 炉头状态 ========== */
typedef enum {
    COMM_STATE_DISCONNECTED,  /* 离线/未连接                                 */
    COMM_STATE_CHECK_INIT,    /* 检查初始化状态                               */
    COMM_STATE_POLLING        /* 正常轮询                                    */
} CommState_t;

/* ========== 寄存器数据（MSG_REG_DATA_READY 携带）========== */
typedef struct {
    uint8_t  head_index;          /* 炉头索引 0-3                             */
    uint8_t  slave_addr;          /* MODBUS 站号                              */
    uint8_t  online;              /* 是否在线                                 */
    uint16_t regs[COMM_REG_COUNT];/* 寄存器值 0x1000-0x1015                   */
} RegData_t;

/* ========== 公共接口 ========== */
void App_CommMgr_Init(void);
void App_CommMgr_Run(void);  /* 每10ms槽位4调用 */

#endif /* APP_COMM_MGR_H */
