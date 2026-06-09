/********************************************************************************
    FileName    :  resonant_msg.h
    Brief       :  谐振分析 MESSAGE — 替代 printf 为 MODBUS 寄存器输出
                   接口完全兼容 printMessage.h
    Copyright (c) Foshan XinSun Electronic Technology CO.,Ltd
********************************************************************************/
#ifndef   RESONANT_MSG_H
#define   RESONANT_MSG_H

/********************************************Head Files*/
#include	<stdint.h>

#define     RESONANT_MSG_BUFF_SIZE      (8)        // 信息索引个数

typedef struct
{
    uint16_t size;
    uint16_t   res;
    uint16_t* buff;
} ResonantMsgBuffDef;


typedef struct          // ntc传感器极限参数
{
    ResonantMsgBuffDef  array[RESONANT_MSG_BUFF_SIZE];  // 第一数据地址首址
    ResonantMsgBuffDef 	paraArray;                      // 参数数据地址
    uint8_t num;            // 信息序号
    uint8_t res1;
    uint8_t res2;
    uint8_t res3;
} ResonantMsgDef;

enum
{
    RESONANT_PAN_MESSAGE=0,      // 检锅信息
    RESONANT_TXA_MESSAGE,        // 谐振电流FMAC
    RESONANT_CURRENT_MESSAGE,    // 谐振电流计算结果
};

uint8_t    ResonantMsg_Push(ResonantMsgDef messageIn);   // 将需要打印的数据压到堆里
uint8_t    ResonantMsg_Out(void);

#endif
//**********************************end of file********************************
