/**
 * drv_comm_mgr.c —— DRV 层通讯消息管理器实现
 *
 * 桥接 APP 层消息与硬件通讯:
 *   MSG_COMM_SEND_REQ  → Drv_Comm_Send()
 *   Drv_Comm_RecvPoll() → MSG_COMM_DATA_UPDATE
 *   Drv_Comm_TxDone()   → MSG_COMM_TX_DONE
 *
 * 独立类型声明 — 与 APP 层的 CommSendReq_t / CommDataUpdate_t 布局一致。
 * 映射关系见 core/interface_map.h。
 *
 * 依赖: drv_comm_mgr.h + drv_comm.h + msg_scheduler.h + msg_def.h
 * 层级: DRV
 */
#include "drv_comm_mgr.h"
#include "drv_comm.h"
#include <string.h>
#include <stddef.h>

/* __weak 回调: 发送给 APP 层, 链接器自动接线, interface_map.h 文档化 */
__weak void AppCommMgr_OnTxDone(uint16_t param, void *data_ptr)
{ (void)param; (void)data_ptr; }
__weak void AppCommMgr_OnDataUpdate(uint16_t param, void *data_ptr)
{ (void)param; (void)data_ptr; }

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

/* ---- __weak 接收: 由 app_comm_mgr 直调 ---- */
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

/* ---- 初始化 ---- */
void Drv_CommMgr_Init(void)
{
    s_pending_valid = 0u;
    s_tx_done_flag  = 0u;
    memset(&s_pending_req, 0, sizeof(s_pending_req));
    memset(&s_rx_data,     0, sizeof(s_rx_data));

    Drv_Comm_Init();
}

/* ---- 每10ms槽位调用 ---- */
void Drv_CommMgr_Update(void)
{
    uint16_t frame_len;

    /* 检查 TX 完成 */
    if (Drv_Comm_TxDone() != 0u) {
        if (s_tx_done_flag == 0u) {
            s_tx_done_flag = 1u;
            AppCommMgr_OnTxDone(0u, NULL);
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
            AppCommMgr_OnDataUpdate(0u, &s_rx_data);
        }
    }
}
