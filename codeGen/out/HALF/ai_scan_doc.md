# 项目源码扫描 — AI 数据流分析辅助文档

共扫描 6 个 .c 文件

---

## ../../LIB/APP/app_power.c

- **Group**: AppLibSrc
- **行数**: 6533

### #include 依赖

  - `#include "../../include_io/app_power_io.h"`
  - `#include	"data_type.h"`
  - `#include	"s_time_base.h"`
  - `#include	"commClass.h"`
  - `#include 	"API_TIM.H"`
  - `#include	"API_hrtim.h"`
  - `#include	"API_gpio.h"`
  - `#include	"API_ADC.h"`
  - `#include	"API_DMA.h"`
  - `#include	"API_DAc.h"`
  - `#include	"API_UART.H"`
  - `#include	"API_FMAC.H"`
  - `#include	"app_power.h"`
  - `#include	"s_pid.h"`
  - `#include 	"proto_i2c.h"`
  - `#include	"simulative_uart.h"`
  - `#include	"s_data_stack.h"`
  - `#include    "printMessage.h"`
  - `#include	"pluse.H"`
  - `#include	"phase.h"`
  - `#include	"simulative_uart.h"`
  - `#include    "printMessage.h"`
  - `#include "../../../../../app/ekf/modbus_ekf_regs.h"`

### 函数

