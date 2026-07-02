/* DLL bridge: 导出 power_calculator.c 的生产函数供 Python 测试 */
#define EXPORT __declspec(dllexport)

#include "power_calculator.h"

/* CalculatePower_FPU 在生产代码中存在但未在头文件中声明 */
extern PowerResult CalculatePower_FPU(
    uint16_t* resonant_current,
    uint16_t* hrtim_values,
    uint16_t* voltage_values,
    PowerCalculatorInputDef* input);

/* ---- 1ms 整数版 ---- */
EXPORT PowerResult DLL_CalculatePower(
    uint16_t* resonant_current,
    uint16_t* hrtim_values,
    uint16_t* voltage_values,
    PowerCalculatorInputDef* input)
{
    return CalculatePower(resonant_current, hrtim_values, voltage_values, input);
}

/* ---- 1ms FPU 版 ---- */
EXPORT PowerResult DLL_CalculatePower_FPU(
    uint16_t* resonant_current,
    uint16_t* hrtim_values,
    uint16_t* voltage_values,
    PowerCalculatorInputDef* input)
{
    return CalculatePower_FPU(resonant_current, hrtim_values, voltage_values, input);
}

/* ---- 20ms 四头电参数 ---- */
EXPORT void DLL_CalculateElecParams_20ms(
    uint16_t* current_buf[4],
    uint16_t* hrtim_buf[4],
    uint16_t* voltage_buf[4],
    PowerCalculatorInputDef* input[4],
    ElecParamsDef elec[4])
{
    CalculateElecParams_20ms(current_buf, hrtim_buf, voltage_buf, input, elec);
}
