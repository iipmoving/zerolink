/**
 * app_comm_mgr.c —— 4炉头 MODBUS 轮询调度器实现
 *
 * 依赖: app_comm_mgr.h + msg_scheduler.h + proto_modbus.h
 * 层级: APP —— 应用层通信管理（通过消息与 DRV 通讯）
 *
 * 通讯路径:
 *   APP 发: Msg_Post(MSG_COMM_SEND_REQ) → DRV → Drv_Comm_Send → HAL
 *   APP 收: DRV → Msg_Post(MSG_COMM_DATA_UPDATE) → APP handle_response
 *   同步:   DRV → Msg_Post(MSG_COMM_TX_DONE)     → APP 状态机推进
 */
#include "app_comm_mgr.h"
#include "proto/proto_modbus.h"
#include <string.h>
#include <stddef.h>

/* __weak 回调: 多接收方广播, 链接器自动接线, interface_map.h 文档化 */
__weak void AppPower_OnRegData(uint16_t param, void *data_ptr)
{ (void)param; (void)data_ptr; }
__weak void AppCooking_OnRegData(uint16_t param, void *data_ptr)
{ (void)param; (void)data_ptr; }
__weak void AppProtect_OnRegData(uint16_t param, void *data_ptr)
{ (void)param; (void)data_ptr; }
__weak void DrvCommMgr_OnSendReq(uint16_t param, void *data_ptr)
{ (void)param; (void)data_ptr; }

/* ========== MODBUS功率控制寄存器(唯一知道这些地址的模块) ========== */
#define COMM_POWER_REG_BASE      0x2000u
#define COMM_POWER_REG_POWERSET  0u      /* 0x2000: 功率设定 = W/25 */
#define COMM_POWER_REG_SWITCH    1u      /* 0x2001: 电源开关控制 */
#define COMM_POWER_REG_FAN       2u      /* 0x2002: 风扇转速(预留) */
#define COMM_POWER_REG_K_VALUE   3u      /* 0x2003: K值(预留) */

/* ========== 独立类型声明（与 DRV 层 DrvComm* 布局一致）========== */
#define APP_COMM_SEND_BUF_SIZE  64u
#define APP_COMM_RX_BUF_SIZE    256u

typedef struct {
    uint8_t  data[APP_COMM_SEND_BUF_SIZE];
    uint16_t len;
} CommSendReq_t;

typedef struct {
    uint8_t  data[APP_COMM_RX_BUF_SIZE];
    uint16_t len;
} CommDataUpdate_t;

/* 与 app_power.c 的 PowerOutput_t 布局一致 */
typedef struct {
    uint8_t  head_idx;
    uint16_t power_watt;
} CommPowerCmd_t;

/* ========== 全局轮询状态 ========== */
#define COMM_S_IDLE          0u
#define COMM_S_WAIT_TX       1u
#define COMM_S_WAIT_RX       2u

/* ========== 炉头上下文 ========== */
typedef struct {
    CommState_t state;              /* 连接状态                            */
    uint8_t     slave_addr;         /* MODBUS 站号                         */
    uint16_t    regs[COMM_REG_COUNT]; /* 寄存器缓存                         */
    uint8_t     retry_cnt;          /* 当前重试计数                        */
    uint8_t     timeout_cnt;        /* 响应超时计数 (10ms单位)             */
} HeadCtx_t;

static HeadCtx_t s_heads[COMM_HEAD_COUNT];
static uint8_t   s_cur_head;       /* 当前轮询炉头索引 0-3                 */
static uint8_t   s_poll_state;     /* 轮询子状态                           */
static uint8_t   s_tick_10ms;      /* 10ms节拍计数                        */
/* ========== 内部: 发送读寄存器请求 ========== */
static void send_read_req(uint8_t head_idx)
{
    HeadCtx_t *ctx;
    uint16_t   frame_len;
    static CommSendReq_t s_tx_req;

    ctx = &s_heads[head_idx];
    frame_len = Proto_Modbus_BuildRead(ctx->slave_addr,
                                       COMM_REG_STATUS,
                                       COMM_REG_COUNT,
                                       s_tx_req.data);
    s_tx_req.len = frame_len;
    DrvCommMgr_OnSendReq((uint16_t)head_idx, &s_tx_req);
    s_poll_state = COMM_S_WAIT_TX;
    ctx->timeout_cnt = 0u;
}

/* ========== 内部: 处理响应帧（MSG_COMM_DATA_UPDATE 回调）========== */
static void handle_response(const uint8_t *rx_data, uint16_t frame_len)
{
    int8_t    result;
    uint8_t   slave, func;
    uint16_t  data[COMM_REG_COUNT];
    uint16_t  count;
    uint8_t   head_idx;
    HeadCtx_t *ctx;
    static RegData_t s_reg_data;

    if (frame_len == 0u) return;

    result = Proto_Modbus_Parse(rx_data, frame_len,
                                &slave, &func, data, &count);
    if (result != PROTO_MODBUS_OK) return;

    /* 查找对应炉头 */
    for (head_idx = 0u; head_idx < COMM_HEAD_COUNT; head_idx++) {
        if (s_heads[head_idx].slave_addr == slave) break;
    }
    if (head_idx >= COMM_HEAD_COUNT) return;

    ctx = &s_heads[head_idx];

    if (func == MODBUS_FUNC_READ && count > 0u) {
        uint16_t i;
        for (i = 0u; i < count && i < COMM_REG_COUNT; i++) {
            ctx->regs[i] = data[i];
        }
        ctx->state  = COMM_STATE_POLLING;
        ctx->retry_cnt = 0u;

        /* 广播寄存器数据到三个接收方 */
        s_reg_data.head_index = head_idx;
        s_reg_data.slave_addr = slave;
        s_reg_data.online     = 1u;
        for (i = 0u; i < COMM_REG_COUNT; i++) {
            s_reg_data.regs[i] = ctx->regs[i];
        }
        AppPower_OnRegData((uint16_t)head_idx, &s_reg_data);
        AppCooking_OnRegData((uint16_t)head_idx, &s_reg_data);
        AppProtect_OnRegData((uint16_t)head_idx, &s_reg_data);
    }
}

