/********************************************************************************
    FileName    :  app_protect.h
    Author      :  rsl
    Version     :  V1.0.1
    Brief       :  IH保护功能，包括高低压，igbt高温，底部传感器高温，以及传感器开短路

    Date        :  2018-10-19
    Modify      :
                   2018-10-19 创建

    Copyright (c)    Foshan XinSun Electronic Technology CO.,Ltd
********************************************************************************/
#ifndef APP_PROTECT_H
#define APP_PROTECT_H

/********************************************Head Files*/
#include "data_type.h"


void Protect_Service(void);
void GetTopTemp(void);
#endif




