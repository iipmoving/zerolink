/**
 * @file    example_a_io.h
 * @brief   Module A — Data Switcher IO interface (Producer)
 * @note    公开接口文件，保留 #define include guard
 *          仅 data_switcher.c 可通过全路径 #include 本文件
 *
 * 协议 (methodology-seed-v2.0/08-data-switcher.md):
 *   status bit0=已构造, bit1=新数据就绪
 *   DoWork 进入时先清 bit1, 有新产出时置位
 *   Switcher 检测 bit1 → 搬运字段 → 清 bit1
 */
#ifndef EXAMPLE_A_IO_H
#define EXAMPLE_A_IO_H

#include <stdint.h>

#pragma pack(4)

/* ====== 输出槽（本模块产出） ====== */
typedef struct {
    uint8_t  status;        /* bit0=已构造, bit1=新数据就绪（本模块设, Switcher 清）*/
    uint8_t  res[3];        /* 32位对齐填充 */
    uint16_t counter;       /* 递增计数器 — 产出数据 */
    uint8_t  mode;          /* 当前模式 */
    uint8_t  res2[1];       /* 对齐到 4 字节 */
} ExampleA_Output_t;        /* sizeof=12 */

#pragma pack()

/* ---- 公开接口 ---- */
void ExampleA_GetIO(void **ppIn, ExampleA_Output_t **ppOut);
void ExampleA_DoWork(void);

#endif /* EXAMPLE_A_IO_H */
