/**
 * drv_comm_mgr.h —— DRV 层通讯消息管理器接口
 *
 * 注册 MSG_COMM_SEND_REQ 处理器，桥接 APP 消息 → Drv_Comm_* 硬件调用。
 * Drv_CommMgr_Update() 每10ms轮询 TX/RX 状态并投递 MSG_COMM_TX_DONE /
 * MSG_COMM_DATA_UPDATE 回 APP。
 *
 * 依赖: drv_comm.h + msg_scheduler.h
 * 层级: DRV
 */
#ifndef DRV_COMM_MGR_H
//#define DRV_COMM_MGR_H

#include <stdint.h>

void Drv_CommMgr_Init(void);
void Drv_CommMgr_Update(void);  /* 每10ms槽位调用 */

#endif /* DRV_COMM_MGR_H */