| 函数名 | 声明 | 有函数体 |
|--------|------|----------|
| user_Process | `static void user_Process(MODULE_INPUT(AppPower) *in, MODULE_OUTPUT(AppPower) *out, AppPower_PipeFlag` |   |
| ProcessInput | `static void ProcessInput(void)` |   |
| APP_ADC_IRQ_PPGstepDecTxA | `void APP_ADC_IRQ_PPGstepDecTxA(Power_AwdDntr_LINK_t* txaDntr, uint16_t* txaBuff);` |   |
| API_HRTIM1_TEST_CMP1_IRQHandlerCallback | `void API_HRTIM1_TEST_CMP1_IRQHandlerCallback(void)` |   |
| API_ADC_Current1AWD_IRQHandlerCallBack | `void API_ADC_Current1AWD_IRQHandlerCallBack(void)` |   |
| API_ADC_Current2AWD_IRQHandlerCallBack | `void API_ADC_Current2AWD_IRQHandlerCallBack(void)` |   |
| PPGgetAdcValueCh1 | `void	PPGgetAdcValueCh1(void);` |   |
| PPGgetAdcValueCh2 | `void	PPGgetAdcValueCh2(void);` |   |
| PPGgetAdcValueCh3 | `void	PPGgetAdcValueCh3(void);` |   |
| PPGgetAdcValueCh4 | `void	PPGgetAdcValueCh4(void);` |   |
| PPGinit | `void	PPGinit(void);							//void			(*PPGinit)(void);																	//初始化` |   |
| PPGdeadTimeCh1 | `void	PPGdeadTimeCh1(uint8_t downDts,uint8_t upDts);					//void			(*PPGdeadTime)(uint8_t downDts,uint` |   |
| PPGdeadTimeCh2 | `void	PPGdeadTimeCh2(uint8_t downDts,uint8_t upDts);					//void			(*PPGdeadTime)(uint8_t downDts,uint` |   |
| PPGdeadTimeCh3 | `void	PPGdeadTimeCh3(uint8_t downDts,uint8_t upDts);					//void			(*PPGdeadTime)(uint8_t downDts,uint` |   |
| PPGdeadTimeCh4 | `void	PPGdeadTimeCh4(uint8_t downDts,uint8_t upDts);					//void			(*PPGdeadTime)(uint8_t downDts,uint` |   |
| PPGsetValueCh1 | `void	PPGsetValueCh1(PPGvalueDef input);					//void			(*PPGsetValue)(uint32_t pwm);											//立即设置P` |   |
| PPGsetValueCh2 | `void	PPGsetValueCh2(PPGvalueDef input);					//void			(*PPGsetValue)(uint32_t pwm);											//立即设置P` |   |
| PPGsetValueCh3 | `void	PPGsetValueCh3(PPGvalueDef input);					//void			(*PPGsetValue)(uint32_t pwm);											//立即设置P` |   |
| PPGsetValueCh4 | `void	PPGsetValueCh4(PPGvalueDef input);					//void			(*PPGsetValue)(uint32_t pwm);											//立即设置P` |   |
| PPGsetDutyCh1 | `void	PPGsetDutyCh1(uint16_t duty);						//立即设置PPG大小(只设置 占空比，周期为公共周期POWERCYCLE），不经过渐变` |   |
| PPGsetDutyCh2 | `void	PPGsetDutyCh2(uint16_t duty);						//立即设置PPG大小(只设置 占空比，周期为公共周期POWERCYCLE），不经过渐变` |   |
| PPGsetDutyCh3 | `void	PPGsetDutyCh3(uint16_t duty);						//立即设置PPG大小(只设置 占空比，周期为公共周期POWERCYCLE），不经过渐变` |   |
| PPGsetDutyCh4 | `void	PPGsetDutyCh4(uint16_t duty);						//立即设置PPG大小(只设置 占空比，周期为公共周期POWERCYCLE），不经过渐变` |   |
| PPGonOffCh1 | `void	PPGonOffCh1(uint8_t onOff);							//void			(*PPGonOff)(uint8_t onOff);			PPG开关` |   |
| PPGonOffCh2 | `void	PPGonOffCh2(uint8_t onOff);							//void			(*PPGonOff)(uint8_t onOff);			PPG开关` |   |
| PPGonOffCh3 | `void	PPGonOffCh3(uint8_t onOff);							//void			(*PPGonOff)(uint8_t onOff);			PPG开关` |   |
| PPGonOffCh4 | `void	PPGonOffCh4(uint8_t onOff);							//void			(*PPGonOff)(uint8_t onOff);			PPG开关` |   |
| PPGgetAdcValue_ch1 | `void	PPGgetAdcValue_ch1(void);				//	void			(*_PPGgetAdcValue)(void);						//得到对应的ADC值` |   |
| APP_POWER_PanCountInitCh1 | `void	APP_POWER_PanCountInitCh1(void);					//void			(*PanCountInit)(void);							//检锅脉冲计数初始化` |   |
| APP_POWER_PanCountInitCh2 | `void	APP_POWER_PanCountInitCh2(void);					//void			(*PanCountInit)(void);							//检锅脉冲计数初始化` |   |
| APP_POWER_PanCountInitCh3 | `void	APP_POWER_PanCountInitCh3(void);					//void			(*PanCountInit)(void);							//检锅脉冲计数初始化` |   |
| APP_POWER_PanCountInitCh4 | `void	APP_POWER_PanCountInitCh4(void);					//void			(*PanCountInit)(void);							//检锅脉冲计数初始化` |   |
| APP_POWER_PanCountGetValue | `uint8_t	APP_POWER_PanCountGetValue(void);			//uint8_t 	(*PanCountGetValue)(void);					//得到检锅数` |   |
| APP_POWER_PanCountSetValue | `void	APP_POWER_PanCountSetValue(uint8_t onOff);;			//void  		(*PanCountSetValue)(uint8_t onOff);				` |   |
| PPGsetHalf | `void	PPGsetHalf(uint8_t ch,uint16_t pwm);					//输出占空比50%的PWM` |   |
| pulse_detector_reset | `void pulse_detector_reset(RealTimePulseDetector* detector);		//脉冲检测初始化` |   |
| Pulse_detector_process | `int Pulse_detector_process(RealTimePulseDetector* detector);	//脉冲检测` |   |
| APP_POWER_PotTypeCheck_FRE | `void	APP_POWER_PotTypeCheck_FRE(uint8_t ch);			//ch 0:  40K下判断锅具	ch 1: 30k 下判断锅具` |   |
| get_MAXMIN_PPG | `void   get_MAXMIN_PPG(void);	//得到最大最小PPG频率值` |   |
| APP_POWER_PotTypeCheckConfirm | `void	APP_POWER_PotTypeCheckConfirm(void);				//通过功率点确认锅具类型` |   |

