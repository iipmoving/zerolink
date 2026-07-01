// ===== [AI GENERATED] 范式接入+骨架, 可被PY替换 =====
#include "../include_io/app_comm_mgr_io.h"

static void Init(void);
MODULE_SKELETON(AppCommMgr);

/* 管道就绪标志: �?BIT 代表一个管道的 ST_NEW 状�?*/
typedef union {
    uint8_t all;
    struct {
        uint8_t protomodbus   : 1;  /* ProtoModbus 数据就绪 */
        uint8_t drvcommmgr   : 1;  /* DrvCommMgr 数据就绪 */
        uint8_t apppower     : 1;  /* AppPower 数据就绪 */
    } bits;
} AppCommMgr_PipeFlags_t;

/* ---- 数据实体（模块私有）---- */
static MODULE_INPUT(AppCommMgr)*   s_inPara;    // 输入参数实体在ADC�?这里只调用不修改
static MODULE_OUTPUT(AppCommMgr)  s_outPara;   // 输出参数缓冲�?

/* ---- 消费�?last_seq �?seq 有效性比�?(空闲SLOT幂等) ---- */
static uint8_t s_last_seq_ProtoModbus = 0xFF;  /* ProtoModbus→AppCommMgr */
static uint8_t s_last_seq_DrvCommMgr = 0xFF;  /* DrvCommMgr→AppCommMgr */
static uint8_t s_last_seq_AppPower = 0xFF;  /* AppPower→AppCommMgr */

/* ---- 内部 OUTPUT_LINK + PARAMS 实例 ---- */
static MODULE_OUTPUT_PARAMS(AppCommMgr, AppPower)  s_AppCommMgrToAppPowerParams;
static MODULE_OUTPUT_LINK(AppCommMgr, AppPower)  s_AppCommMgrToAppPowerLink;
static MODULE_OUTPUT_PARAMS(AppCommMgr, AppCooking)  s_AppCommMgrToAppCookingParams;
static MODULE_OUTPUT_LINK(AppCommMgr, AppCooking)  s_AppCommMgrToAppCookingLink;
static MODULE_OUTPUT_PARAMS(AppCommMgr, AppProtect)  s_AppCommMgrToAppProtectParams;
static MODULE_OUTPUT_LINK(AppCommMgr, AppProtect)  s_AppCommMgrToAppProtectLink;
static MODULE_OUTPUT_PARAMS(AppCommMgr, ProtoModbus)  s_AppCommMgrToProtoModbusParams;
static MODULE_OUTPUT_LINK(AppCommMgr, ProtoModbus)  s_AppCommMgrToProtoModbusLink;
static MODULE_OUTPUT_PARAMS(AppCommMgr, DrvCommMgr)  s_AppCommMgrToDrvCommMgrParams;
static MODULE_OUTPUT_LINK(AppCommMgr, DrvCommMgr)  s_AppCommMgrToDrvCommMgrLink;


/* 用户业务入口: flags.bits 指示哪些管道有新数据 (seq 比对通过)
 * 实际定义在用户代码区 (可引用用户变�?函数) */
static void user_Process(MODULE_INPUT(AppCommMgr) *in, MODULE_OUTPUT(AppCommMgr) *out, AppCommMgr_PipeFlags_t flags);

static void ProcessInput(void)
{
    MODULE_INPUT(AppCommMgr) *in  = (MODULE_INPUT(AppCommMgr)*)g_input.para;
    MODULE_OUTPUT(AppCommMgr) *out = (MODULE_OUTPUT(AppCommMgr)*)g_output.para;

    /* === 输入�? seq 有效性比�?=== */
    AppCommMgr_PipeFlags_t flags = {0};
    {
        uint8_t cur_seq = in->ProtoModbus_params->seq;
        if (cur_seq != s_last_seq_ProtoModbus) {
            flags.bits.protomodbus = 1;
            s_last_seq_ProtoModbus = cur_seq;
        }
    }
    {
        uint8_t cur_seq = in->DrvCommMgr_params->seq;
        if (cur_seq != s_last_seq_DrvCommMgr) {
            flags.bits.drvcommmgr = 1;
            s_last_seq_DrvCommMgr = cur_seq;
        }
    }
    {
        uint8_t cur_seq = in->AppPower_params->seq;
        if (cur_seq != s_last_seq_AppPower) {
            flags.bits.apppower = 1;
            s_last_seq_AppPower = cur_seq;
        }
    }

    /* === 计算�? 用户业务 === */
    user_Process(in, out, flags);
}