/* ========== __weak 接收: 由 drv_comm_mgr 直调 ========== */
void AppCommMgr_OnDataUpdate(uint16_t param, void *data_ptr)
{
    CommDataUpdate_t *rx;

    if (data_ptr == NULL) return;
    rx = (CommDataUpdate_t *)data_ptr;
    if (rx->len == 0u) return;

    (void)param;
    handle_response(rx->data, rx->len);
    s_poll_state = COMM_S_IDLE;
    s_cur_head = (s_cur_head + 1u) % COMM_HEAD_COUNT;
}

/* ========== __weak 接收: 由 drv_comm_mgr 直调 ========== */
void AppCommMgr_OnTxDone(uint16_t param, void *data_ptr)
{
    (void)param;
    (void)data_ptr;

    if (s_poll_state == COMM_S_WAIT_TX) {
        s_poll_state = COMM_S_WAIT_RX;
        s_heads[s_cur_head].timeout_cnt = 0u;
    }
}

/* ========== __weak 接收: 由 app_power 直调, 抽象功率→MODBUS帧 ========== */
void AppCommMgr_OnPowerCmd(uint16_t param, void *data_ptr)
{
    CommPowerCmd_t *cmd;
    uint8_t         slave_addr;
    uint16_t        power_val;
    uint16_t        frame_len;
    static CommSendReq_t s_tx_req;
    (void)param;

    if (data_ptr == NULL) return;
    cmd = (CommPowerCmd_t *)data_ptr;

    slave_addr = COMM_SLAVE_ADDR_BASE
                 + cmd->head_idx * COMM_SLAVE_ADDR_STEP;

    /* 瓦特→MODBUS寄存器值(协议知识,仅本模块知道) */
    if (cmd->power_watt > 5u) {
        power_val = cmd->power_watt / 25u;
        if (power_val < 8u) power_val = 8u;
    } else {
        power_val = 0u;
    }

    /* Frame 1: 功率设定 */
    frame_len = Proto_Modbus_BuildWriteSingle(slave_addr,
                    COMM_POWER_REG_BASE + COMM_POWER_REG_POWERSET,
                    power_val, s_tx_req.data);
    s_tx_req.len = frame_len;
    DrvCommMgr_OnSendReq((uint16_t)cmd->head_idx, &s_tx_req);

    /* Frame 2: 开关控制 */
    frame_len = Proto_Modbus_BuildWriteSingle(slave_addr,
                    COMM_POWER_REG_BASE + COMM_POWER_REG_SWITCH,
                    (power_val > 0u) ? 0x10u : 0x01u, s_tx_req.data);
    s_tx_req.len = frame_len;
    DrvCommMgr_OnSendReq((uint16_t)cmd->head_idx, &s_tx_req);
}

/* ========== 初始化 ========== */
void App_CommMgr_Init(void)
{
    uint8_t i;

    for (i = 0u; i < COMM_HEAD_COUNT; i++) {
        s_heads[i].state       = COMM_STATE_DISCONNECTED;
        s_heads[i].slave_addr  = COMM_SLAVE_ADDR_BASE
                                 + i * COMM_SLAVE_ADDR_STEP;
        s_heads[i].retry_cnt   = 0u;
        s_heads[i].timeout_cnt = 0u;
    }
    s_cur_head   = 0u;
    s_poll_state = COMM_S_IDLE;
    s_tick_10ms  = 0u;
}

/* ========== 每10ms槽位调用 ========== */
void App_CommMgr_Run(void)
{
    uint8_t   head_idx;
    HeadCtx_t *ctx;

    /* 超时检测 */
    if (s_poll_state == COMM_S_WAIT_RX) {
        head_idx = s_cur_head;
        ctx = &s_heads[head_idx];
        ctx->timeout_cnt++;
        if (ctx->timeout_cnt >= COMM_TIMEOUT_10MS) {
            ctx->retry_cnt++;
            if (ctx->retry_cnt >= COMM_RETRY_MAX) {
                ctx->state = COMM_STATE_DISCONNECTED;
                ctx->retry_cnt = 0u;
                s_cur_head = (s_cur_head + 1u) % COMM_HEAD_COUNT;
                s_poll_state = COMM_S_IDLE;
            } else {
                send_read_req(head_idx);
            }
        }
    }

    /* 100ms节拍: 发起下一轮轮询 */
    s_tick_10ms++;
    if (s_tick_10ms >= COMM_POLL_PERIOD_10MS) {
        s_tick_10ms = 0u;

        if (s_poll_state == COMM_S_IDLE) {
            send_read_req(s_cur_head);
        }
    }
}