### #define 常量

  - `DNTR_BUFF_MAX` = 5
  - `DNTR(id)` = (&Power_Dntr[(id)])
  - `IRON_POWER_H` = 1000/25
  - `IRON_POWER_L` = 600/25
  - `POTNUM` = 4				//炉头数
  - `MasterCh` = 0			//主炉头序号
  - `SlaveCh` = 1				//从炉头序号
  - `MasterRCh` = 2			//主炉头序号
  - `SlaveRCh` = 3				//从炉头序号
  - `PowerPotReset` = PowerAll.flag.bit.rest
  - `PowerPotCheckEnd` = PowerAll.flag.bit.potCheckEnd
  - `PowerOvpValueAll` = PowerAll.ovp
  - `PowerChangeStatus` = PowerAll.changeStatus
  - `PowerScrCnt` = PowerAll.scrCnt
  - `PowerPotNum` = PowerAll.PotNum
  - `PowerMinFre` = PowerAll.minFre
  - `PowerCycle` = PowerAll.pCycle
  - `PowerStack` = PowerAll.stack
  - `g_surge_power` = PowerControl->staticReg->SurgePower		//初始判锅功率
  - `s_leave_time` = PowerControl->staticReg->LeaveTime	//移锅确认次数

---

## ../../LIB/APP/adc_sensor.c

- **Group**: AppLibSrc
- **行数**: 1691

### #include 依赖

  - `#include "../../include_io/app_adc_io.h"`
  - `#include "APP_ADC.H"                   /* enums, DIV_* macros                    */`
  - `#include "power_calculator.h"          /* PowerCalculatorInputDef, PowerResult    */`
  - `#include "API_adc.h"                   /* TxA_ADC_AdcBUFF_NUM, ADC_SELECT_ENUM   */`
  - `#include "API_FMAC.H"                  /* API_FMAC_MEMDEF, FmacLeve              */`
  - `#include "API_hrtim.h"                 /* PotNum, FRE_PER_ADC, API_PPG_GetPeroid */`
  - `#include "API_gpio.h"`
  - `#include "API_TIM.h"`
  - `#include "api_dma.h"`
  - `#include "adc_processing.h"`
  - `#include "printMessage.h"`
  - `#include "wave_capture.h"`
  - `#include "API_adc.h"`
  - `#include "API_TIM.h"`
  - `#include "api_dma.h"`
  - `#include "API_OPAMP.H"`
  - `#include "API_hrtim.h"`
  - `#include "API_gpio.h"`
  - `#include "API_I2C.H"`
  - `#include "API_UART.H"`
  - `#include "API_FMAC.H"`
  - `#include "printMessage.h"`
  - `#include "wave_capture.h"`

### 函数