MODULE_EXPORT(AppCommMgr);

// ===== [END AI GENERATED] =====
#include "core/std_module.h"
#include "../include_io/app_comm_mgr_io.h"
#include "app_comm_mgr.h"
#include <string.h>
#include <stddef.h>

/* ---- 数据结构 ---- */
typedef struct {
    uint8_t  data_ready;   /* 收到新数�?*/
    uint8_t  tx_done;      /* 发送完�?*/
    uint8_t  power_cmd;    /* 功率命令 */
} InData_t;
/* 输出 �?寄存器数据广�?(Switcher 路由�?app_power/cooking/protect) */
typedef struct {
    uint8_t  has_reg;
    uint8_t  head_idx;
    uint8_t  slave_addr;
    uint8_t  online;
    uint16_t regs[22];
} OutData_t;

#if 0  /* DEDUP: MODULE_SKELETON */
MODULE_SKELETON(AppCommMgr);
#endif  /* DEDUP: MODULE_SKELETON */

/* ========== 协议层抽�?========== */
#define PROTO_PARSE_OK         0      /* 解析成功 (�?PROTO_PARSE_OK 对齐) */
#define PROTO_FUNC_READ        0x03u  /* 读寄存器 (协议知识, 仅本模块)       */

/* __weak 协议函数空壳 �?�?proto/ 模块提供强符号实�?*/
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

/* @V1_VOIDPTR: __weak 声明保留过渡, 已迁�?LINK route */

/* ========== MODBUS实时控制寄存�?唯一知道这些地址的模�? ========== */
#define COMM_REG_WORKSTATE       0x2014u /* 工作状�?bit0=总开�?bit1=风机 bit4=电压 */
#define COMM_REG_TARGETPOWER     0x2016u /* 目标功率 = W */

/* ========== 独立类型声明（与 DRV �?DrvComm* 布局一致）========== */
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

/* �?app_power.c �?PowerOutput_t 布局一�?*/
typedef struct {
    uint8_t  head_idx;
    uint16_t power_watt;
} CommPowerCmd_t;

/* ========== 全局轮询状�?========== */
#define COMM_S_IDLE          0u
#define COMM_S_WAIT_TX       1u
#define COMM_S_WAIT_RX       2u

/* ========== 炉头上下�?========== */
typedef struct {
    CommState_t state;              /* 连接状�?                           */
    uint8_t     slave_addr;         /* MODBUS 站号                         */
    uint16_t    regs[COMM_REG_COUNT]; /* 寄存器缓�?                        */
    uint8_t     retry_cnt;          /* 当前重试计数                        */
    uint8_t     timeout_cnt;        /* 响应超时计数 (10ms单位)             */
} HeadCtx_t;

static HeadCtx_t s_heads[COMM_HEAD_COUNT];
static uint8_t   s_cur_head;       /* 当前轮询炉头索引 0-3                 */
static uint8_t   s_poll_state;     /* 轮询子状�?                          */
static uint8_t   s_tick_10ms;      /* 10ms节拍计数                        */
/* ========== 内部: 发送读寄存器请�?========== */
static void send_read_req(uint8_t head_idx)
{
    HeadCtx_t *ctx;
    uint16_t   frame_len;
    MODULE_OUTPUT_PARAMS(AppCommMgr, DrvCommMgr) *tx = &s_AppCommMgrToDrvCommMgrParams;

    ctx = &s_heads[head_idx];
    frame_len = Proto_BuildRead(ctx->slave_addr,
                                       COMM_REG_STATUS,
                                       COMM_REG_COUNT,
                                       tx->tx_data);
    tx->tx_len  = frame_len;
    tx->head_idx = head_idx;
    s_AppCommMgrToDrvCommMgrLink.seq++;
    s_poll_state = COMM_S_WAIT_TX;
    ctx->timeout_cnt = 0u;
}

