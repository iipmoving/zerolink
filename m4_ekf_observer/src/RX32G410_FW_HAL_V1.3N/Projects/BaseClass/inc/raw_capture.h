/********************************************************************************
    FileName    :  raw_capture.h
    Brief       :  RawCapture — 原始寄存器级数据采集 → MODBUS 0x5100 区
                   帧格式与 WaveCapture 一致 (自描述):
                     [size][帧数据][参数]
                   每帧 = 1ms 计算单元, 帧内 sample_idx 从 0 起。

                   帧数据: [i_adc][v_adc][cnt] × N (每采样点 3 words)
                   参数:   [cmp_uon][cmp_uoff][cmp_lon][cmp_loff][power] (5 words)

                   PC 端生成: t_us(序号×采样间隔) + Vdc_adc(v_adc 均值)

    Date        :  2026-06-01
    Copyright (c) Foshan XinSun Electronic Technology CO.,Ltd
********************************************************************************/
#ifndef RAW_CAPTURE_H
#define RAW_CAPTURE_H

#include <stdint.h>
#include "printMessage.h"    /* MessageDef */

/* ========== 常量 ===================================================== */

#define RAW_HEADER_WORDS        6           /* 帧头 6 个寄存器 */
#define RAW_MAX_DATA_WORDS      4000        /* 数据体 ~8KB */
#define RAW_DEFAULT_FRAMES      10          /* 默认采集帧数 (=10ms) */
#define RAW_WORDS_PER_SAMPLE    3           /* i_adc + v_adc + cnt */
#define RAW_PARA_WORDS          5           /* cmp_uon + cmp_uoff + cmp_lon + cmp_loff + power */

/* MODBUS 0x5100 寄存器偏移 */
#define RAW_REG_ACK             0x5100u
#define RAW_REG_STATUS          0x5101u
#define RAW_REG_FRAME_ID        0x5102u
#define RAW_REG_COUNT           0x5103u
#define RAW_REG_DATA_WORDS      0x5104u
#define RAW_REG_MAX_FRAMES      0x5105u
#define RAW_REG_DATA            0x5106u

/* STATUS bit */
#define RAW_STATUS_READY        0x01u
#define RAW_STATUS_COLLECTING   0x02u

/* ========== MODBUS 帧结构 (0x5100 区完整映射) ======================== */

typedef struct {
    /* ---- 帧头 (6 words, 0x5100-0x5105) ---- */
    volatile uint16_t ack;          /* 0x5100 W: host写任意值=确认读完 */
    volatile uint16_t status;       /* 0x5101 R: bit0=READY, bit1=COLLECTING */
    volatile uint16_t frame_id;     /* 0x5102 R: 批次自增ID */
    volatile uint16_t count;        /* 0x5103 R: 本批次帧数 */
    volatile uint16_t data_words;   /* 0x5104 R: data[] 总占用字数 (含所有帧的size前缀) */
    volatile uint16_t max_frames;   /* 0x5105 R/W: 目标帧数 (默认10) */

    /* ---- 数据体 (0x5106+) ---- */
    /* 自描述格式: [size0][帧0数据+参数][size1][帧1数据+参数]...
       每帧: [i_adc×N][v_adc×N][cnt×N][cmp_uon][cmp_uoff][cmp_lon][cmp_loff]
       size = N×3 + 4 */
    volatile uint16_t data[RAW_MAX_DATA_WORDS];
} RawCaptureFrame;

/* ========== API ===================================================== */

void     RawCapture_Init(void);
void*    RawCapture_GetFramePtr(void);
uint8_t  RawCapture_IsReady(void);
void     RawCapture_MarkRead(void);

/**
 * @brief 推入一帧原始数据 (与 WaveCapture_PushMessage 调用方式一致)
 * @param msg  MessageDef 指针:
 *             array[0] = i_adc (谐振电流 ADC), size = 采样点数 N
 *             array[1] = v_adc (母线电压 ADC), size = N
 *             array[2] = cnt   (HRTIM CNT),     size = N
 *             paraArray       = [cmp_uon, cmp_uoff, cmp_lon, cmp_loff, power], size = 5
 * @return 1=成功, 0=拒绝
 * @note   满 max_frames 帧后自动冻结, status bit0=1
 */
uint8_t  RawCapture_PushMessage(MessageDef* msg);

/**
 * @brief ACK 写回调 — 供 MODBUS Check_Write_Data 注册
 *        检测到 host 写 ack!=0 且 status=READY 时自动解冻
 * @return 1=允许写入
 */
uint8_t  RawCapture_OnAckWrite(void);

#endif /* RAW_CAPTURE_H */
