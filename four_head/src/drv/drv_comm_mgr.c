// ===== [AI GENERATED] 范式接入+骨架, 可被PY替换 =====
#include "../include_io/drv_comm_mgr_io.h"

static void Init(void);
MODULE_SKELETON(DrvCommMgr);

/* 管道就绪标志: 每 BIT 代表一个管道的 ST_NEW 状态 */
typedef union {
    uint8_t all;
    struct {
        uint8_t appcommmgr   : 1;  /* AppCommMgr 数据就绪 */
    } bits;
} DrvCommMgr_PipeFlags_t;

/* ---- 数据实体（模块私有）---- */
static MODULE_INPUT(DrvCommMgr)*   s_inPara;    // 输入参数实体在ADC， 这里只调用不修改
static MODULE_OUTPUT(DrvCommMgr)  s_outPara;   // 输出参数缓冲区

/* ---- 消费者 last_seq — seq 有效性比对 (空闲SLOT幂等) ---- */
static uint8_t s_last_seq_AppCommMgr = 0xFF;  /* AppCommMgr→DrvCommMgr */

/* ---- 内部 OUTPUT_LINK + PARAMS 实例 ---- */
static MODULE_OUTPUT_PARAMS(DrvCommMgr, AppCommMgr)  s_DrvCommMgrToAppCommMgrParams;
static MODULE_OUTPUT_LINK(DrvCommMgr, AppCommMgr)  s_DrvCommMgrToAppCommMgrLink;


/* 用户业务入口: flags.bits 指示哪些管道有新数据 (seq 比对通过)
 * 实际定义在用户代码区 (可引用用户变量/函数) */
static void user_Process(MODULE_INPUT(DrvCommMgr) *in, MODULE_OUTPUT(DrvCommMgr) *out, DrvCommMgr_PipeFlags_t flags);

static void ProcessInput(void)
{
    MODULE_INPUT(DrvCommMgr) *in  = (MODULE_INPUT(DrvCommMgr)*)g_input.para;
    MODULE_OUTPUT(DrvCommMgr) *out = (MODULE_OUTPUT(DrvCommMgr)*)g_output.para;

    /* === 输入段: seq 有效性比对 === */
    DrvCommMgr_PipeFlags_t flags = {0};
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

MODULE_EXPORT(DrvCommMgr);

// ===== [END AI GENERATED] =====
#include "core/std_module.h"
#include "../include_io/drv_comm_mgr_io.h"
#include "drv_comm_mgr.h"
#include "drv_comm.h"
#include <string.h>
#include <stddef.h>

static void Init(void)
{
    memset(&s_outPara, 0, sizeof(s_outPara));
    s_DrvCommMgrToAppCommMgrLink.params = &s_DrvCommMgrToAppCommMgrParams;
    s_outPara.AppCommMgr_params         = &s_DrvCommMgrToAppCommMgrLink;
    g_input.para  = &s_inPara;
    g_output.para = &s_outPara;
}

/* APP 层强符号声明 — 已迁移 LINK route, 保留 #if 0 过渡 */
#if 0  /* DEDUP: AppCommMgr_OnTxDone/OnDataUpdate — 已迁移 LINK route */
void AppCommMgr_OnTxDone(uint16_t param, void *data_ptr);
void AppCommMgr_OnDataUpdate(uint16_t param, void *data_ptr);
#endif

/* ---- 独立声明: 与 APP 层 CommSendReq_t 布局一致 ---- */
#define DRV_COMM_SEND_BUF_SIZE  64u
#define DRV_COMM_RX_BUF_SIZE    256u

typedef struct {
    uint8_t  data[DRV_COMM_SEND_BUF_SIZE];
    uint16_t len;
} DrvCommSendReq_t;

typedef struct {
    uint8_t  data[DRV_COMM_RX_BUF_SIZE];
    uint16_t len;
} DrvCommDataUpdate_t;

/* ---- 静态状态 ---- */
static DrvCommSendReq_t    s_pending_req;
static uint8_t             s_pending_valid;
static DrvCommDataUpdate_t s_rx_data;
static uint8_t             s_tx_done_flag;

/* ---- __weak 接收: 已迁移 LINK route, 保留 #if 0 过渡 ---- */
#if 0  /* DEDUP: DrvCommMgr_OnSendReq — 已迁移 LINK route */
void DrvCommMgr_OnSendReq(uint16_t param, void *data_ptr)
{
    DrvCommSendReq_t *req;
    (void)param;

    req = (DrvCommSendReq_t *)data_ptr;
    if (req == NULL || req->len == 0u) return;

    if (Drv_Comm_TxDone() == 0u) {
        /* TX 忙，暂存待发送 (last-write-wins) */
        s_pending_req.len = (req->len <= DRV_COMM_SEND_BUF_SIZE)
                          ? req->len : DRV_COMM_SEND_BUF_SIZE;
        memcpy(s_pending_req.data, req->data, s_pending_req.len);
        s_pending_valid = 1u;
        return;
    }

    Drv_Comm_Send(req->data, req->len);
}
#endif

/* ---- 每10ms槽位调用 ---- */
void Drv_CommMgr_Update(void)
{
    uint16_t frame_len;

    /* 检查 TX 完成 */
    if (Drv_Comm_TxDone() != 0u) {
        if (s_tx_done_flag == 0u) {
            s_tx_done_flag = 1u;
            s_DrvCommMgrToAppCommMgrParams.event = 1u;
            s_DrvCommMgrToAppCommMgrLink.seq++;
        }
        /* 有待发送帧则立即发送 */
        if (s_pending_valid) {
            Drv_Comm_Send(s_pending_req.data, s_pending_req.len);
            s_pending_valid = 0u;
            s_tx_done_flag  = 0u;
        }
    } else {
        s_tx_done_flag = 0u;
    }

    /* 检查 RX: 完整帧就绪 */
    frame_len = Drv_Comm_RecvPoll();
    if (frame_len > 0u) {
        s_rx_data.len = Drv_Comm_Read(s_rx_data.data,
                                       DRV_COMM_RX_BUF_SIZE);
        Drv_Comm_Flush();
        if (s_rx_data.len > 0u) {
            s_DrvCommMgrToAppCommMgrParams.event = 2u;
            memcpy(s_DrvCommMgrToAppCommMgrParams.rx_data, s_rx_data.data, s_rx_data.len);
            s_DrvCommMgrToAppCommMgrParams.rx_len = s_rx_data.len;
            s_DrvCommMgrToAppCommMgrLink.seq++;
        }
    }
}

/* ---- user_Process: 从 AppCommMgr LINK 读取发送请求 ---- */
static void user_Process(MODULE_INPUT(DrvCommMgr) *in, MODULE_OUTPUT(DrvCommMgr) *out, DrvCommMgr_PipeFlags_t flags)
{
    if (flags.bits.appcommmgr) {
        MODULE_INPUT_PARAMS(AppCommMgr, DrvCommMgr) *p = in->AppCommMgr_params->params;
        if (p != NULL && p->tx_len > 0u) {
            if (Drv_Comm_TxDone() != 0u) {
                Drv_Comm_Send(p->tx_data, p->tx_len);
            } else {
                s_pending_req.len = (p->tx_len <= DRV_COMM_SEND_BUF_SIZE)
                                  ? p->tx_len : DRV_COMM_SEND_BUF_SIZE;
                memcpy(s_pending_req.data, p->tx_data, s_pending_req.len);
                s_pending_valid = 1u;
            }
        }
    }
    (void)out;
}
