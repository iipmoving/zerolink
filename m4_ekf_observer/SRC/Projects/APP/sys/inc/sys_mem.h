/********************************************************************************
    FileName    :  sys_mem.h
    Author      :  rsl
    Version     :  V1.0.1
    Brief       :  内存处理函数，包括一些字符处理函数等

    Date        :  2018-09-06
    Modify      :
                   2018-09-06 创建

    Copyright (c)    Foshan XinSun Electronic Technology CO.,Ltd
********************************************************************************/

#ifndef SYS_MEM_H
#define SYS_MEM_H



/********************************************Head Files*/
#include "data_type.h"

void *Sys_MemSet(void *mem, INT8U dat, INT16U len);
void *Sys_MemSet2(void *mem, INT8U dat, INT8U len);
void *Sys_MemCpy(void *dest, void *src, INT8U len);
void *Sys_MemCpy2(void *dest, void *src, INT8U len);
INT8U Sys_MemIsEmpty(void *mem, INT8U len);
INT16U Sys_Strlen(void *str);
INT8U Sys_MemIsEqual(void *mem1, void *mem2, INT8U len);
INT8U *Sys_StrStrEx(INT8U *str1, INT8U *str2);

INT8U Sys_IntToAscii(INT32S num, INT8U *str);
//INT8U Sys_NumToAsciiEx(INT16U num, INT8U *str);
INT32U Sys_AsciiToNum(INT8U *str, INT8U len);
#endif

//**********************************end of file********************************

