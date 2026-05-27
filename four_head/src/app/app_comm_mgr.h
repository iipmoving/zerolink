/**
 * app_comm_mgr.h —— 4炉头 MODBUS 轮询调度器接口
 *
 * 依赖: <stdint.h> (无其他模块依赖 — proto 通过 __weak 对接)
 * 层级: APP —— 应用层通信管理 + 通讯数据集中缓存
 *
 * 职责:
 *   1. MODBUS 轮询调度 (100ms/炉头, 4头循环)
 *   2. 通讯数据集中缓存 (s_heads[].regs) — 寄存器数据的唯一真相源
 *   3. 协议抽象 — 通过 __weak Proto_BuildRead/Parse 对接 proto/ 层
 *
 * 数据广播:
 *   收到有效响应后 → AppPower_OnRegData / AppCooking_OnRegData /
 *                    AppProtect_OnRegData 三条 __weak 广播
 *   其他模块通过 __weak 回调直接拿到 RegData_t 指针，不持有通讯状态
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
