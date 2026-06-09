/**
 * @file    example_b_io.h
 * @brief   Module B — Data Switcher IO interface (Consumer)
 * @note    公开接口文件，保留 #define include guard
 *          仅 data_switcher.c 可通过全路径 #include 本文件
 */
#ifndef EXAMPLE_B_IO_H
#define EXAMPLE_B_IO_H

#include <stdint.h>

#pragma pack(4)

/* ====== 输入槽（本模块消费） ====== */
typedef struct {
    uint8_t  status;        /* bit0=已构造, bit1=新数据到达（Switcher 设, 本模块清）*/
    uint8_t  res[3];        /* 32位对齐填充 */
    uint16_t counter;       /* 从 A 接收的计数器值 */
    uint8_t  mode;          /* 从 A 接收的模式值 */
    uint8_t  res2[1];       /* 对齐到 4 字节 */
} ExampleB_Input_t;         /* sizeof=12 — 与 ExampleA_Output_t 布局一致 */

#pragma pack()

/* ---- 公开接口 ---- */
void ExampleB_GetIO(ExampleB_Input_t **ppIn, void **ppOut);
void ExampleB_DoWork(void);

#endif /* EXAMPLE_B_IO_H */
