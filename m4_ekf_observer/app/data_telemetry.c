/**
 * @file    data_telemetry.c
 * @brief   Telemetry — 数据流复制实现
 * @note    内嵌缓冲区，零指针跟随，MODBUS 直接映射
 */

#include "data_telemetry.h"
#include <string.h>

static TelemetryData_t s_data;

void Telemetry_Init(void)
{
    memset(&s_data, 0, sizeof(s_data));
}

TelemetryData_t* Telemetry_GetData(void)
{
    return &s_data;
}