| 函数名 | 声明 | 有函数体 |
|--------|------|----------|
| user_Process | `static void user_Process(MODULE_INPUT(AppAdc) *in, MODULE_OUTPUT(AppAdc) *out, AppAdc_PipeFlags_t fl` |   |
| ProcessInput | `static void ProcessInput(void)` |   |
| Init | `static void     Init(void);` |   |
| ProcessInput | `static void     ProcessInput(void);` |   |
| APP_ADC_TxaAvgSum | `static void     APP_ADC_TxaAvgSum(uint8_t ch);` |   |
| APP_ADC_AVG_Fun | `static void     APP_ADC_AVG_Fun(void);` |   |
| getIcVc20msOccur | `static uint8_t  getIcVc20msOccur(void);` |   |
| clrIcVc20msOccur | `static void     clrIcVc20msOccur(void);` |   |
| getADCaverage20ms | `static uint32_t getADCaverage20ms(uint8_t ch);` |   |
| AdcValueFun | `uint8_t         AdcValueFun(void);` |   |
| APP_ADC_TxaBuffChange | `static void     APP_ADC_TxaBuffChange(uint8_t ch, TxaHrtimPointDef* point);` |   |
| APP_ADC_GetTxaPeiodPoint | `static void     APP_ADC_GetTxaPeiodPoint(void);` |   |
| APP_ADC_SELECT_GROUP | `static void     APP_ADC_SELECT_GROUP(uint8_t ch);` |   |
| APP_ADC_GET_TxaAvg | `static void     APP_ADC_GET_TxaAvg(void);` |   |
| APP_ADC_GET_TEMPE | `static void     APP_ADC_GET_TEMPE(void);` |   |
| APP_ADC_ZERO_IrqFun | `void            APP_ADC_ZERO_IrqFun(void);` |   |
| APP_ZERO_Adc100usCallBack | `void            APP_ZERO_Adc100usCallBack(void);` |   |
| API_T12_EOC_IRQHandlerCallBack | `void            API_T12_EOC_IRQHandlerCallBack(void);` |   |
| API_HRTIM1_TEST_UPD_IRQHandlerCallback | `void            API_HRTIM1_TEST_UPD_IRQHandlerCallback(uint8_t sourse);` |   |
| API_DMA_TxA_IRQHandlerCallBack | `void            API_DMA_TxA_IRQHandlerCallBack(uint8_t num);` |   |
| API_ADC_PENDV_IRQHandler | `void            API_ADC_PENDV_IRQHandler(void);` |   |
| API_FMAC_AppAdcOverCallBack | `void            API_FMAC_AppAdcOverCallBack(API_FMAC_MEMDEF* FmacMem);` |   |
| API_DMA_M2M_OverCallback | `void            API_DMA_M2M_OverCallback(void);` |   |
| API_ADC_DMA_RecoverFun | `static void     API_ADC_DMA_RecoverFun(ADC_SELECT_ENUM ch);` |   |
| APP_ADC_FmacSetTxa | `static void     APP_ADC_FmacSetTxa(uint16_t size);` |   |
| APP_ADC_MesageBuff | `static uint8_t  APP_ADC_MesageBuff(void);` |   |
| APP_ADC_TxaMessageOut | `static void     APP_ADC_TxaMessageOut(PowerCalculatorInputDef* input);` |   |
| Init | `static void Init(void)` |   |
| AppAdc_getCaculatorValue | `void		AppAdc_getCaculatorValue(MODULE_INPUT(AppAdc) *in)` |   |
| user_Process | `static void user_Process(MODULE_INPUT(AppAdc) *in, MODULE_OUTPUT(AppAdc) *out, AppAdc_PipeFlags_t fl` |   |
| APP_ADC_TxaAvgSum | `void	APP_ADC_TxaAvgSum(uint8_t ch)	//每20ms统计一次功率值` |   |
| APP_ADC_TxaAvgSum | `static void APP_ADC_TxaAvgSum(uint8_t ch)` |   |
| AppAdc_setPowerOutValue | `void		AppAdc_setPowerOutValue(void)` |   |
| APP_ADC_AVG_Fun | `static void APP_ADC_AVG_Fun(void)` |   |
| getIcVc20msOccur | `static uint8_t getIcVc20msOccur(void)` |   |
| clrIcVc20msOccur | `static void clrIcVc20msOccur(void)` |   |
| getADCaverage20ms | `static uint32_t getADCaverage20ms(uint8_t ch)` |   |
| AdcValueFun | `uint8_t AdcValueFun(void)` |   |
| APP_ADC_TxaBuffChange | `static void APP_ADC_TxaBuffChange(uint8_t ch, TxaHrtimPointDef* point)` |   |
| APP_ADC_GetTxaPeiodPoint | `static void APP_ADC_GetTxaPeiodPoint(void)` |   |

### #define 常量

  - `PotChWork` = PotCh1
  - `fmacLeveNum` = FmacLeve
  - `FMAC_OFFSET` = (FmacLeve/2+2)
  - `ADC_POTMAX` = 4
  - `AdcInputValue` = AdcFunRam.inputValue
  - `AdcAverage20ms` = AdcFunRam.adcAverage
  - `Adc20msCount` = AdcFunRam.adc20msCount
  - `IcVc20msOccur` = AdcFunRam.flag20ms
  - `AdcSelect` = AdcFunRam.adcSelect

---

## ../../BaseClass/src/calculator.c

- **Group**: baseClass
- **行数**: 469

### #include 依赖

  - `#include "../../include_io/calculator_io.h"`
  - `#include <string.h>`
  - `#include <math.h>`

