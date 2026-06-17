/********************************************************************************
    FileName    :  sys_mem.c
    Author      :  rsl
    Version     :  V1.0.1
    Brief       :  内存处理函数，包括一些字符处理函数等

    Date        :  2018-09-06
    Modify      :
                   2018-09-06 创建

    Copyright (c)    Foshan XinSun Electronic Technology CO.,Ltd
********************************************************************************/

/********************************************Head Files*/
#include "sys_mem.h"

/*********************************************常量声明*/

/*********************************************局部变量申请*/


/*********************************************函数列表*/

/********************************************************************************
*name         : void *Sys_MemSet(void *mem, INT8U dat, INT8U len)
*author       : rsl
*function     : 内存设置函数
*para         : mem,要设置的内存首地址，dat，设置成的数据，len，要设置的长度
*return       : 设置内存的首地址
*brief        :
********************************************************************************/
void *Sys_MemSet(void *mem, INT8U dat, INT16U len)
{
    INT8U  *ret = (INT8U*)mem;

    while(len--)
    {
        *ret++ = dat;
    }

    return mem;
}

/********************************************************************************
*name         : void *Sys_MemCpy(void *dest, void *src, INT8U len)
*author       : rsl
*function     : 内存拷贝函数
*para         : dest为目标内存区，src为源内存区，len为需要拷贝的字节数
*return       : 指向dest的指针
*brief        : 未考虑空间重叠情况,调用时注意
********************************************************************************/
void *Sys_MemCpy(void *dest, void *src, INT8U len)
{
    INT8U  *ret = (INT8U*)dest;
    INT8U  *dest_t = ret;
    INT8U  *src_t = (INT8U*)src;

    while(len--)
    {
        *dest_t++ = *src_t++;
    }

    return ret;
}

//void *Sys_MemCpy2(void *dest, void *src, INT8U len)
//{
//    INT8U  *ret = (INT8U*)dest;
//    INT8U  *dest_t = ret;
//    INT8U  *src_t = (INT8U*)src;

//    while(len--)
//    {
//        *dest_t++ = *src_t++;
//    }

//    return ret;
//}

/********************************************************************************
*name         : INT8U Sys_MemIsEmpty(void *mem, INT8U len)
*author       : rsl
*function     : 判断一片内存区域是否为0
*para         : mem为源内存区，len为内存区长度
*return       : TRUE为空，FALSE不为空
*brief        :
********************************************************************************/
#if 0
INT8U Sys_MemIsEmpty(void *mem, INT8U len)
{
    INT8U  *ptr = (INT8U*)mem;

    while(len--)
    {
        if(*(ptr++) != 0)
        {
            return FALSE;
        }
    }

    return TRUE;
}
/********************************************************************************
*name         : INT8U Sys_MemIsEqual(void *mem1, void *mem2, INT8U len)
*author       : rsl
*function     : 比较内存是否相等
*para         : mem1,2两个内存地址, len比较长度
*return       : FALSE不等，TRUE相等
*brief        : len不能超过内存长度，否则会出错
********************************************************************************/
INT8U Sys_MemIsEqual(void *mem1, void *mem2, INT8U len)
{
    INT8U *ptr1 = (INT8U *)mem1;
    INT8U *ptr2 = (INT8U *)mem2;

    while(len--)
    {
        if(*ptr1++ != *ptr2++)
        {
            return FALSE;
        }
    }

    return TRUE;
}
/********************************************************************************
*name         : INT16U Sys_Strlen(void *str)
*author       : rsl
*function     : 字符串长度获取
*para         : 字符串首地址
*return       : 长度
*brief        : 输入的字符串必须有'\0'结尾，否则长度会出错
********************************************************************************/
INT16U Sys_Strlen(void *str)
{
    INT8U  *ptr = (INT8U*)str;
    INT16U len = 0;

    while(*ptr != 0x00)
    {
        len++;
        ptr++;
    }

    return len;
}