/* ========== 内部: 处理响应帧（MSG_COMM_DATA_UPDATE 回调�?========= */
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

        /* 广播寄存器数�? �?g_output �?Switcher 路由到三个接收方 */
        OutData_t *o = (OutData_t *)g_output.para;
        o->has_reg = 1;
        o->head_idx  = head_idx;
        o->slave_addr = slave;
        o->online    = 1;
        for (i = 0; i < COMM_REG_COUNT; i++) o->regs[i] = ctx->regs[i];
    }
}

/* ========== V1 回调: 已迁�?user_Process route, 保留声明�?drv_comm_mgr TODO 过渡 ========== */
#if 0  /* DEDUP: OnDataUpdate �?已迁�?user_Process route=2 */
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
#endif

#if 0  /* DEDUP: OnTxDone �?已迁�?user_Process route=1 */
void AppCommMgr_OnTxDone(uint16_t param, void *data_ptr)
{
    (void)param;
    (void)data_ptr;

    if (s_poll_state == COMM_S_WAIT_TX) {
        s_poll_state = COMM_S_WAIT_RX;
        s_heads[s_cur_head].timeout_cnt = 0u;
    }
}
#endif

#if 0  /* DEDUP: OnPowerCmd �?V1 孤立回调, 无调用�? 功率命令�?AppPower→DrvCommMgr LINK */
/* ========== __weak 接收: �?app_power 直调, 抽象功率→MODBUS�?========== */
void AppCommMgr_OnPowerCmd(uint16_t param, void *data_ptr)
{
    CommPowerCmd_t *cmd;
    uint8_t         slave_addr;
    uint16_t        power_val;
    uint16_t        frame_len;
    MODULE_OUTPUT_PARAMS(AppCommMgr, DrvCommMgr) *tx = &s_AppCommMgrToDrvCommMgrParams;
    (void)param;

    if (data_ptr == NULL) return;
    cmd = (CommPowerCmd_t *)data_ptr;

    slave_addr = COMM_SLAVE_ADDR_BASE
                 + cmd->head_idx * COMM_SLAVE_ADDR_STEP;

    if (cmd->power_watt > 5u) {
        power_val = cmd->power_watt / 25u;
        if (power_val < 8u) power_val = 8u;
    } else {
        power_val = 0u;
    }

    tx->head_idx = cmd->head_idx;

    /* Frame 1: 目标功率 �?0x2016 */
    frame_len = Proto_BuildWriteSingle(slave_addr,
                    COMM_REG_TARGETPOWER, power_val, tx->tx_data);
    tx->tx_len = frame_len;
    s_AppCommMgrToDrvCommMgrLink.seq++;

    /* Frame 2: 工作状�?�?0x2014 */
    frame_len = Proto_BuildWriteSingle(slave_addr,
                    COMM_REG_WORKSTATE,
                    (power_val > 0u) ? 0x13u : 0x00u, tx->tx_data);
    tx->tx_len = frame_len;
    s_AppCommMgrToDrvCommMgrLink.seq++;
}
#endif

/* ========== 初始�?========== */
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
    g_input.para  = &s_inPara;
    g_output.para = &s_outPara;
    memset(&s_outPara, 0, sizeof(s_outPara));
    s_AppCommMgrToAppPowerLink.params     = &s_AppCommMgrToAppPowerParams;
    s_outPara.AppPower_params             = &s_AppCommMgrToAppPowerLink;
    s_AppCommMgrToAppCookingLink.params   = &s_AppCommMgrToAppCookingParams;
    s_outPara.AppCooking_params           = &s_AppCommMgrToAppCookingLink;
    s_AppCommMgrToAppProtectLink.params   = &s_AppCommMgrToAppProtectParams;
    s_outPara.AppProtect_params           = &s_AppCommMgrToAppProtectLink;
    s_AppCommMgrToProtoModbusLink.params  = &s_AppCommMgrToProtoModbusParams;
    s_outPara.ProtoModbus_params          = &s_AppCommMgrToProtoModbusLink;
    s_AppCommMgrToDrvCommMgrLink.params   = &s_AppCommMgrToDrvCommMgrParams;
    s_outPara.DrvCommMgr_params           = &s_AppCommMgrToDrvCommMgrLink;
}