### 函数

| 函数名 | 声明 | 有函数体 |
|--------|------|----------|
| user_Process | `static void user_Process(MODULE_INPUT(Calculator) *in, MODULE_OUTPUT(Calculator) *out, Calculator_Pi` |   |
| ProcessInput | `static void ProcessInput(void)` |   |
| FindZeroCrossing | `static uint16_t FindZeroCrossing(const uint16_t* current, uint16_t start, uint16_t end);` |   |
| ProcessAllHead | `static void ProcessAllHead(MODULE_INPUT(Calculator)* head_in, MODULE_OUTPUT(Calculator)* head_out);` |   |
| CalculatePower | `static void CalculatePower(uint16_t* resonant_current,uint16_t* hrtim_values,uint16_t* voltage_data,` |   |
| user_Process | `static void user_Process(MODULE_INPUT(Calculator) *in, MODULE_OUTPUT(Calculator) *out, Calculator_Pi` |   |
| ProcessAllHead | `static void ProcessAllHead(MODULE_INPUT(Calculator)* head_in, MODULE_OUTPUT(Calculator)* head_out)` |   |
| CalculatePower | `static void  CalculatePower(uint16_t* resonant_current,uint16_t* hrtim_values,uint16_t* voltage_data` |   |
| CalculatePhaseAngle | `static void CalculatePhaseAngle(Calculator_OutHead_t* head_out, uint16_t* hrtim_values,` |   |
| FillHRTIMStatus | `static void FillHRTIMStatus(Calculator_OutHead_t* head_out, Calculator_InputParams_t* input)` |   |
| FindZeroCrossing | `static uint16_t FindZeroCrossing(const uint16_t* current, uint16_t start, uint16_t end)` |   |
| Init | `static void Init(void)` |   |

### #define 常量

  - `CALC_POTMAX` = 4
  - `V_SCALE` = 0.10606f    // V/count  (Vref/4096 / 分压比)
  - `I_SCALE` = 0.02523f    // A/count  (Vref/4096 / I分压比)
  - `VDC_SCALE` = 0.10606f    // V/count  (同 V 分压网络)
  - `HRTIM_CLK_HZ` = 144000000   // HRTIM 时钟 144MHz
  - `FRE_PER_ADC` = 384         // 频率每ADC
  - `PHASE_DEG_BASE` = 1800        // 180.0 度定标

---

## ../../BaseClass/src/elec_params.c

- **Group**: baseClass
- **行数**: 487

### #include 依赖

  - `#include "../../include_io/elec_params_io.h"`
  - `#include <math.h>`
  - `#include <string.h>`
  - `#include	"API_gpio.h"`

### 函数

| 函数名 | 声明 | 有函数体 |
|--------|------|----------|
| user_Process | `static void user_Process(MODULE_INPUT(ElecParams) *in, MODULE_OUTPUT(ElecParams) *out, ElecParams_Pi` |   |
| ProcessInput | `static void ProcessInput(void)` |   |
| sort_f | `static void sort_f(float* buf, uint8_t n) {` | ✓ |
| median_f | `static float median_f(float* buf, uint8_t n) {` | ✓ |
| ElecParams_Calc | `static uint8_t ElecParams_Calc(MODULE_OUTPUT_PARAMS(ElecParams, EKF_LKF) *result,` |   |
| Init | `static void Init(void) {` | ✓ |
| user_Process | `static void user_Process(MODULE_INPUT(ElecParams) *in, MODULE_OUTPUT(ElecParams) *out, ElecParams_Pi` |   |

### #define 常量

  - `M_PI` = 3.14159265358979323846f
  - `HRTIM_CLK_HZ` = 768000000.0f
  - `PERIO_CNT` = 20
  - `ELEC_POTMAX` = 4
  - `IH_C_FARAD` = 0.9e-6f     // 谐振电容 0.9 μF
  - `IH_I_SCALE` = 0.02523f    // I_adc → A
  - `IH_VDC_SCALE` = 0.10606f    // Vdc_adc → V
  - `IH_PHI_SCALE` = 0.1f        // phi 单位 0.1° → °
  - `IH_I_PEAK_MIN_RATIO` = 0.3f        // I_peak < 中位数×0.3 丢弃
  - `IH_L_MIN_uH` = 10.0f        // 有效 L 下限
  - `IH_FRES_MIN_kHz` = 5.0f         // 有效 f_
  - `IH_HV_WIN_CNT_MIN` = 4						//相位角取值范围（20个点分成2个10ms 里面的高点， 4和5）