/********************************************************************************
*name         : INT8U *Sys_StrStrEx(INT8U *str1, INT8U *str2)
*author       : rsl
*function     : 找出str2在str1中是否出现过
*para         : str1,str2首地址
*return       : 0未出现，>0str2地址
*brief        : str2支持统配字符，*代表数字，#代表字母，&数字或字母，统配不支持字符中出现符号
********************************************************************************/
INT8U *Sys_StrStrEx(INT8U *str1, INT8U *str2)
{
    INT8U n;

    if(*str2)
    {
        while(*str1)
        {
            for(n = 0; ((*(str1+n) == *(str2+n)) || \
                        ((*(str2+n) == '*')&&((*(str1+n)>= '0')&&(*(str1+n) <= '9')))||\
                        ((*(str2+n) == '#')&&(((*(str1+n)>= 'A')&&(*(str1+n) <= 'Z')) || ((*(str1+n)>= 'a')&&(*(str1+n) <= 'z'))))||\
                        (*(str2+n) == '&')\
                       ); n++ )
            {
                if(!*(str2+n+1))
                {
                    return (INT8U *)str1;
                    return n;
                }
            }
            str1++;
        }
        return 0;
    }

    return 0;
}


/********************************************************************************
*name         : void Sys_NumToAscii(INT16U num, INT8U *str)
*author       : rsl
*function     : 数字转ascii
*para         : num 16bit数字， str,ascii存放地址
*return       : 数据位数
*brief        : 10进制
********************************************************************************/
/* 索引表 */
//code INT8U IntToAsciiTabl[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
INT8U Sys_IntToAscii(INT32S num, INT8U *str)
{

    INT32U unum; /* 中间变量 */
    INT8U i=0,j,k;
    INT8U temp;

    /* 确定unum的值 */
    if(/*(radix == 10) && */(num < 0)) /* 十进制负数 */
    {
        unum = 0 - num;
        str[i++]='-';
    }
    else
    {
        unum = num; /* 其它情况 */
    }
    /* 逆序 */
    do
    {
        str[i++] = unum % 10 + '0'; //IntToAsciiTabl[unum % radix];
        unum /= 10; //radix;
    }
    while(unum);

    str[i]='\0';        //最后一位补〇

    /* 转换 */
    if(str[0]=='-')
    {
        k=1; /* 十进制负数 */
    }
    else
    {
        k=0;
    }

    for(j=k; j<=(i-k-1)/2; j++)
    {
        temp = str[j];
        str[j] = str[i-j-1];
        str[i-j-1] = temp;
    }

    return i;
}
#endif
#if 0
/********************************************************************************
*name         : INT8U Sys_NumToAsciiEx(INT16U num, INT8U *str)
*author       : rsl
*function     : 数字转ascii, 并把最高位的0去掉
*para         : num 16bit数字， str,ascii存放地址
*return       : 数据长度
*brief        : 暂时只支持4位数据输出
********************************************************************************/
INT8U Sys_NumToAsciiEx(INT16U num, INT8U *str)
{
    INT8U  len = 0;
    INT8U i;
    INT8U ptr[4];

    if(num == 0)
    {
        str[0] = '0';
        return 1;
    }


    while(num > 0)
    {
        ptr[len++] = num%10+'0';
        num /= 10;
    }

    for(i=0; i<len; i++)
    {
        str[i] = ptr[len-i-1];
    }

    return len;
}

/********************************************************************************
*name         : INT16U Sys_AsciiToNum(INT8U *str, INT8U len)
*author       : rsl
*function     : ascii 转数字 最大接收4位数据，转32bit数字
*para         : str,ascii存放地址  len 长度
*return       : 数字
*brief        :
********************************************************************************/
INT32U Sys_AsciiToNum(INT8U *str, INT8U len)
{
    INT32U tmp = 0;

    if(len > 6)
    {
        return 0;
    }

    while(len > 0)
    {
        tmp *= 10;
        tmp += *str - '0';
        len--;
        str++;
    }

    return tmp;
}
#endif
//**********************************end of file********************************
