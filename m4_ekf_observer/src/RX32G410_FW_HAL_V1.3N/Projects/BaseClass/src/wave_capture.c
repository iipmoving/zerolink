/********************************************************************************
    FileName    :  wave_capture.c
    Brief       :  WaveCapture — 多帧拼接 → MODBUS 0x5000 区
                   基于 printMessage.c 的 Push 逻辑, 一次拷贝全部数组
                   累积 N 帧后冻结, 等 MODBUS 主机写 ACK 后解冻

                   数据体自描述格式:
                     [size0][帧0数据][size1][帧1数据]...
                     每帧前 1 word = 该帧总字数 (array + paraArray)

    Date        :  2026-06-01
    Copyright (c) Foshan XinSun Electronic Technology CO.,Ltd
********************************************************************************/

#include "wave_capture.h"
#include <string.h>

/* ========== 帧缓冲区 (MODBUS 0x5000 直接映射) ======================== */
static WaveCaptureFrame s_frame;

/* ========== 内部状态 ================================================= */
static uint16_t  s_frame_count;       /* 本批已收帧数 */
static uint16_t  s_frame_id;          /* 批次ID (自增) */
static uint16_t  s_write_offset;      /* 当前写位置 (data[] 内偏移) */
static uint8_t   s_lock;              /* 防重入锁 */

/* ========== 初始化 =================================================== */
void WaveCapture_Init(void)
{
    memset((void*)&s_frame, 0, sizeof(s_frame));
    s_frame.max_frames = WAVE_DEFAULT_FRAMES;

    s_frame_count  = 0;
    s_frame_id     = 0;
    s_write_offset = 0;
    s_lock         = 0;
}

/* ========== 获取帧指针 (MODBUS Area 注册用) ========================== */
void* WaveCapture_GetFramePtr(void)
{
    return (void*)&s_frame;
}

/* ========== 检查帧就绪 =============================================== */
uint8_t WaveCapture_IsReady(void)
{
    return (s_frame.status & WAVE_STATUS_READY) ? 1 : 0;
}

/* ========== 确认读完 → 解冻 ========================================== */
void WaveCapture_MarkRead(void)
{
    s_frame.status      = 0;
    s_frame.count       = 0;
    s_frame.data_words  = 0;
    s_frame.ack         = 0;
    s_frame.frame_id    = ++s_frame_id;

    s_frame_count  = 0;
    s_write_offset = 0;
}

/* ========== 推入一帧 (与 PrintMessagePush 调用方式完全一致) =========== */
uint8_t WaveCapture_PushMessage(MessageDef* msg)
{
    uint16_t i, frame_words, capacity;
    uint16_t* dest;

    /* ---- 参数校验 ---- */
    if (msg == 0) return 0;

    /* 帧就绪中 → 拒绝, 等 MarkRead */
    if (s_frame.status & WAVE_STATUS_READY) return 0;

    /* 防重入 */
    if (s_lock) return 0;
    s_lock = 1;

    /* ---- 计算本帧总字数: array[0..n] + paraArray ---- */
    frame_words = 0;
    for (i = 0; i < PRINT_MESSAGE_BUFF_SIZE; i++) {
        if (msg->array[i].size > 0 && msg->array[i].buff != 0) {
            frame_words += msg->array[i].size;
        }
    }
    if (msg->paraArray.size > 0 && msg->paraArray.buff != 0) {
        frame_words += msg->paraArray.size;
    }

    if (frame_words == 0) {
        s_lock = 0;
        return 0;
    }

    /* ---- 检查容量: 本帧 = 1(size前缀) + frame_words ---- */
    capacity = 1 + frame_words;
    if (s_write_offset + capacity > WAVE_MAX_DATA_WORDS) {
        s_lock = 0;
        return 0;
    }

    /* ---- 首帧: 设置 COLLECTING 状态 ---- */
    if (s_frame_count == 0) {
        s_frame.status |= WAVE_STATUS_COLLECTING;
    }

    /* ---- 写入 size 前缀 ---- */
    s_frame.data[s_write_offset] = frame_words;
    s_write_offset++;

    /* ---- 拷贝 array[0..n] 全部数据 ---- */
    dest = (uint16_t*)&s_frame.data[s_write_offset];
    for (i = 0; i < PRINT_MESSAGE_BUFF_SIZE; i++) {
        uint16_t sz = msg->array[i].size;
        if (sz > 0 && msg->array[i].buff != 0) {
            uint16_t j;
            for (j = 0; j < sz; j++) {
                *dest++ = msg->array[i].buff[j];
            }
        }
    }

    /* ---- 拷贝 paraArray ---- */
    if (msg->paraArray.size > 0 && msg->paraArray.buff != 0) {
        uint16_t j;
        for (j = 0; j < msg->paraArray.size; j++) {
            *dest++ = msg->paraArray.buff[j];
        }
    }

    /* ---- 推进写指针 ---- */
    s_write_offset += frame_words;
    s_frame_count++;

    /* ---- 帧满 → 冻结 ---- */
    if (s_frame_count >= s_frame.max_frames) {
        s_frame.count      = s_frame_count;
        s_frame.data_words = s_write_offset;
        s_frame.frame_id   = s_frame_id;
        s_frame.status     = WAVE_STATUS_READY;
    }

    s_lock = 0;
    return 1;
}

/* ========== ACK 写回调 (MODBUS Check_Write_Data) ====================== */
unsigned char WaveCapture_OnAckWrite(void)
{
    if (s_frame.ack && (s_frame.status & WAVE_STATUS_READY)) {
        WaveCapture_MarkRead();
    }
    return 1;
}
