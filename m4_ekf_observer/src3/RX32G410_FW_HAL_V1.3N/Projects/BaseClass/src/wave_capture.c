/********************************************************************************
    FileName    :  wave_capture.c
    Brief       :  WaveCapture — 多帧拼接, 通用数据采集缓冲
                   基于 printMessage.c 的 Push 逻辑, 一次拷贝全部数组
                   累积 N 帧后冻结, 等外部读完再解冻

                   数据体自描述格式:
                     [size0][帧0数据][size1][帧1数据]...
                     每帧前 1 word = 该帧总字数 (array + paraArray)

    Date        :  2026-06-01
    Copyright (c) Foshan XinSun Electronic Technology CO.,Ltd
********************************************************************************/

#include "wave_capture.h"
#include <string.h>

/* ========== 帧缓冲区 =================================================== */
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
//		WaveCapture_GenerateTestData();
}

/* ========== __weak 强覆盖: 提供帧缓冲区 ================================= */
void* Modbus_ProvideCaptureBuffer(void)
{
    return (void*)&s_frame;
}

/* ========== __weak 强覆盖: ACK 回调 ==================================== */
void Modbus_OnCaptureAck(uint16_t val)
{
    if (val && (s_frame.status & WAVE_STATUS_READY)) {
        WaveCapture_MarkRead();
    }
}

/* ========== 获取帧缓冲区指针 (供外部 MODBUS 映射) ====================== */
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

/* ========== ACK 写回调 (供外部 MODBUS Check_Write_Data) ================ */
unsigned char WaveCapture_OnAckWrite(void)
{
    if (s_frame.ack && (s_frame.status & WAVE_STATUS_READY)) {
        WaveCapture_MarkRead();
    }
    return 1;
}

/* ========== 测试数据生成 =============================================== */
#define TEST_SAMPLES  50     /* 每帧采样点数 (1ms) */

void WaveCapture_GenerateTestData(void)
{
    uint16_t test_cnt [TEST_SAMPLES];  /* HRTIM CNT 快照 */
    uint16_t test_vad [TEST_SAMPLES];  /* 谐振电压 ADC */
    uint16_t test_iad [TEST_SAMPLES];  /* 谐振电流 ADC */
    uint16_t test_para[5];             /* CMP1-4, POWER */
    uint8_t f, j;

    if (s_frame.status & WAVE_STATUS_READY) {
        WaveCapture_MarkRead();
    }

    for (f = 0; f < WAVE_DEFAULT_FRAMES; f++) {
        CaptureMsgDef_t msg;
        memset(&msg, 0, sizeof(msg));

        for (j = 0; j < TEST_SAMPLES; j++) {
            test_cnt[j] = (uint16_t)(f * 1000 + j);
            test_vad[j] = (uint16_t)(2000 + (j % 25) * 5);
            test_iad[j] = (uint16_t)(j < 25 ? j * 40 : (49 - j) * 40);
        }
        msg.array[0].buff = test_cnt;
        msg.array[0].size = TEST_SAMPLES;
        msg.array[1].buff = test_vad;
        msg.array[1].size = TEST_SAMPLES;
        msg.array[2].buff = test_iad;
        msg.array[2].size = TEST_SAMPLES;

        test_para[0] = (uint16_t)(1000 + f * 10);
        test_para[1] = (uint16_t)(2000 + f * 10);
        test_para[2] = (uint16_t)(3000 + f * 10);
        test_para[3] = (uint16_t)(4000 + f * 10);
        test_para[4] = (uint16_t)(500 + f * 100);
        msg.paraArray.buff = test_para;
        msg.paraArray.size = 5;

        WaveCapture_PushMessage(&msg);
    }
}

/* ========== 推入一帧 (与 PrintMessagePush 调用方式完全一致) =========== */
uint8_t WaveCapture_PushMessage(void* msg_in)
{
    uint16_t i, frame_words, capacity;
    uint16_t* dest;
		CaptureMsgDef_t* msg=(CaptureMsgDef_t* )msg_in;
    /* ---- 参数校验 ---- */
    if (msg == 0) return 0;

    /* 帧就绪中 → 拒绝, 等 MarkRead */
    if (s_frame.status & WAVE_STATUS_READY) return 0;

    /* 防重入 */
    if (s_lock) return 0;
    s_lock = 1;

    /* ---- 计算本帧总字数: array[0..n] + paraArray ---- */
    frame_words = 0;
    for (i = 0; i < CAPTURE_MSG_BUFF_SIZE; i++) {
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
    for (i = 0; i < CAPTURE_MSG_BUFF_SIZE; i++) {
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
