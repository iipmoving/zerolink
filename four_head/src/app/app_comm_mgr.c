/**
 * app_comm_mgr.c —— 4炉头通信轮询调度器 + 通讯数据集中缓存
 *
 * 依赖: app_comm_mgr.h (独立声明), <string.h>, <stddef.h>
 * 层级: APP —— 应用层通信管理
 *
 * 通讯路径 (协议抽象 = 不直接 include proto/):
 *   APP 发: __weak Proto_BuildRead/BuildWriteSingle → PROTO 强符号实现
 *   APP 收: PROTO 解包后 → __weak AppCommMgr_OnDataUpdate → 解析缓存
 *   数据广播: 寄存器缓存写入后 → __weak AppPower/Cooking/Protect_OnRegData
 *
 * 设计原则:
 *   1. PROTO 与 DRV 同层隔离 — APP 不直接 include proto/
 *   2. 通讯数据集中缓存 (s_heads[].regs) — 其他模块不存通讯状态
 *   3. 协议可替换 — 换协议只需换 proto 模块的强符号实现
 */
#include "core/std_module.h"
#include "app_comm_mgr.h"
#include <string.h>
#include <stddef.h>

/* ---- 数据结构 ---- */
typedef struct {
    uint8_t  data_ready;   /* 收到新数据 */
    uint8_t  tx_done;      /* 发送完成 */
    uint8_t  power_cmd;    /* 功率命令 */
} InData_t;
/* 输出 — 寄存器数据广播 (Switcher 路由到 app_power/cooking/protect) */
typedef struct {
    uint8_t  has_reg;
    uint8_t  head_idx;
    uint8_t  slave_addr;
    uint8_t  online;
    uint16_t regs[22];
} OutData_t;

static InData_t  s_in;
static OutData_t s_out;

MODULE_SKELETON();

/* ========== 协议层抽象 ========== */
#define PROTO_PARSE_OK         0      /* 解析成功 (与 PROTO_PARSE_OK 对齐) */
#define PROTO_FUNC_READ        0x03u  /* 读寄存器 (协议知识, 仅本模块)       */

/* __weak 协议函数空壳 — 由 proto/ 模块提供强符号实现 */
__weak uint16_t Proto_BuildRead(uint8_t slave, uint16_t reg,
                                uint16_t count, uint8_t *buf)
{ (void)slave; (void)reg; (void)count; (void)buf; return 0u; }

__weak int8_t Proto_Parse(const uint8_t *rx, uint16_t len,
                          uint8_t *slave, uint8_t *func,
                          uint16_t *data, uint16_t *count)
{ (void)rx; (void)len; (void)slave; (void)func; (void)data; (void)count; return -1; }

__weak uint16_t Proto_BuildWriteSingle(uint8_t slave, uint16_t reg,
                                       uint16_t val, uint8_t *buf)
{ (void)slave; (void)reg; (void)val; (void)buf; return 0u; }

/* __weak: 仅 DRV 层方向保留，APP→APP 走 Switcher 路由 */
__weak void DrvCommMgr_OnSendReq(uint16_t param, void *data_ptr)
{ (void)param; (void)data_ptr; }

/* ========== MODBUS实时控制寄存器(唯一知道这些地址的模块) ========== */
#define COMM_REG_WORKSTATE       0x2014u /* 工作状态 bit0=总开关 bit1=风机 bit4=电压 */
#define COMM_REG_TARGETPOWER     0x2016u /* 目标功率 = W */

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
    frame_len = Proto_BuildRead(ctx->slave_addr,
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

    if (frame_len == 0u) return;

    result = Proto_Parse(rx_data, frame_len,
                                &slave, &func, data, &count);
    if (result != PROTO_PARSE_OK) return;

    /* 查找对应炉头 */
    for (head_idx = 0u; head_idx < COMM_HEAD_COUNT; head_idx++) {
        if (s_heads[head_idx].slave_addr == slave) break;
    }
    if (head_idx >= COMM_HEAD_COUNT) return;

    ctx = &s_heads[head_idx];

    if (func == PROTO_FUNC_READ && count > 0u) {
        uint16_t i;
        for (i = 0u; i < count && i < COMM_REG_COUNT; i++) {
            ctx->regs[i] = data[i];
        }
        ctx->state  = COMM_STATE_POLLING;
        ctx->retry_cnt = 0u;

        /* 广播寄存器数据: 写 g_output → Switcher 路由到三个接收方 */
        OutData_t *o = (OutData_t *)g_output.para;
        o->has_reg = 1;
        o->head_idx  = head_idx;
        o->slave_addr = slave;
        o->online    = 1;
        for (i = 0; i < COMM_REG_COUNT; i++) o->regs[i] = ctx->regs[i];
        g_output.info.status |= ST_OUT;
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

    /* Frame 1: 目标功率 → 0x2016 */
    frame_len = Proto_BuildWriteSingle(slave_addr,
                    COMM_REG_TARGETPOWER, power_val, s_tx_req.data);
    s_tx_req.len = frame_len;
    DrvCommMgr_OnSendReq((uint16_t)cmd->head_idx, &s_tx_req);

    /* Frame 2: 工作状态 → 0x2014
     *   正常加热: bit0=1(总开关) | bit1=1(风机) | bit4=1(220V) = 0x13
     *   关机: 全写0 = 0x00 */
    frame_len = Proto_BuildWriteSingle(slave_addr,
                    COMM_REG_WORKSTATE,
                    (power_val > 0u) ? 0x13u : 0x00u, s_tx_req.data);
    s_tx_req.len = frame_len;
    DrvCommMgr_OnSendReq((uint16_t)cmd->head_idx, &s_tx_req);
}

/* ========== 初始化 ========== */
static void Init(void)
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
    g_input.para  = &s_in;
    g_output.para = &s_out;
}

static void ProcessInput(void)
{
    /* 当前逻辑在 App_CommMgr_Run 中处理 */
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

/* v2.0 桥接 */
void App_CommMgr_Init(void) { Constructor(); }

/* ---- 导出 ---- */
MODULE_EXPORT(AppCommMgr);
