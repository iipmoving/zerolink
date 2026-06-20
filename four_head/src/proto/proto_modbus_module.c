/**
 * proto_modbus_module.c —— MODBUS 协议模块包装层
 *
 * 纯协议库 proto_modbus.c 的 Switcher 模块壳
 * 2tick 请求-响应: AppCommMgr请求 → 本模块DoWork → AppCommMgr响应
 */
#include "core/std_module.h"
#include "../include_io/proto_modbus_io.h"
#include "../proto/proto_modbus.h"
#include <stddef.h>

MODULE_SKELETON(ProtoModbus);

static MODULE_INPUT(ProtoModbus)  *s_inPara;
static MODULE_OUTPUT(ProtoModbus)  s_outPara;

static MODULE_OUTPUT_LINK(ProtoModbus, AppCommMgr)  s_out_link;
static MODULE_OUTPUT_PARAMS(ProtoModbus, AppCommMgr) s_out_params;

static void Init(void)
{
    memset(&s_outPara, 0, sizeof(s_outPara));
    memset(&s_out_link, 0, sizeof(s_out_link));
    memset(&s_out_params, 0, sizeof(s_out_params));
    s_out_link.params = &s_out_params;
    s_outPara.AppCommMgr_params = &s_out_link;
    g_input.para  = &s_inPara;
    g_output.para = &s_outPara;
}
MODULE_EXPORT(ProtoModbus);

static void ProcessInput(void)
{
    MODULE_INPUT(ProtoModbus) *in = (MODULE_INPUT(ProtoModbus)*)g_input.para;
    MODULE_OUTPUT(ProtoModbus) *out = &s_outPara;
    MODULE_INPUT_LINK(AppCommMgr, ProtoModbus) *req;
    MODULE_OUTPUT_PARAMS(ProtoModbus, AppCommMgr) *rsp;

    if (!in || !in->AppCommMgr_params) return;
    req = in->AppCommMgr_params;
    if (!req || !req->params) return;

    rsp = out->AppCommMgr_params->params;
    if (!rsp) return;

    switch (req->params->cmd) {
    case 1u:
        rsp->tx_len = Proto_Modbus_BuildRead(
            req->params->slave, req->params->read_reg,
            req->params->read_count, rsp->tx_data);
        rsp->result  = 0;
        rsp->slave   = req->params->slave;
        rsp->func    = MODBUS_FUNC_READ;
        break;
    case 2u:
        rsp->tx_len = Proto_Modbus_BuildWriteSingle(
            req->params->slave, req->params->write_reg,
            req->params->write_val, rsp->tx_data);
        rsp->result  = 0;
        rsp->slave   = req->params->slave;
        rsp->func    = MODBUS_FUNC_WRITE_SINGLE;
        break;
    case 3u:
        rsp->result = Proto_Modbus_Parse(
            req->params->rx_data, req->params->rx_len,
            &rsp->slave, &rsp->func,
            rsp->data, &rsp->data_count);
        break;
    default:
        break;
    }

    out->AppCommMgr_params->status |= ST_NEW;
    out->AppCommMgr_params->seq++;
    g_output.info.status |= ST_OUT;
}