#if 0  /* DEDUP: ProcessInput */
static void ProcessInput(void)
{
    /* 当前逻辑�?App_CommMgr_Run 中处�?*/
}
#endif  /* DEDUP: ProcessInput */

/* ========== route 分发: 替代 V1 OnTxDone/OnDataUpdate 回调 ========== */
static void user_Process(MODULE_INPUT(AppCommMgr) *in, MODULE_OUTPUT(AppCommMgr) *out, AppCommMgr_PipeFlags_t flags)
{
    if (flags.bits.drvcommmgr) {
        MODULE_INPUT_PARAMS(DrvCommMgr, AppCommMgr) *p = in->DrvCommMgr_params->params;
        switch (p->event) {
        case 1u:
            if (s_poll_state == COMM_S_WAIT_TX) {
                s_poll_state = COMM_S_WAIT_RX;
                s_heads[s_cur_head].timeout_cnt = 0u;
            }
            break;
        case 2u:
            if (p->rx_len > 0u) {
                handle_response(p->rx_data, p->rx_len);
                s_poll_state = COMM_S_IDLE;
                s_cur_head = (s_cur_head + 1u) % COMM_HEAD_COUNT;
            }
            break;
        default:
            break;
        }
    }
    if (flags.bits.apppower) {
        MODULE_INPUT_PARAMS(AppPower, AppCommMgr) *p = in->AppPower_params->params;
        MODULE_OUTPUT_PARAMS(AppCommMgr, DrvCommMgr) *tx = &s_AppCommMgrToDrvCommMgrParams;
        uint8_t  slave_addr;
        uint16_t power_val;
        uint16_t frame_len;

        slave_addr = COMM_SLAVE_ADDR_BASE
                     + p->head_idx * COMM_SLAVE_ADDR_STEP;

        if (p->target_power > 5u) {
            power_val = p->target_power / 25u;
            if (power_val < 8u) power_val = 8u;
        } else {
            power_val = 0u;
        }

        tx->head_idx = p->head_idx;

        /* Frame 1: 目标功率 �?0x2016 */
        frame_len = Proto_BuildWriteSingle(slave_addr,
                        COMM_REG_TARGETPOWER, power_val, tx->tx_data);
        tx->tx_len = frame_len;
        s_AppCommMgrToDrvCommMgrLink.seq++;

        /* Frame 2: 工作状�?�?0x2014 */
        frame_len = Proto_BuildWriteSingle(slave_addr,
                        COMM_REG_WORKSTATE,
                        (power_val > 0u) ? 0x13u : 0x00u, tx->tx_data);
        tx->tx_len = frame_len;
        s_AppCommMgrToDrvCommMgrLink.seq++;
    }
    (void)out;
}

/* ========== �?0ms槽位调用 ========== */
void App_CommMgr_Run(void)
{
    uint8_t   head_idx;
    HeadCtx_t *ctx;

    /* 超时检�?*/
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

    /* 100ms节拍: 发起下一轮轮�?*/
    s_tick_10ms++;
    if (s_tick_10ms >= COMM_POLL_PERIOD_10MS) {
        s_tick_10ms = 0u;

        if (s_poll_state == COMM_S_IDLE) {
            send_read_req(s_cur_head);
        }
    }
}

/* v2.0 桥接 */
#if 0  /* DEDUP: Init wrapper */
void App_CommMgr_Init(void) { Constructor(); }
#endif  /* DEDUP: Init wrapper */

/* ---- 导出 ---- */
#if 0  /* DEDUP: MODULE_EXPORT */
MODULE_EXPORT(AppCommMgr);
#endif  /* DEDUP: MODULE_EXPORT */
