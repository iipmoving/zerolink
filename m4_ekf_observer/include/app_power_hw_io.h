/**
 * @file    app_power_hw_io.h
 * @brief   APP_Power ↔ DrvHrtimConsumer 数据流接口
 * @layer   inter-layer (Data Switcher IO)
 *
 * PowerHw_Command_t: APP → DRV 抽象功率控制命令
 *   - 增量式 (ppg_delta), 非绝对值
 *   - 不包含任何 HRTIM 具体寄存器信息
 *   - 桥模式无关 (半桥/全桥共用同一结构体)
 *
 * PowerHw_Status_t: DRV → APP 硬件状态反馈
 *   - DRV 从 HW 读取或影子变量维护
 *   - 每 10ms 随 data_switcher 路由回 APP
 *
 * DRV 层 Route 文件 include 本文件 + "API_HRTIM.h"
 * APP 层功能包 include 本文件 (只读 status, 只写 command)
 */
#ifndef APP_POWER_HW_IO_H
#define APP_POWER_HW_IO_H

#include <stdint.h>

/* ---- 最大炉头数 (与桥模式无关, 编译时固定) ---- */
#define HW_POTMAX   4

/* ===================================================================
 * PowerHw_Command_t — APP → DRV
 *
 * APP 的输出: "我想让硬件做什么"
 * DRV 的输入: "APP 要我做什么"
 * =================================================================== */
typedef struct {
    /* === 增量命令 (DRV 内部累加为绝对值) === */
    int16_t  ppg_delta[HW_POTMAX];        /* PID 输出的增减量                */
    uint8_t  delta_valid[HW_POTMAX];      /* 增量有效标志                    */

    /* === 开关/状态命令 === */
    uint8_t  power_on[HW_POTMAX];         /* 1=加热, 0=停止                 */

    /* === 检锅请求 === */
    uint8_t  pan_request_ch;              /* 请求检锅通道 (0xFF=无请求)      */
    uint8_t  pan_request;                 /* 检锅请求触发                    */

    /* === 同步请求 === */
    uint8_t  sync_request;                /* 请求频率同步                    */
    uint16_t target_freq;                 /* 目标频率 (可选, 0=不指定)       */

    /* === 填充 & 扩展 === */
    uint8_t  __reserved[14];

} PowerHw_Command_t;                      /* sizeof: 4+4+4+2+2+1+2+14 = 33 → 对齐到 36 */

/* ===================================================================
 * PowerHw_Status_t — DRV → APP
 *
 * DRV 的输出 (反馈): "硬件当前状态是什么"
 * APP 的输入: "当前 duty 多少? 检锅结果? 有无 BK?"
 * =================================================================== */
typedef struct {
    /* === 当前输出状态 (DRV 影子变量) === */
    uint16_t current_duty[HW_POTMAX];     /* 当前占空比 (给 APP 限幅参考)    */
    uint16_t current_period[HW_POTMAX];   /* 当前周期                        */
    uint8_t  current_on_off[HW_POTMAX];   /* 当前开关状态                    */

    /* === 检锅结果 === */
    uint8_t  pan_result[HW_POTMAX];       /* 0=无锅, 1=有锅, 0xFF=检测中    */
    uint8_t  pan_pulse_count;             /* 检锅脉冲数 (调试用)            */
    uint8_t  pan_fault;                   /* 检锅故障标志                    */

    /* === 保护状态 (DRV 从 HW 读取) === */
    uint8_t  bk_flag[HW_POTMAX];          /* BK 标志 (Burst Kill)            */
    uint8_t  fault[HW_POTMAX];            /* 硬件故障码                      */
    uint8_t  surge_flag;                  /* 浪涌标志                        */

    /* === 数据有效 === */
    uint8_t  valid;                       /* 1=本帧数据有效                  */

    /* === 填充 & 扩展 === */
    uint8_t  __reserved[9];

} PowerHw_Status_t;                       /* sizeof: 8+8+4+4+1+1+1+4+4+1+1+1+1+9 = 48 → 对齐到 48 */

#endif /* APP_POWER_HW_IO_H */
