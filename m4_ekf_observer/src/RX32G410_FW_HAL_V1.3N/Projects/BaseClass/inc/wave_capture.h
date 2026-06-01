/********************************************************************************
    FileName    :  wave_capture.h
    Brief       :  WaveCapture — 多帧拼接 → MODBUS FC03 回读
                   基于 printMessage 的 Push 逻辑 (一次拷贝全部数组)
                   累积 N 帧后冻结, 等 MODBUS 主机读完再解冻

                   MessageDef 的 array/size/num 由调用方配置,
                   本模块不关心里面存的是什么。

    Date        :  2026-06-01
    Copyright (c) Foshan XinSun Electronic Technology CO.,Ltd
********************************************************************************/
#ifndef WAVE_CAPTURE_H
#define WAVE_CAPTURE_H

#include <stdint.h>
#include "printMessage.h"    /* MessageDef */

/* ========== 常量 ===================================================== */

#define WAVE_HEADER_WORDS       6           /* 帧头 6 个寄存器 */
#define WAVE_MAX_DATA_WORDS     4000        /* 数据体 ~8KB */
#define WAVE_DEFAULT_FRAMES     10          /* 默认采集帧数 */

/* MODBUS 0x5000 寄存器偏移 */
#define WAVE_REG_ACK            0x5000u
#define WAVE_REG_STATUS         0x5001u
#define WAVE_REG_FRAME_ID       0x5002u
#define WAVE_REG_COUNT          0x5003u
#define WAVE_REG_DATA_WORDS     0x5004u
#define WAVE_REG_MAX_FRAMES     0x5005u
#define WAVE_REG_DATA           0x5006u

/* STATUS bit */
#define WAVE_STATUS_READY       0x01u
#define WAVE_STATUS_COLLECTING  0x02u

/* ========== MODBUS 帧结构 (0x5000 区完整映射) ======================== */

typedef struct {
    /* ---- 帧头 (6 words, 0x5000-0x5005) ---- */
    volatile uint16_t ack;          /* 0x5000 W: host写任意值=确认读完 */
    volatile uint16_t status;       /* 0x5001 R: bit0=READY, bit1=COLLECTING */
    volatile uint16_t frame_id;     /* 0x5002 R: 批次自增ID */
    volatile uint16_t count;        /* 0x5003 R: 本批次实际帧数 */
    volatile uint16_t data_words;   /* 0x5004 R: data[] 总占用字数 (含所有帧的size前缀) */
    volatile uint16_t max_frames;   /* 0x5005 R/W: 目标帧数 (默认10, host可写) */

    /* ---- 数据体 (0x5006+) ---- */
    /* 自描述格式: [size0][帧0数据][size1][帧1数据]...
       每帧前 1 word 是该帧的字数, MATLAB 依次读 size → 读数据 → 重复 COUNT 次 */
    volatile uint16_t data[WAVE_MAX_DATA_WORDS];
} WaveCaptureFrame;

/* ========== API ===================================================== */

void     WaveCapture_Init(void);
void*    WaveCapture_GetFramePtr(void);
uint8_t  WaveCapture_IsReady(void);
void     WaveCapture_MarkRead(void);

/**
 * @brief 推入一帧数据 (与 PrintMessagePush 签名一致)
 * @param msg  MessageDef 指针, array[i].buff→数据源, array[i].size→长度
 * @return 1=成功, 0=拒绝 (帧满未读/数据溢出)
 * @note   每帧独立, 大小不必相同
 *         满 max_frames 帧后自动冻结, status bit0=1
 */
uint8_t  WaveCapture_PushMessage(MessageDef* msg);

/**
 * @brief ACK 写回调 — 供 MODBUS Check_Write_Data 注册
 *        检测到 host 写 ack!=0 且 status=READY 时自动解冻
 * @return 1=允许写入
 */
uint8_t  WaveCapture_OnAckWrite(void);

#endif /* WAVE_CAPTURE_H */
