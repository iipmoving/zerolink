/**
 * @file    telemetry.c
 * @brief   数据流遥测 — 捕获 Calculator 输入副本 + ElecParams 输出
 * @layer   base_class
 *
 * 输入:
 *   Calculator (通过 Calculator → Telemetry 管道转发)
 *   ElecParams (通过 ElecParams → Telemetry 管道转发)
 * 输出: 无 (MODBUS Area 5 映射内部缓冲区)
 */
#include "../../include_io/telemetry_io.h"
#include <string.h>

/* ==========================================================================
 * 内部常量
 * ========================================================================== */
#define TELE_CALC_PERIODS 20
#define TELE_POTCH         0    /* 选择监听的炉头 (0-3) */
#define TELE_CALC_WORDS    15   /* 每周期 30 字节 = 15 uint16 (pack(4)) */
#define TELE_ELEC_WORDS    26   /* 12 float(24 uint16) + valid(2 uint16) */
#define TELE_TOTAL_WORDS   (4 + TELE_CALC_PERIODS * TELE_CALC_WORDS + TELE_ELEC_WORDS)

/* ---- 控制寄存器 ---- */
typedef struct {
    uint16_t ctrl_start;      /* 写1=开始监测, 写0=停止 */
    uint16_t reserved;
    uint16_t status;          /* bit0=running, bit1=calc_ready, bit2=elec_ready */
    uint16_t reserved2;
} TelemCtrl_t;

/* ---- 完整遥测数据 ---- */
typedef struct {
    TelemCtrl_t                          ctrl;
    MODULE_INPUT_PARAMS(Calculator, Telemetry) calc[TELE_CALC_PERIODS];
    MODULE_INPUT_PARAMS(ElecParams, Telemetry) elec;  /* 26 words: 12 float + valid + res[3] */
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

    /* ---- 复制 Calculator 输入副本 (20周期, 仅炉头 TELE_POTCH) ---- */
    if (in->Calculator_params && (in->Calculator_params->status & ST_NEW)) {
        for (uint8_t c = 0; c < TELE_CALC_PERIODS; c++) {
            td->calc[c] = in->Calculator_params->params[TELE_POTCH][c];
        }
        td->ctrl.status |= 0x02;  /* calc_ready */
    }

    /* ---- 复制 ElecParams 输出 (仅炉头 TELE_POTCH) ---- */
    if (in->ElecParams_params && (in->ElecParams_params->status & ST_NEW)) {
        in->ElecParams_params->status &= ~ST_NEW;
        memcpy(&td->elec, &in->ElecParams_params->params[TELE_POTCH], sizeof(td->elec));
        td->ctrl.status |= 0x04;  /* elec_ready */
    }

    td->ctrl.status |= 0x01;  /* running */
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

