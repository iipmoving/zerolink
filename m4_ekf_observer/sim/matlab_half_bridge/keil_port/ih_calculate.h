/******************************************************************************
 * ih_calculate.h — 半桥感应加热 参量计算函数声明 (CMSIS-DSP)
 *
 * 所有函数接收 RAW 缓冲区，输出 IH_Result_t
 * 使用 float 运算，充分利用 M4 FPU
 ******************************************************************************/

#ifndef IH_CALCULATE_H
#define IH_CALCULATE_H

#include "ih_params.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  单帧完整参数计算 (一次调用完成全部)
 * @param  raw    输入: 原始数据缓冲区
 * @param  result 输出: 计算结果
 * @return 0=成功, -1=数据不足
 */
int32_t IH_Calculate(const RawBuffer_t *raw, IH_Result_t *result);

/**
 * @brief  HRTIM 时序分析
 * @param  raw    输入
 * @param  timing 输出: 四边沿CMP/死区/占空比/控制模式
 */
void IH_CalcTiming(const RawBuffer_t *raw, TimingResult_t *timing);

/**
 * @brief  时域参量 (谐振频率/峰值/RMS/相位)
 * @param  raw      输入
 * @param  timing   输入 (备 f_sw)
 * @param  waveform 输出
 */
void IH_CalcWaveform(const RawBuffer_t *raw,
                     const TimingResult_t *timing,
                     WaveformResult_t *waveform);

/**
 * @brief  功率计算 (P/S/Q/PF/效率)
 * @param  raw      输入
 * @param  waveform 输入
 * @param  power    输出
 */
void IH_CalcPower(const RawBuffer_t *raw,
                  const WaveformResult_t *waveform,
                  PowerResult_t *power);

/**
 * @brief  RLC 阻抗建模
 * @param  waveform  输入
 * @param  power     输入
 * @param  impedance 输出
 */
void IH_CalcImpedance(const WaveformResult_t *waveform,
                      const PowerResult_t *power,
                      ImpedanceResult_t *impedance);

/**
 * @brief  获取结果文本 (用于串口输出 / LCD)
 * @param  result 计算结果
 * @param  buf    输出缓冲区 (至少 512 字节)
 * @param  size   缓冲区大小
 * @return 写入的字符数
 */
int IH_ResultToString(const IH_Result_t *result, char *buf, int size);

#ifdef __cplusplus
}
#endif

#endif /* IH_CALCULATE_H */
