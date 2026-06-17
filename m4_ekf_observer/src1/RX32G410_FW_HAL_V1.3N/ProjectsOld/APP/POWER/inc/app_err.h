/********************************************************************************
    FileName    :  app_err.h
    Author      :  rsl
    Version     :  V1.0.1
    Brief       :  应用错误处理

    Date        :  2018-10-18
    Modify      :
                   2018-10-18 创建

    Copyright (c)    Foshan XinSun Electronic Technology CO.,Ltd
********************************************************************************/
#include "s_errcode.h"

/*
    系统一共设置了16个错误类型，其中按照规格书
    E0  ---  硬件故障               E4  --- 顶部传感器开路
    E1  ---  IGBT超温                 E5  --- 顶部传感器短路
    E2  ---  电源过压                  E6  --- 底部传感器开路
    E3  ---  电源欠压                  E7  --- 底部传感器短路
    E10  ---  无锅

    这些错误出现的情况下，都要中断加热，两个错误同时出现后，按0-16的自然顺序报错
*/
#define ERR_STOP_MASK   (BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT6|BIT7|BIT8|BIT9|BIT10|BIT11|BIT12|BIT13) //报错掩码

extern INT16U xdata ErrReport;
extern INT16U xdata BotErrorCnt;

INT8U Err_GetFirstStop(void);
void Err_WifiReport(void);
#define Err_ClrReport() ErrReport = 0; BotErrorCnt = 0;

//**********************************end of file********************************


