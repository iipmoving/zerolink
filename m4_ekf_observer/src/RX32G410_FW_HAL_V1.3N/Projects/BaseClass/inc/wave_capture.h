/********************************************************************************
    FileName    :  wave_capture.h
    Brief       :  WaveCapture — 多帧拼接, 通用数据采集缓冲
                   累积 N 帧后冻结, 等外部读完再解冻

                   MessageDef 的 array/size/num 由调用方配置,
                   本模块不关心里面存的是什么。

    Date        :  2026-06-01
    Copyright (c) Foshan XinSun Electronic Technology CO.,Ltd
********************************************************************************/
#ifndef WAVE_CAPTURE_H
#define WAVE_CAPTURE_H

#include <stdint.h>

/* ========== 独立消息结构 (与 printMessage.h 布局一致, 零 include) ======== */
#define  CAPTURE_MSG_BUFF_SIZE   (8)

/* 数据源名称枚举 — 供 CSV 表头使用 */
typedef enum {
    CAPTURE_SRC_CNT    = 0,   /* HRTIM CNT 快照 */
    CAPTURE_SRC_V_AD   = 1,   /* 谐振电压 ADC */
    CAPTURE_SRC_I_AD   = 2,   /* 谐振电流 ADC */
    CAPTURE_SRC_VDC_AD = 3,   /* 直流母线 ADC */
    CAPTURE_SRC_CMP1   = 10,  /* 比较值 1 */
    CAPTURE_SRC_CMP2   = 11,  /* 比较值 2 */
    CAPTURE_SRC_CMP3   = 12,  /* 比较值 3 */
    CAPTURE_SRC_CMP4   = 13,  /* 比较值 4 */
    CAPTURE_SRC_POWER  = 20,  /* 功率 */
} CaptureSrcId;

typedef struct {
    uint16_t size;
    uint16_t name_id;   /* CaptureSrcId */
    uint16_t* buff;
} CaptureMsgBuff_t;

typedef struct {
    CaptureMsgBuff_t  array[CAPTURE_MSG_BUFF_SIZE];
    CaptureMsgBuff_t  paraArray;
    uint8_t  num;
    uint8_t  res1;
    uint8_t  res2;
    uint8_t  res3;
} CaptureMsgDef_t;

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
       每帧前 1 word 是该帧的字数 */
    volatile uint16_t data[WAVE_MAX_DATA_WORDS];
} WaveCaptureFrame;

/* ========== API ===================================================== */

void     WaveCapture_Init(void);
void*    WaveCapture_GetFramePtr(void);
uint8_t  WaveCapture_IsReady(void);
void     WaveCapture_MarkRead(void);

/**
 * @brief 推入一帧数据
 * @param msg  CaptureMsgDef_t 指针, array[i].buff→数据源, array[i].size→长度
 * @return 1=成功, 0=拒绝 (帧满未读/数据溢出)
 * @note   满 max_frames 帧后自动冻结, status bit0=1
 */
uint8_t  WaveCapture_PushMessage(void* msg_in);

/* 测试数据生成 — 填充 10 帧锯齿波 */
void     WaveCapture_GenerateTestData(void);

#endif /* WAVE_CAPTURE_H */
