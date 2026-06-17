/**
 * @file    telemetry.c
 * @brief   数据流遥测 — 捕获 Calculator 输入副本 + ElecParams 输出
 * @layer   base_class
 *
 * 输入: ElecParams (通过 ElecParams → Telemetry 输出管道)
 * 输出: 无 (MODBUS Area 5 映射内部缓冲区)
 */
#include "../../include_io/telemetry_io.h"
#include <string.h>

/* ==========================================================================
 * 内部缓冲区 — MODBUS 直接映射 (390 uint16 = 780 bytes)
 * ========================================================================== */
#define TELE_CALC_PERIODS  20
#define TELE_CALC_WORDS    18   /* 每周期 18 uint16 (含 uint32 split) */
#define TELE_ELEC_WORDS    26   /* 12 float(24 uint16) + valid(2 uint16) */
#define TELE_TOTAL_WORDS   (4 + TELE_CALC_PERIODS * TELE_CALC_WORDS + TELE_ELEC_WORDS)

/* ---- 控制寄存器 ---- */
typedef struct {
    uint16_t ctrl_start;      /* 写1=开始监测, 写0=停止 */
    uint16_t reserved;
    uint16_t status;          /* bit0=running, bit1=calc_ready, bit2=elec_ready */
    uint16_t reserved2;
} TelemCtrl_t;

/* ---- 单周期 Calculator 输入副本 (18 uint16) ---- */
typedef struct {
    uint16_t hrtim_highOff;
    uint16_t hrtim_lowOff;
    uint16_t hrtim_highOn;
    uint16_t hrtim_lowOn;
    uint16_t peak_current;
    uint16_t act_curr_high_lo;
    uint16_t act_curr_high_hi;
    uint16_t act_curr_low_lo;
    uint16_t act_curr_low_hi;
    uint16_t volt_sum_lo;
    uint16_t volt_sum_hi;
    uint16_t voltage_count;
    uint16_t zero_cross_high;
    uint16_t zero_cross_low;
    uint16_t peak_point;
    uint16_t res[3];          /* 补齐到 18 */
} TelemCalcPeriod_t;

/* ---- 完整遥测数据 ---- */
typedef struct {
    TelemCtrl_t             ctrl;
    TelemCalcPeriod_t       calc[TELE_CALC_PERIODS];
    float                   elec[12];  /* 12 float = 24 uint16 */
    uint16_t                elec_valid;
} TelemetryData_t;

/* ==========================================================================
 * 数据实体
 * ========================================================================== */
static MODULE_INPUT(Telemetry)  s_in;
static MODULE_OUTPUT(Telemetry) s_out;
static TelemetryData_t          s_telem_data;

/* ==========================================================================
 * 骨架 + API
 * ========================================================================== */
MODULE_SKELETON(Telemetry);

/* ---- 初始化 ---- */
static void Init(void)
{
    memset(&s_in,  0, sizeof(s_in));
    memset(&s_out, 0, sizeof(s_out));
    memset(&s_telem_data, 0, sizeof(s_telem_data));
    g_input.para  = &s_in;
    g_output.para = &s_out;
}

/* ---- ProcessInput ---- */
static void ProcessInput(void)
{
    MODULE_INPUT(Telemetry) *in = (MODULE_INPUT(Telemetry) *)g_input.para;
    TelemetryData_t *td = &s_telem_data;

    /* 检查输入指针 */
    if (!in->ElecParams_params) return;

    /* ====== 输入段: 检查数据 LINK status ====== */
    if (!(in->ElecParams_params->status & ST_NEW)) return;
    in->ElecParams_params->status &= ~ST_NEW;

    td->ctrl.status |= 0x01;  /* running */

    /* ---- 复制 Calculator 输入副本 (20周期) ---- */
    for (uint8_t h = 0; h < TELEMETRY_POTMAX; h++) {
        const MODULE_OUTPUT_PARAMS(ElecParams, Telemetry) *tp = &in->ElecParams_params->params[h];
        if (tp->calc_copy) {
            /* 只复制 head 0 的数据 */
            for (uint8_t c = 0; c < TELE_CALC_PERIODS; c++) {
                TelemCalcPeriod_t *dst = &td->calc[c];
                const MODULE_INPUT_PARAMS(Calculator, ElecParams) *src = &tp->calc_copy[c];

                dst->hrtim_highOff   = src->hrtim_highOff;
                dst->hrtim_lowOff    = src->hrtim_lowOff;
                dst->hrtim_highOn    = src->hrtim_highOn;
                dst->hrtim_lowOn     = src->hrtim_lowOn;
                dst->peak_current    = src->peak_current;
                dst->act_curr_high_lo = (uint16_t)src->active_current_sum_high;
                dst->act_curr_high_hi = (uint16_t)(src->active_current_sum_high >> 16);
                dst->act_curr_low_lo  = (uint16_t)src->active_current_sum_low;
                dst->act_curr_low_hi  = (uint16_t)(src->active_current_sum_low >> 16);
                dst->volt_sum_lo      = (uint16_t)src->voltage_sum;
                dst->volt_sum_hi      = (uint16_t)(src->voltage_sum >> 16);
                dst->voltage_count    = src->voltage_count;
                dst->zero_cross_high  = src->zero_cross_high;
                dst->zero_cross_low   = src->zero_cross_low;
                dst->peak_point       = src->peak_point;
            }
            td->ctrl.status |= 0x02;  /* calc_ready */
        }
    }

    /* ---- 复制 ElecParams 输出 (head 0) ---- */
    for (uint8_t h = 0; h < TELEMETRY_POTMAX; h++) {
        const MODULE_OUTPUT_PARAMS(ElecParams, Telemetry) *tp = &in->ElecParams_params->params[h];
        if (tp->elec_out) {
            /* 只复制 head 0 的数据 */
            if (h == 0) {
                memcpy(td->elec, tp->elec_out, 12 * sizeof(float));
                td->elec_valid = (uint16_t)tp->elec_out->valid;
                td->ctrl.status |= 0x04;  /* elec_ready */
            }
        }
    }
}

/* ---- 导出 ---- */
MODULE_EXPORT(Telemetry);

/* ---- MODBUS 缓冲区访问器 ---- */
void* Telemetry_GetBuffer(void)
{
    return (void*)&s_telem_data;
}

int Telemetry_GetBufferSize(void)
{
    return (int)(TELE_TOTAL_WORDS * sizeof(uint16_t));
}

/* ---- InputCallback: 从 ElecParams 拉取 Telemetry 输出管道 ---- */
INPUT_CALLBACK(ElecParams, Telemetry)
{
    INPUT_GET_SLOT(ElecParams, Telemetry);
}