---

## ../../BaseClass/src/ekf_lkf.c

- **Group**: baseClass
- **行数**: 368

### #include 依赖

  - `#include "../../include_io/ekf_lkf_io.h"`
  - `#include <math.h>`
  - `#include <string.h>`

### 函数

| 函数名 | 声明 | 有函数体 |
|--------|------|----------|
| user_Process | `static void user_Process(MODULE_INPUT(EKF_LKF) *in, MODULE_OUTPUT(EKF_LKF) *out, EKF_LKF_PipeFlags_t` |   |
| ProcessInput | `static void ProcessInput(void)` |   |
| obs_from_elec | `static void obs_from_elec(const MODULE_INPUT_PARAMS(ElecParams, EKF_LKF) *p, float *z)` |   |
| ekf_init | `static void ekf_init(EKF_InternalState *s, const MODULE_INPUT_PARAMS(ElecParams, EKF_LKF) *p, const ` |   |
| ekf_predict | `static void ekf_predict(EKF_InternalState *s)` |   |
| ekf_update | `static void ekf_update(EKF_InternalState *s, const float *z,` |   |
| Init | `static void Init(void)` |   |
| ekf_outPut | `void				ekf_outPut(MODULE_INPUT(EKF_LKF) *in,MODULE_OUTPUT(EKF_LKF) *out)` |   |
| user_Process | `static void user_Process(MODULE_INPUT(EKF_LKF) *in, MODULE_OUTPUT(EKF_LKF) *out, EKF_LKF_PipeFlags_t` |   |

### #define 常量

  - `EKF_POTMAX` = 4
  - `N_STATES` = 3   /* [R, L, f_res] */
  - `N_MEAS` = 5   /* [I_peak, Vdc, phi, f_sw, P_W] */
  - `FLOAT_TO_INT(val)` = ((int32_t)((val) * 100.0f + 0.5f))

---

## ../../core/data_switcher.c

- **Group**: core
- **行数**: 145
- **说明**: Data Switcher — PULL 路由调度器 (v2.3 LINK+PARAMS)

### #include 依赖

  - `#include "std_module.h"`
  - `#include "data_switcher.h"`
  - `#include "../include_io/app_adc_io.h"`
  - `#include "../include_io/app_power_io.h"`
  - `#include "../include_io/calculator_io.h"`
  - `#include "../include_io/elec_params_io.h"`
  - `#include "../include_io/ekf_lkf_io.h"`

### 函数

| 函数名 | 声明 | 有函数体 |
|--------|------|----------|
| Switcher_Init | `void Switcher_Init(void)` |   |
| Switcher_Slot_AppAdc | `void Switcher_Slot_AppAdc(void) { s_slot[SLOT_AppAdc].pDoWork(); }` | ✓ |
| Switcher_Slot_AppPower | `void Switcher_Slot_AppPower(void) { s_slot[SLOT_AppPower].pDoWork(); }` | ✓ |
| Switcher_Slot_Calculator | `void Switcher_Slot_Calculator(void) { s_slot[SLOT_Calculator].pDoWork(); }` | ✓ |
| Switcher_Slot_ElecParams | `void Switcher_Slot_ElecParams(void) { s_slot[SLOT_ElecParams].pDoWork(); }` | ✓ |
| Switcher_Slot_EKF_LKF | `void Switcher_Slot_EKF_LKF(void) { s_slot[SLOT_EKF_LKF].pDoWork(); }` | ✓ |
| Switcher_Run_All | `void Switcher_Run_All(void)` |   |
| Switcher_Run_Slot1 | `void Switcher_Run_Slot1(void)` |   |
| Switcher_Run_TK | `void Switcher_Run_TK(void)` |   |

---
