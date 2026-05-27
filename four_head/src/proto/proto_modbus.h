/**
 * proto_modbus.h —— MODBUS RTU 协议编解码接口
 *
 * 依赖: <stdint.h>（无其他模块依赖）
 * 层级: PROTO —— 纯协议层，不 include msg_scheduler.h
 *
 * CRC-16: 软件查表法（芯片硬件CRC不支持REFIN/REFOUT）
 *
 * 帧格式 (RTU):
 *   [slave][func][data...][CRC_lo][CRC_hi]
 *   帧间隔 >= 3.5 char times (~0.6ms @57600)
 */
#ifndef PROTO_MODBUS_H
#define PROTO_MODBUS_H

#include <stdint.h>

/* ========== MODBUS 功能码 ========== */
#define MODBUS_FUNC_READ          0x03u
#define MODBUS_FUNC_WRITE_SINGLE  0x06u
#define MODBUS_FUNC_WRITE_MULTI   0x10u
#define MODBUS_EXC_FLAG           0x80u

/* ========== 从机站号 ========== */
#define MODBUS_SLAVE_HEAD1        5u
#define MODBUS_SLAVE_HEAD2        10u
#define MODBUS_SLAVE_HEAD3        15u
#define MODBUS_SLAVE_HEAD4        20u

/* ========== 帧缓冲大小 ========== */
#define MODBUS_RX_BUF_SIZE        256u  /* RX最大帧长 (hal_comm提供)          */
#define MODBUS_TX_BUF_SIZE        64u   /* TX最大帧长 (8+22*2+CRC)           */
#define MODBUS_MAX_REG_READ       22u   /* 单次最大读寄存器数(0x1000-0x1015) */

/* ========== 解析返回值 ========== */
#define PROTO_MODBUS_OK           0     /* 解析成功                           */
#define PROTO_MODBUS_ERR_CRC     -1     /* CRC校验失败                        */
#define PROTO_MODBUS_ERR_EXC     -2     /* 从机返回异常                       */
#define PROTO_MODBUS_ERR_LEN     -3     /* 帧长度不足                         */

/* ========== 公共接口 ========== */

/* CRC-16 校验 (MODBUS 多项式 0x8005, 初值 0xFFFF) */
uint16_t Proto_Modbus_CRC16(const uint8_t *data, uint16_t len);

/* 构建 0x03 读寄存器帧, 返回帧长度 (含CRC) */
uint16_t Proto_Modbus_BuildRead(uint8_t slave_addr, uint16_t reg_addr,
                                uint16_t reg_count, uint8_t *tx_buf);

/* 构建 0x06 写单个寄存器帧, 返回帧长度 (含CRC) */
uint16_t Proto_Modbus_BuildWriteSingle(uint8_t slave_addr, uint16_t reg_addr,
                                       uint16_t data, uint8_t *tx_buf);

/* 构建 0x10 写多个寄存器帧, 返回帧长度 (含CRC) */
uint16_t Proto_Modbus_BuildWriteMulti(uint8_t slave_addr, uint16_t reg_addr,
                                      uint16_t reg_count, const uint8_t *data,
                                      uint8_t *tx_buf);

/* 解析 MODBUS 响应帧
 * 入参: rx_buf(原始帧), rx_len(帧长度)
 * 出参: out_slave(从机地址), out_func(功能码, bit7=1表示异常),
 *        out_data(寄存器数据/回显数据), out_count(uint16_t条目数)
 * 返回: PROTO_MODBUS_OK / ERR_CRC / ERR_EXC / ERR_LEN
 * 调用方自行分配 out_data 缓冲 (至少 MODBUS_MAX_REG_READ 个 uint16_t) */
int8_t Proto_Modbus_Parse(const uint8_t *rx_buf, uint16_t rx_len,
                          uint8_t *out_slave, uint8_t *out_func,
                          uint16_t *out_data, uint16_t *out_count);

#endif /* PROTO_MODBUS_H */
