
#include "rx32g4xx_config_def.h"
#include "rx32g4xx_hal.h"
#include "system_init.h"
#include "system_bsp.h"

#include 	"api_tim.h"


/*
TIM1:BLANK	消抖
TIM2:FAN
TIM3:TXA        触发ADC
TIM4:  
TIM5:
TIM6:	100US
TIM7:	PAN
TIM8:	ZERO
TIM16:  SCR

*/
#define TIM_freq (192*1000*1000)  //hz
#define TIM_freq_us (192)  				//1us计数值 
//-------过零常量设置----------------------------------------------
#define		TIM_ZERO					TIM8
#define		TIM_ZERO_OUTPUT_CHANNEL		TIM_CHANNEL_5

#define	ZERO_freq_50hz	50		//hz
#define	ZERO_freq_60hz	60		//hz

#define ZERO_Prescaler 191  

#define ZERO_ARR_50hz (TIM_freq/(ZERO_Prescaler+1)/ZERO_freq_50hz)
#define ZERO_ARR_60hz (TIM_freq/(ZERO_Prescaler+1)/ZERO_freq_60hz)

#define	TIM_DIV200_50hz	(50*200)		//hz		50HZ下200等分
#define	TIM_DIV200_60hz	(60*200)		//hz		60HZ下200等分	

#define TIM_DIV200_Prescaler 191  

#define TIM_DIV200_ARR_50hz (uint32_t)((TIM_freq/(TIM_DIV200_Prescaler+1)/TIM_DIV200_50hz))
#define TIM_DIV200_ARR_60hz (uint32_t)(TIM_freq/(TIM_DIV200_Prescaler+1)/TIM_DIV200_60hz)
//==========100us常量设置==================================


#define		TIM_100US		TIM6				//



//============================================
//==========4us常量设置==================================

#define		TIM_PAN		TIM7			//触发检锅ADC DMA 注意DMA DMA_REQUEST_PAN_UP要一致
#define 	TIM_PAN_Prescaler 0  
#define		TIM_PAN_OUTPUT_CHANNEL		TIM_CHANNEL_3





#define			TIM_PAN_us		1

#define 		TIM_ARR_PAN (TIM_freq_us*TIM_PAN_us/(TIM_PAN_Prescaler+1))
#define 		TIM_IRQn_PAN						TIM7_DAC_IRQn
#define 		IRQ_PRIORITY_TIM_PAN		IRQ_PRIORITY_TIM7_DAC


//==========FAN常量设置==================================
#define		TIM_FAN		TIM2			//触发检锅ADC DMA
#define		TIM_FAN_OUTPUT_CHANNEL		TIM_CHANNEL_4

#define 	TIM_FAN_Prescaler 0  
#define		TIM_FAN_us		10	
#define 	TIM_ARR_FAN (TIM_freq_us*TIM_FAN_us/(TIM_FAN_Prescaler+1))

#define 		TIM_IT_FAN							TIM_IT_CC4
#define 		TIM_FLAG_FAN						TIM_FLAG_CC4
#define 		TIM_IRQn_FAN						TIM2_IRQn
#define 		IRQ_PRIORITY_TIM_FAN		IRQ_PRIORITY_TIM2

//==========FAN常量设置==================================
#define		TIM_SCR		TIM16			//触发检锅ADC DMA
#define		TIM_SCR_OUTPUT_CHANNEL		TIM_CHANNEL_1

#define 	TIM_SCR_Prescaler 0  
#define		TIM_SCR_us		10	
#define 	TIM_ARR_SCR (TIM_freq_us*TIM_SCR_us/(TIM_SCR_Prescaler+1))

#define 		TIM_IT_SCR							TIM_IT_CC1
#define 		TIM_FLAG_SCR						TIM_FLAG_CC1
#define 		TIM_IRQn_SCR						TIM1_UP_TIM16_IRQn
#define 		IRQ_PRIORITY_TIM_SCR		        IRQ_PRIORITY_TIM1_UP_TIM16

//==========TXA 延时常量设置(用于谐振电流每1us间隔DMA触发)==================================
#define		    TIM_TXA		TIM3					//谐振电流过流检测延时
#define		    TIM_TXA_OUTPUT_CHANNEL		TIM_CHANNEL_ALL  //CH1 2 3 4

#define 	    TIM_TXA_Prescaler 0  

#define 	    TIM_ARR_TXA 	(TIM_freq_us*TIM_TXA_PERIO_us/(TIM_TXA_Prescaler+1))			//周期
#define				TIM_TXA_CC1		(TIM_freq_us*TIM_TXA_CC1_us/(TIM_TXA_Prescaler+1))	//第一ADC DMA点
#define				TIM_TXA_CC2		(TIM_freq_us*TIM_TXA_CC2_us/(TIM_TXA_Prescaler+1))	//第二ADC DMA点

#define				TIM_TXA_CC3		(TIM_freq_us*TIM_TXA_CC3_us/(TIM_TXA_Prescaler+1))	//第一ADC DMA点
#define				TIM_TXA_CC4		(TIM_freq_us*TIM_TXA_CC4_us/(TIM_TXA_Prescaler+1))	//第二ADC DMA点


#define 		TIM_IT_TXA							TIM_IT_CC1
#define 		TIM_FLAG_TXA						TIM_FLAG_CC1
#define 		TIM_IRQn_TXA						TIM3_IRQn
#define 		IRQ_PRIORITY_TIM_TXA		        IRQ_PRIORITY_TIM3


//==========comp  COMP_CR_BLANKING_SEL_TIM1_OC5 遮蔽源 ==================================
#define		TIM_BLANKING		TIM1					//谐振电流过流消抖

#if			TIM_BLANKING_TEST	//配置HRTIM COMP消抖 测试通道
    #define		TIM_BLANKING_OUTPUT_CHANNEL		TIM_CHANNEL_4//用于测试HRTIM COMP消抖
#else
    #define		TIM_BLANKING_OUTPUT_CHANNEL		TIM_CHANNEL_5
#endif


#define 	TIM_BLANKING_Prescaler 23  
#define		TIM_BLANKING_us		200	
#define 	TIM_ARR_BLANKING 	(TIM_freq_us*TIM_BLANKING_us/(TIM_BLANKING_Prescaler+1))	//

#define 		TIM_IT_BLANKING							TIM_IT_CC5
#define 		TIM_FLAG_BLANKING						TIM_FLAG_CC5
#define 		TIM_IRQn_BLANKING						TIM1_CC_IRQn
#define 		IRQ_PRIORITY_TIM_BLANKING				IRQ_PRIORITY_TIM1_CC


uint32_t        API_TIM_TxaSmcrSave;            //保存从模式

//==========comp  COMP_CR_BLANKING_SEL_TIM5_OC3 遮蔽源 ==================================
#if 0
#define		TIM_BLANKING_DOUBLE		TIM5					//倍频时谐振电流过流消抖

#if			TIM_BLANKING_TEST	//配置HRTIM COMP消抖 测试通道
    #define		TIM_BLANKING_DOUBLE_OUTPUT_CHANNEL		TIM_CHANNEL_4//用于测试HRTIM COMP消抖
#else
    #define		TIM_BLANKING_DOUBLE_OUTPUT_CHANNEL		TIM_CHANNEL_3
#endif


// #define 	TIM_BLANKING_Prescaler 23  
// #define		TIM_BLANKING_us		200	
#define 	TIM_ARR_BLANKING_DOUBLE 	(TIM_freq_us*TIM_BLANKING_us/(TIM_BLANKING_Prescaler+1))	//


#define 		TIM_IT_BLANKING_DOUBLE							TIM_IT_CC3
#define 		TIM_FLAG_BLANKING_DOUBLE						TIM_FLAG_CC3
#define 		TIM_IRQn_BLANKING_DOUBLE						TIM5_CC_IRQn
#define 		IRQ_PRIORITY_TIM_BLANKING_DOUBLE				IRQ_PRIORITY_TIM5_CC
		
#endif

typedef struct          //多个通道设置
{
    TIM_OC_InitTypeDef*		OC;
    uint32_t       channel;	

}TIM_Base_OcChannelDef;
typedef struct          //多个通道设置
{
    uint32_t size;          //有多少个通道
    TIM_Base_OcChannelDef   ocChannel[4];

}TIM_Base_OcGroupDef;

typedef struct 
{
		TIM_TypeDef*		timx;					//TIMX选择
		TIM_Base_InitTypeDef* Init;		//初始化参数
		TIM_MasterConfigTypeDef* master;	//主模式设置
		TIM_SlaveConfigTypeDef*	slave;		//从模式设置	

		TIM_OC_InitTypeDef*		OC;					//输出设置  多通道时OC地址为TIM_Base_OcGroupDef
        uint32_t 	channel;						//输出通道号 通道号为TIM_CHANNEL_ALL时为多通道

		uint32_t	it;											//中断号
		uint32_t	it_flag;								//中断标志
		uint32_t  irq;										//中断请求	
		uint32_t  priority;								//中断优先级
}TIM_Base_HandleTypeDef;

TIM_HandleTypeDef    htim_pan;			///TIM3
TIM_HandleTypeDef    htim_100us;		///TIM8	
TIM_HandleTypeDef    htim_zero;			///TIM6
TIM_HandleTypeDef    htim_FAN;			///TIM2
TIM_HandleTypeDef    htim_TXA;			///TIM7	
TIM_HandleTypeDef    htim_BLANKING;		///TIM1
TIM_HandleTypeDef    htim_BLANKING_DOUBLE;		///TIM5
TIM_HandleTypeDef    htim_SCR;		///TIM5

TIM_Base_InitTypeDef const	TIMx_Init={
  
	ZERO_Prescaler,/*uint32_t Prescaler;         !< Specifies the prescaler value used to divide the TIM clock.
                                   This parameter can be a number between Min_Data = 0x0000 and Max_Data = 0xFFFF
                                   Macro __HAL_TIM_CALC_PSC() can be used to calculate prescaler value */

  TIM_COUNTERMODE_UP,/*uint32_t CounterMode;       !< Specifies the counter mode.
                                   This parameter can be a value of @ref TIM_Counter_Mode */

  ZERO_ARR_50hz-1,/*uint32_t Period;            !< Specifies the period value to be loaded into the active
                                   Auto-Reload Register at the next update event.
                                   This parameter can be a number between Min_Data = 0x0000 and Max_Data = 0xFFFF
                                   (or 0xFFEF if dithering is activated)Macros __HAL_TIM_CALC_PERIOD(),
                                    __HAL_TIM_CALC_PERIOD_DITHER(),__HAL_TIM_CALC_PERIOD_BY_DELAY(),
                                    __HAL_TIM_CALC_PERIOD_DITHER_BY_DELAY()can be used to calculate Period value */

  TIM_CLOCKDIVISION_DIV1,/*uint32_t ClockDivision;     !< Specifies the clock division.
                                   This parameter can be a value of @ref TIM_ClockDivision */

  0,/*uint32_t RepetitionCounter;  !< Specifies the repetition counter value. Each time the RCR downcounter
                                    reaches zero, an update event is generated and counting restarts
                                    from the RCR value (N).
                                    This means in PWM mode that (N+1) corresponds to:
                                        - the number of PWM periods in edge-aligned mode
                                        - the number of half PWM period in center-aligned mode
                                     GP timers: this parameter must be a number between Min_Data = 0x00 and
                                     Max_Data = 0xFF.
                                     Advanced timers: this parameter must be a number between Min_Data = 0x0000 and
                                     Max_Data = 0xFFFF. */

  TIM_AUTORELOAD_PRELOAD_ENABLE,/*uint32_t AutoReloadPreload;  !< Specifies the auto-reload preload.
                                   This parameter can be a value of @ref TIM_AutoReloadPreload */



};

TIM_Base_InitTypeDef const	TIM_PAN_Init={
  
	TIM_PAN_Prescaler,/*uint32_t Prescaler;         !< Specifies the prescaler value used to divide the TIM clock.
                                   This parameter can be a number between Min_Data = 0x0000 and Max_Data = 0xFFFF
                                   Macro __HAL_TIM_CALC_PSC() can be used to calculate prescaler value */

  TIM_COUNTERMODE_UP,/*uint32_t CounterMode;       !< Specifies the counter mode.
                                   This parameter can be a value of @ref TIM_Counter_Mode */

  TIM_ARR_PAN-1,/*uint32_t Period;            !< Specifies the period value to be loaded into the active
                                   Auto-Reload Register at the next update event.
                                   This parameter can be a number between Min_Data = 0x0000 and Max_Data = 0xFFFF
                                   (or 0xFFEF if dithering is activated)Macros __HAL_TIM_CALC_PERIOD(),
                                    __HAL_TIM_CALC_PERIOD_DITHER(),__HAL_TIM_CALC_PERIOD_BY_DELAY(),
                                    __HAL_TIM_CALC_PERIOD_DITHER_BY_DELAY()can be used to calculate Period value */

  TIM_CLOCKDIVISION_DIV1,/*uint32_t ClockDivision;     !< Specifies the clock division.
                                   This parameter can be a value of @ref TIM_ClockDivision */

  0,/*uint32_t RepetitionCounter;  !< Specifies the repetition counter value. Each time the RCR downcounter
                                    reaches zero, an update event is generated and counting restarts
                                    from the RCR value (N).
                                    This means in PWM mode that (N+1) corresponds to:
                                        - the number of PWM periods in edge-aligned mode
                                        - the number of half PWM period in center-aligned mode
                                     GP timers: this parameter must be a number between Min_Data = 0x00 and
                                     Max_Data = 0xFF.
                                     Advanced timers: this parameter must be a number between Min_Data = 0x0000 and
                                     Max_Data = 0xFFFF. */

  TIM_AUTORELOAD_PRELOAD_ENABLE,/*uint32_t AutoReloadPreload;  !< Specifies the auto-reload preload.
                                   This parameter can be a value of @ref TIM_AutoReloadPreload */



};

TIM_Base_InitTypeDef const	TIM_TXA_Init={
  
	TIM_TXA_Prescaler,/*uint32_t Prescaler;         !< Specifies the prescaler value used to divide the TIM clock.
                                   This parameter can be a number between Min_Data = 0x0000 and Max_Data = 0xFFFF
                                   Macro __HAL_TIM_CALC_PSC() can be used to calculate prescaler value */

  TIM_COUNTERMODE_UP,/*uint32_t CounterMode;       !< Specifies the counter mode.
                                   This parameter can be a value of @ref TIM_Counter_Mode */

  TIM_ARR_TXA-1,/*uint32_t Period;            !< Specifies the period value to be loaded into the active
                                   Auto-Reload Register at the next update event.
                                   This parameter can be a number between Min_Data = 0x0000 and Max_Data = 0xFFFF
                                   (or 0xFFEF if dithering is activated)Macros __HAL_TIM_CALC_PERIOD(),
                                    __HAL_TIM_CALC_PERIOD_DITHER(),__HAL_TIM_CALC_PERIOD_BY_DELAY(),
                                    __HAL_TIM_CALC_PERIOD_DITHER_BY_DELAY()can be used to calculate Period value */

  TIM_CLOCKDIVISION_DIV1,/*uint32_t ClockDivision;     !< Specifies the clock division.
                                   This parameter can be a value of @ref TIM_ClockDivision */

  0,/*uint32_t RepetitionCounter;  !< Specifies the repetition counter value. Each time the RCR downcounter
                                    reaches zero, an update event is generated and counting restarts
                                    from the RCR value (N).
                                    This means in PWM mode that (N+1) corresponds to:
                                        - the number of PWM periods in edge-aligned mode
                                        - the number of half PWM period in center-aligned mode
                                     GP timers: this parameter must be a number between Min_Data = 0x00 and
                                     Max_Data = 0xFF.
                                     Advanced timers: this parameter must be a number between Min_Data = 0x0000 and
                                     Max_Data = 0xFFFF. */

  TIM_AUTORELOAD_PRELOAD_DISABLE,/*uint32_t AutoReloadPreload;  !< Specifies the auto-reload preload.
                                   This parameter can be a value of @ref TIM_AutoReloadPreload */



};

TIM_Base_InitTypeDef const	TIM_FAN_Init={
  
	TIM_FAN_Prescaler,/*uint32_t Prescaler;         !< Specifies the prescaler value used to divide the TIM clock.
                                   This parameter can be a number between Min_Data = 0x0000 and Max_Data = 0xFFFF
                                   Macro __HAL_TIM_CALC_PSC() can be used to calculate prescaler value */

  TIM_COUNTERMODE_UP,/*uint32_t CounterMode;       !< Specifies the counter mode.
                                   This parameter can be a value of @ref TIM_Counter_Mode */

  TIM_ARR_FAN-1,/*uint32_t Period;            !< Specifies the period value to be loaded into the active
                                   Auto-Reload Register at the next update event.
                                   This parameter can be a number between Min_Data = 0x0000 and Max_Data = 0xFFFF
                                   (or 0xFFEF if dithering is activated)Macros __HAL_TIM_CALC_PERIOD(),
                                    __HAL_TIM_CALC_PERIOD_DITHER(),__HAL_TIM_CALC_PERIOD_BY_DELAY(),
                                    __HAL_TIM_CALC_PERIOD_DITHER_BY_DELAY()can be used to calculate Period value */

  TIM_CLOCKDIVISION_DIV1,/*uint32_t ClockDivision;     !< Specifies the clock division.
                                   This parameter can be a value of @ref TIM_ClockDivision */

  0,/*uint32_t RepetitionCounter;  !< Specifies the repetition counter value. Each time the RCR downcounter
                                    reaches zero, an update event is generated and counting restarts
                                    from the RCR value (N).
                                    This means in PWM mode that (N+1) corresponds to:
                                        - the number of PWM periods in edge-aligned mode
                                        - the number of half PWM period in center-aligned mode
                                     GP timers: this parameter must be a number between Min_Data = 0x00 and
                                     Max_Data = 0xFF.
                                     Advanced timers: this parameter must be a number between Min_Data = 0x0000 and
                                     Max_Data = 0xFFFF. */

  TIM_AUTORELOAD_PRELOAD_DISABLE,/*uint32_t AutoReloadPreload;  !< Specifies the auto-reload preload.
                                   This parameter can be a value of @ref TIM_AutoReloadPreload */



};
TIM_Base_InitTypeDef const	TIM_SCR_Init={
  
	TIM_SCR_Prescaler,/*uint32_t Prescaler;         !< Specifies the prescaler value used to divide the TIM clock.
                                   This parameter can be a number between Min_Data = 0x0000 and Max_Data = 0xFFFF
                                   Macro __HAL_TIM_CALC_PSC() can be used to calculate prescaler value */

  TIM_COUNTERMODE_UP,/*uint32_t CounterMode;       !< Specifies the counter mode.
                                   This parameter can be a value of @ref TIM_Counter_Mode */

  TIM_ARR_SCR-1,/*uint32_t Period;            !< Specifies the period value to be loaded into the active
                                   Auto-Reload Register at the next update event.
                                   This parameter can be a number between Min_Data = 0x0000 and Max_Data = 0xFFFF
                                   (or 0xFFEF if dithering is activated)Macros __HAL_TIM_CALC_PERIOD(),
                                    __HAL_TIM_CALC_PERIOD_DITHER(),__HAL_TIM_CALC_PERIOD_BY_DELAY(),
                                    __HAL_TIM_CALC_PERIOD_DITHER_BY_DELAY()can be used to calculate Period value */

  TIM_CLOCKDIVISION_DIV1,/*uint32_t ClockDivision;     !< Specifies the clock division.
                                   This parameter can be a value of @ref TIM_ClockDivision */

  0,/*uint32_t RepetitionCounter;  !< Specifies the repetition counter value. Each time the RCR downcounter
                                    reaches zero, an update event is generated and counting restarts
                                    from the RCR value (N).
                                    This means in PWM mode that (N+1) corresponds to:
                                        - the number of PWM periods in edge-aligned mode
                                        - the number of half PWM period in center-aligned mode
                                     GP timers: this parameter must be a number between Min_Data = 0x00 and
                                     Max_Data = 0xFF.
                                     Advanced timers: this parameter must be a number between Min_Data = 0x0000 and
                                     Max_Data = 0xFFFF. */

  TIM_AUTORELOAD_PRELOAD_DISABLE,/*uint32_t AutoReloadPreload;  !< Specifies the auto-reload preload.
                                   This parameter can be a value of @ref TIM_AutoReloadPreload */



};

TIM_Base_InitTypeDef const	TIM_BLANKING_Init={
  
	TIM_BLANKING_Prescaler,/*uint32_t Prescaler;         !< Specifies the prescaler value used to divide the TIM clock.
                                   This parameter can be a number between Min_Data = 0x0000 and Max_Data = 0xFFFF
                                   Macro __HAL_TIM_CALC_PSC() can be used to calculate prescaler value */

  TIM_COUNTERMODE_UP,/*uint32_t CounterMode;       !< Specifies the counter mode.
                                   This parameter can be a value of @ref TIM_Counter_Mode */

  TIM_ARR_BLANKING-1,/*uint32_t Period;            !< Specifies the period value to be loaded into the active
                                   Auto-Reload Register at the next update event.
                                   This parameter can be a number between Min_Data = 0x0000 and Max_Data = 0xFFFF
                                   (or 0xFFEF if dithering is activated)Macros __HAL_TIM_CALC_PERIOD(),
                                    __HAL_TIM_CALC_PERIOD_DITHER(),__HAL_TIM_CALC_PERIOD_BY_DELAY(),
                                    __HAL_TIM_CALC_PERIOD_DITHER_BY_DELAY()can be used to calculate Period value */

  TIM_CLOCKDIVISION_DIV1,/*uint32_t ClockDivision;     !< Specifies the clock division.
                                   This parameter can be a value of @ref TIM_ClockDivision */

  0,/*uint32_t RepetitionCounter;  !< Specifies the repetition counter value. Each time the RCR downcounter
                                    reaches zero, an update event is generated and counting restarts
                                    from the RCR value (N).
                                    This means in PWM mode that (N+1) corresponds to:
                                        - the number of PWM periods in edge-aligned mode
                                        - the number of half PWM period in center-aligned mode
                                     GP timers: this parameter must be a number between Min_Data = 0x00 and
                                     Max_Data = 0xFF.
                                     Advanced timers: this parameter must be a number between Min_Data = 0x0000 and
                                     Max_Data = 0xFFFF. */

  TIM_AUTORELOAD_PRELOAD_DISABLE,/*uint32_t AutoReloadPreload;  !< Specifies the auto-reload preload.
                                   This parameter can be a value of @ref TIM_AutoReloadPreload */



};

//主模式使能
TIM_MasterConfigTypeDef const TIM_MasterConfig_Enable=
{
  TIM_TRGO_UPDATE,/*uint32_t  MasterOutputTrigger;   !< Trigger output (TRGO) selection
                                        This parameter can be a value of @ref TIM_Master_Mode_Selection */
  TIM_TRGO2_RESET,/*uint32_t  MasterOutputTrigger2;  !< Trigger output2 (TRGO2) selection
                                        This parameter can be a value of @ref TIM_Master_Mode_Selection_2 */
  TIM_MASTERSLAVEMODE_ENABLE,/*uint32_t  MasterSlaveMode;       !< Master/slave mode selection
                                        This parameter can be a value of @ref TIM_Master_Slave_Mode
                                        @note When the Master/slave mode is enabled, the effect of
                                        an event on the trigger input (TRGI) is delayed to allow a
                                        perfect synchronization between the current timer and its
                                        slaves (through TRGO). It is not mandatory in case of timer
                                        synchronization mode. */
};


//主模式取消
TIM_MasterConfigTypeDef const TIM_MasterConfig_Disable=
{
  TIM_TRGO_UPDATE,/*uint32_t  MasterOutputTrigger;   !< Trigger output (TRGO) selection
                                        This parameter can be a value of @ref TIM_Master_Mode_Selection */
  TIM_TRGO2_RESET,/*uint32_t  MasterOutputTrigger2;  !< Trigger output2 (TRGO2) selection
                                        This parameter can be a value of @ref TIM_Master_Mode_Selection_2 */
  TIM_MASTERSLAVEMODE_DISABLE,/*uint32_t  MasterSlaveMode;       !< Master/slave mode selection
                                        This parameter can be a value of @ref TIM_Master_Slave_Mode
                                        @note When the Master/slave mode is enabled, the effect of
                                        an event on the trigger input (TRGI) is delayed to allow a
                                        perfect synchronization between the current timer and its
                                        slaves (through TRGO). It is not mandatory in case of timer
                                        synchronization mode. */
};
#define	TIM_40us_MasterConfig		TIM_MasterConfig_Disable
#define	TIM_TXA_MasterConfig		TIM_MasterConfig_Enable
#define	TIM_FAN_MasterConfig		TIM_MasterConfig_Disable
#define	TIM_PAN_MasterConfig		TIM_MasterConfig_Disable
#define	TIM_SCR_MasterConfig		TIM_MasterConfig_Disable

//HRTIM触发复位
TIM_SlaveConfigTypeDef const TIM_BLANKING_SlaveConfig=
{
  TIM_SLAVEMODE_RESET, 			/*uint32_t  SlaveMode;         !< Slave mode selection
                                    This parameter can be a value of @ref TIM_Slave_Mode */
  TIM_TS_ITR10,						/*uint32_t  InputTrigger;      !< Input Trigger source
                                    This parameter can be a value of @ref TIM_Trigger_Selection */
  TIM_INPUTCHANNELPOLARITY_RISING,	/*uint32_t  TriggerPolarity;   !< Input Trigger polarity
                                    This parameter can be a value of @ref TIM_Trigger_Polarity */
  TIM_TRIGGERPRESCALER_DIV1,		/*uint32_t  TriggerPrescaler;  !< Input trigger prescaler
                                    This parameter can be a value of @ref TIM_Trigger_Prescaler */
  0,								/*uint32_t  TriggerFilter;     !< Input trigger filter*/
      
};

TIM_SlaveConfigTypeDef const TIM_TXA_SlaveConfig=
{
  TIM_SLAVEMODE_DISABLE, 			/*uint32_t  SlaveMode;         !< Slave mode selection
                                    This parameter can be a value of @ref TIM_Slave_Mode */
  TIM_TS_ITR10,						/*uint32_t  InputTrigger;      !< Input Trigger source
                                    This parameter can be a value of @ref TIM_Trigger_Selection */
  TIM_INPUTCHANNELPOLARITY_RISING,	/*uint32_t  TriggerPolarity;   !< Input Trigger polarity
                                    This parameter can be a value of @ref TIM_Trigger_Polarity */
  TIM_TRIGGERPRESCALER_DIV1,		/*uint32_t  TriggerPrescaler;  !< Input trigger prescaler
                                    This parameter can be a value of @ref TIM_Trigger_Prescaler */
  0,								/*uint32_t  TriggerFilter;     !< Input trigger filter*/
      
};




TIM_OC_InitTypeDef const 		TIM_BLANKING_OC_Init=
{
  TIM_OCMODE_PWM1,			/*uint32_t OCMode;        !< Specifies the TIM mode.
                               This parameter can be a value of @ref TIM_Output_Compare_and_PWM_modes */

  TIM_BLANKING_PLUSE,				/*uint32_t Pulse;         !< Specifies the pulse value to be loaded into the Capture Compare Register.
                               This parameter can be a number between Min_Data = 0x0000 and Max_Data = 0xFFFF
                               (or 0xFFEF if dithering is activated)
                               Macros __HAL_TIM_CALC_PULSE(), __HAL_TIM_CALC_PULSE_DITHER() can be used to calculate
                               Pulse value */

  TIM_OCPOLARITY_HIGH,		/*uint32_t OCPolarity;    !< Specifies the output polarity.
                               This parameter can be a value of @ref TIM_Output_Compare_Polarity */

  TIM_OCNPOLARITY_HIGH,/*uint32_t OCNPolarity;   !< Specifies the complementary output polarity.
                               This parameter can be a value of @ref TIM_Output_Compare_N_Polarity
                               @note This parameter is valid only for timer instances supporting break feature. */

  TIM_OCFAST_DISABLE,/*uint32_t OCFastMode;    !< Specifies the Fast mode state.
                               This parameter can be a value of @ref TIM_Output_Fast_State
                               @note This parameter is valid only in PWM1 and PWM2 mode. */


	TIM_OCIDLESTATE_RESET, /*uint32_t OCIdleState;   !< Specifies the TIM Output Compare pin state during Idle state.
                               This parameter can be a value of @ref TIM_Output_Compare_Idle_State
                               @note This parameter is valid only for timer instances supporting break feature. */

  TIM_OCNIDLESTATE_RESET,/*uint32_t OCNIdleState;  !< Specifies the TIM Output Compare pin state during Idle state.
                               This parameter can be a value of @ref TIM_Output_Compare_N_Idle_State
                               @note This parameter is valid only for timer instances supporting break feature. */
} ;

#define	TIM_BLANKING_OC	TIM_BLANKING_OC_Init			//占空比在程序中设置


TIM_OC_InitTypeDef const 		TIM_OC_Init=
{
    TIM_OCMODE_PWM1,			/*uint32_t OCMode;        !< Specifies the TIM mode.
                               This parameter can be a value of @ref TIM_Output_Compare_and_PWM_modes */

    0,				/*uint32_t Pulse;         !< Specifies the pulse value to be loaded into the Capture Compare Register.
                               This parameter can be a number between Min_Data = 0x0000 and Max_Data = 0xFFFF
                               (or 0xFFEF if dithering is activated)
                               Macros __HAL_TIM_CALC_PULSE(), __HAL_TIM_CALC_PULSE_DITHER() can be used to calculate
                               Pulse value */

    TIM_OCPOLARITY_HIGH,		/*uint32_t OCPolarity;    !< Specifies the output polarity.
                               This parameter can be a value of @ref TIM_Output_Compare_Polarity */

    TIM_OCNPOLARITY_HIGH,/*uint32_t OCNPolarity;   !< Specifies the complementary output polarity.
                               This parameter can be a value of @ref TIM_Output_Compare_N_Polarity
                               @note This parameter is valid only for timer instances supporting break feature. */

    TIM_OCFAST_DISABLE,/*uint32_t OCFastMode;    !< Specifies the Fast mode state.
                               This parameter can be a value of @ref TIM_Output_Fast_State
                               @note This parameter is valid only in PWM1 and PWM2 mode. */


	TIM_OCIDLESTATE_RESET, /*uint32_t OCIdleState;   !< Specifies the TIM Output Compare pin state during Idle state.
                               This parameter can be a value of @ref TIM_Output_Compare_Idle_State
                               @note This parameter is valid only for timer instances supporting break feature. */

    TIM_OCNIDLESTATE_RESET,/*uint32_t OCNIdleState;  !< Specifies the TIM Output Compare pin state during Idle state.
                               This parameter can be a value of @ref TIM_Output_Compare_N_Idle_State
                               @note This parameter is valid only for timer instances supporting break feature. */
} ;

#define	TIM_FAN_OC	TIM_OC_Init			//占空比在程序中设置
#define TIM_SCR_OC  TIM_OC_Init	


TIM_OC_InitTypeDef const 		TIM_TXA_OC_Init=
{
  TIM_OCMODE_PWM1,			/*uint32_t OCMode;        !< Specifies the TIM mode.
                               This parameter can be a value of @ref TIM_Output_Compare_and_PWM_modes */

  TIM_TXA_CC1,				/*uint32_t Pulse;         !< Specifies the pulse value to be loaded into the Capture Compare Register.
                               This parameter can be a number between Min_Data = 0x0000 and Max_Data = 0xFFFF
                               (or 0xFFEF if dithering is activated)
                               Macros __HAL_TIM_CALC_PULSE(), __HAL_TIM_CALC_PULSE_DITHER() can be used to calculate
                               Pulse value */

  TIM_OCPOLARITY_HIGH,		/*uint32_t OCPolarity;    !< Specifies the output polarity.
                               This parameter can be a value of @ref TIM_Output_Compare_Polarity */

  TIM_OCNPOLARITY_HIGH,/*uint32_t OCNPolarity;   !< Specifies the complementary output polarity.
                               This parameter can be a value of @ref TIM_Output_Compare_N_Polarity
                               @note This parameter is valid only for timer instances supporting break feature. */

  TIM_OCFAST_DISABLE,/*uint32_t OCFastMode;    !< Specifies the Fast mode state.
                               This parameter can be a value of @ref TIM_Output_Fast_State
                               @note This parameter is valid only in PWM1 and PWM2 mode. */


	TIM_OCIDLESTATE_RESET, /*uint32_t OCIdleState;   !< Specifies the TIM Output Compare pin state during Idle state.
                               This parameter can be a value of @ref TIM_Output_Compare_Idle_State
                               @note This parameter is valid only for timer instances supporting break feature. */

  TIM_OCNIDLESTATE_RESET,/*uint32_t OCNIdleState;  !< Specifies the TIM Output Compare pin state during Idle state.
                               This parameter can be a value of @ref TIM_Output_Compare_N_Idle_State
                               @note This parameter is valid only for timer instances supporting break feature. */
} ;

#define	TIM_TXA_OC	TIM_TXA_OC_Init




TIM_Base_OcGroupDef     TIM_TXA_OC_GROUP={
    4,
    {
        {
        (TIM_OC_InitTypeDef*)&TIM_TXA_OC,
        TIM_CHANNEL_1,
         },
        {
        (TIM_OC_InitTypeDef*)&TIM_TXA_OC,
        TIM_CHANNEL_2,
        },
        {
       (TIM_OC_InitTypeDef*)&TIM_TXA_OC,
        TIM_CHANNEL_3,
        },
        {
        (TIM_OC_InitTypeDef*)&TIM_TXA_OC,
        TIM_CHANNEL_4,
        },
    },    
};


#define	TIM_40us_OC	0		




TIM_Base_HandleTypeDef	const TIM_BLANKING_BaseHandleTypeDef=
{
		TIM_BLANKING,							//		TIM_TypeDef*		timx;					//TIMX选择
		(TIM_Base_InitTypeDef*)&TIM_BLANKING_Init,							//		TIM_Base_InitTypeDef* Init;		//初始化参数
		0,	//		TIM_MasterConfigTypeDef* master;	//主从模式设置
		(TIM_SlaveConfigTypeDef*)&TIM_BLANKING_SlaveConfig,//TIM_SlaveConfigTypeDef*	slave;		//从模式设置
		(TIM_OC_InitTypeDef*)&TIM_BLANKING_OC,					//		TIM_OC_InitTypeDef*		OC;					//输出设置
		TIM_BLANKING_OUTPUT_CHANNEL,										//		uint32_t 	channel;									//输出通道号
		0,										//TIM_IT_UPDATE,		//		uint32_t	it;											//中断号
		0,//TIM_FLAG_UPDATE,			//		uint32_t	it_flag;								//中断标志
		TIM_IRQn_BLANKING,					//		uint32_t  irq;										//中断请求	
		IRQ_PRIORITY_TIM_BLANKING,	//		uint32_t  priority;								//中断优先级
	
};
#define		TIM_BLANKING_Base	TIM_BLANKING_BaseHandleTypeDef



TIM_Base_HandleTypeDef	const TIM_PAN_BaseHandleTypeDef=
{
		TIM_PAN,							//		TIM_TypeDef*		timx;					//TIMX选择
		(TIM_Base_InitTypeDef*)&TIM_PAN_Init,							//		TIM_Base_InitTypeDef* Init;		//初始化参数
		0,          //(TIM_MasterConfigTypeDef*)&TIM_PAN_MasterConfig,	//		TIM_MasterConfigTypeDef* master;	//主从模式设置
		0,									//TIM_SlaveConfigTypeDef*	slave;		//从模式设置
		0,          //(TIM_OC_InitTypeDef*)&TIM_OC_Init,		//TIM_OC_InitTypeDef*		OC;					//输出设置

		TIM_PAN_OUTPUT_CHANNEL,										//		uint32_t 	channel;									//输出通道号
		0,										//TIM_IT_UPDATE,		//		uint32_t	it;											//中断号
		0,//TIM_FLAG_UPDATE,			//		uint32_t	it_flag;								//中断标志
		TIM_IRQn_PAN,					//		uint32_t  irq;										//中断请求	
		IRQ_PRIORITY_TIM_PAN,	//		uint32_t  priority;								//中断优先级
	
};
#define		TIM_PAN_Base	TIM_PAN_BaseHandleTypeDef

TIM_Base_HandleTypeDef	const TIM_TXA_BaseHandleTypeDef=
{
		TIM_TXA,							//		TIM_TypeDef*		timx;					//TIMX选择
		(TIM_Base_InitTypeDef*)&TIM_TXA_Init,							//		TIM_Base_InitTypeDef* Init;		//初始化参数
		(TIM_MasterConfigTypeDef*)&TIM_TXA_MasterConfig,	//		TIM_MasterConfigTypeDef* master;	//主从模式设置
		0,//(TIM_SlaveConfigTypeDef*)&TIM_TXA_SlaveConfig,//TIM_SlaveConfigTypeDef*	slave;		//从模式设置
		(TIM_OC_InitTypeDef*)&TIM_TXA_OC_GROUP,					//		TIM_OC_InitTypeDef*		OC;					//输出设置
		TIM_TXA_OUTPUT_CHANNEL,										//		uint32_t 	channel;									//输出通道号
		0,//TIM_IT_TXA,							//		uint32_t	it;											//中断号
		TIM_FLAG_TXA,						//		uint32_t	it_flag;								//中断标志
		0,//TIM_IRQn_TXA,						//		uint32_t  irq;										//中断请求	
		IRQ_PRIORITY_TIM_TXA,		        //		uint32_t  priority;								//中断优先级
	
};
#define		TIM_TXA_Base	TIM_TXA_BaseHandleTypeDef

TIM_Base_HandleTypeDef	const TIM_FAN_BaseHandleTypeDef=
{
		TIM_FAN,							//		TIM_TypeDef*		timx;					//TIMX选择
		(TIM_Base_InitTypeDef*)&TIM_FAN_Init,							//		TIM_Base_InitTypeDef* Init;		//初始化参数
		(TIM_MasterConfigTypeDef*)&TIM_FAN_MasterConfig,	//		TIM_MasterConfigTypeDef* master;	//主从模式设置
		0,//TIM_SlaveConfigTypeDef*	slave;		//从模式设置
		(TIM_OC_InitTypeDef*)&TIM_FAN_OC,					//		TIM_OC_InitTypeDef*		OC;					//输出设置
		TIM_FAN_OUTPUT_CHANNEL,										//		uint32_t 	channel;									//输出通道号
		0,//TIM_IT_FAN,							//		uint32_t	it;											//中断号
		0,//TIM_FLAG_FAN,						//		uint32_t	it_flag;								//中断标志
		TIM_IRQn_FAN,						//		uint32_t  irq;										//中断请求	
		IRQ_PRIORITY_TIM_FAN,		//		uint32_t  priority;								//中断优先级
	
};
#define		TIM_FAN_Base	TIM_FAN_BaseHandleTypeDef

TIM_Base_HandleTypeDef	const TIM_SCR_BaseHandleTypeDef=
{
		TIM_SCR,							//		TIM_TypeDef*		timx;					//TIMX选择
		(TIM_Base_InitTypeDef*)&TIM_SCR_Init,							//		TIM_Base_InitTypeDef* Init;		//初始化参数
		0,//(TIM_MasterConfigTypeDef*)&TIM_SCR_MasterConfig,	//		TIM_MasterConfigTypeDef* master;	//主从模式设置
		0,//TIM_SlaveConfigTypeDef*	slave;		//从模式设置
		(TIM_OC_InitTypeDef*)&TIM_SCR_OC,					//		TIM_OC_InitTypeDef*		OC;					//输出设置
		TIM_SCR_OUTPUT_CHANNEL,										//		uint32_t 	channel;									//输出通道号
		0,//TIM_IT_FAN,							//		uint32_t	it;											//中断号
		0,//TIM_FLAG_FAN,						//		uint32_t	it_flag;								//中断标志
		TIM_IRQn_SCR,						//		uint32_t  irq;										//中断请求	
		IRQ_PRIORITY_TIM_SCR,		//		uint32_t  priority;								//中断优先级
	
};

#define		TIM_SCR_Base	TIM_SCR_BaseHandleTypeDef

//TIM_Base_HandleTypeDef	const TIM_COM_BaseHandleTypeDef=
//{
//		TIM_4US,							//		TIM_TypeDef*		timx;					//TIMX选择
//		(TIM_Base_InitTypeDef*)&TIM_4us_Init,							//		TIM_Base_InitTypeDef* Init;		//初始化参数
//		(TIM_MasterConfigTypeDef*)&TIM_40us_MasterConfig,	//		TIM_MasterConfigTypeDef* master;	//主从模式设置
//		0,//TIM_SlaveConfigTypeDef*	slave;		//从模式设置
//		(TIM_OC_InitTypeDef*)0,					//		TIM_OC_InitTypeDef*		OC;					//输出设置
//		0,										//		uint32_t 	channel;									//输出通道号
//		0,										//TIM_IT_UPDATE,		//		uint32_t	it;											//中断号
//		TIM_FLAG_UPDATE,			//		uint32_t	it_flag;								//中断标志
//		TIM_IRQn_4US,					//		uint32_t  irq;										//中断请求	
//		IRQ_PRIORITY_TIM_4US,	//		uint32_t  priority;								//中断优先级
//	
//};
//#define		TIM_4US_Base	TIM_4US_BaseHandleTypeDef

__weak void	Error_Handler(void)
{
	
}	
__weak	void	API_MCU_100US_IRQHandler(void)
{

}
__weak	void	API_TIM_PAN_IRQHandler(void)
{

}
__weak	void	API_MCU_TXA_IRQHandler(void)
{

}




void	API_TIM_100US_STOP(void)
{
	__HAL_TIM_DISABLE(&htim_100us);
	
}
void	API_TIM_100US_RESET(void)
{

	__HAL_TIM_DISABLE(&htim_100us);
	__HAL_TIM_SET_COUNTER(&htim_100us,0);
	__HAL_TIM_ENABLE(&htim_100us);	
//	HAL_TIM_GenerateEvent(&htim_100us,TIM_EVENTSOURCE_UPDATE);
}

void	API_TIM_TXA_RESET(void)
{  
//      htim_TXA.Instance->SMCR=0;
	__HAL_TIM_DISABLE(&htim_TXA);
//	__HAL_TIM_SET_COUNTER(&htim_TXA,0);	

//    htim_TXA.Instance->SMCR=  API_TIM_TxaSmcrSave;
//	htim_TXA.Instance->CNT=htim_TXA.Instance->ARR-2;
//	HAL_TIM_GenerateEvent(&htim_TXA,TIM_EVENTSOURCE_UPDATE);
	__HAL_TIM_SET_COUNTER(&htim_TXA,-1);			//  这里不能用UPDATE 会导至ADC触发，但TIM没有启动
	 __HAL_TIM_CLEAR_FLAG(&htim_TXA,TIM_TXA_Base.it_flag);	
	// __HAL_TIM_ENABLE_IT(&htim_TXA,TIM_TXA_Base.it);
	
    __HAL_TIM_ENABLE(&htim_TXA);
}
void	API_TIM_TXA_STOP(void)
{

//		HAL_TIM_PWM_Stop(&htim_TXA,TIM_CHANNEL_1);
	__HAL_TIM_MOE_DISABLE(&htim_TXA);
	htim_TXA.Instance->CCER=0;
	__HAL_TIM_DISABLE(&htim_TXA);
}
void	API_TIM_100US_SET_50Hz(void)
{
	__HAL_TIM_SET_AUTORELOAD(&htim_100us,TIM_DIV200_ARR_50hz-1);
//	__HAL_TIM_SET_COMPARE(&htim_100us,TIM_ZERO_OUTPUT_CHANNEL,ZERO_ARR_50hz/2);
}	
void	API_TIM_100US_SET_60Hz(void)
{
	__HAL_TIM_SET_AUTORELOAD(&htim_100us,ZERO_ARR_60hz-1);
//	__HAL_TIM_SET_COMPARE(&htim_zero,TIM_ZERO_OUTPUT_CHANNEL,ZERO_ARR_60hz/2);
}	



void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef* htim_pwm)
{
	  if(htim_pwm->Instance==TIM1)
    {
        /* Peripheral clock enable */
        __HAL_RCC_TIM1_CLK_ENABLE();
    }		
	
	  if(htim_pwm->Instance==TIM2)
    {
        /* Peripheral clock enable */
        __HAL_RCC_TIM2_CLK_ENABLE();
    }		
	  if(htim_pwm->Instance==TIM3)
    {
        /* Peripheral clock enable */
        __HAL_RCC_TIM3_CLK_ENABLE();
    }		
	  if(htim_pwm->Instance==TIM4)
    {
        /* Peripheral clock enable */
        __HAL_RCC_TIM4_CLK_ENABLE();
    }		
    if(htim_pwm->Instance==TIM5)
    {
        /* Peripheral clock enable */
        __HAL_RCC_TIM5_CLK_ENABLE();
    }		
	
    if(htim_pwm->Instance==TIM6)
    {
        /* Peripheral clock enable */
        __HAL_RCC_TIM6_CLK_ENABLE();
    }
	
    if(htim_pwm->Instance==TIM7)
    {
        /* Peripheral clock enable */
        __HAL_RCC_TIM7_CLK_ENABLE();
    }	
	
    if(htim_pwm->Instance==TIM8)
    {
        /* Peripheral clock enable */
        __HAL_RCC_TIM8_CLK_ENABLE();
    }
    if(htim_pwm->Instance==TIM16)
    {
        /* Peripheral clock enable */
        __HAL_RCC_TIM16_CLK_ENABLE();
    }
		
		
}

void HAL_TIM_MspPostInit(TIM_HandleTypeDef* htim)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(htim->Instance==TIM1)
    {   
#if	TIM_BLANKING_TEST			//TIM1_CH4	FAULT 消隐输出设置
        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**TIM1 GPIO Configuration
        *  测试 PA11     ------> TIM1_CH4
        */
        GPIO_InitStruct.Pin = GPIO_PIN_11;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF11_TIM1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
#endif
    }
    if(htim->Instance==TIM3)
    {   
#ifdef	DEBUG_POWER_OUT			//TIM3_CH3	 检锅ADC触发源
			
			
//        __HAL_RCC_GPIOC_CLK_ENABLE();
//        /**TIM1 GPIO Configuration
//        *  测试 PA11     ------> TIM1_CH4
//        */
//        GPIO_InitStruct.Pin = GPIO_PIN_12;
//        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//        GPIO_InitStruct.Pull = GPIO_PULLUP;
//        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
//        GPIO_InitStruct.Alternate = GPIO_AF8_TIM3;
//        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
#endif
    }		
		
}



void	API_TIM_BASE_Init(TIM_HandleTypeDef* timHandle,TIM_Base_HandleTypeDef*  timBase)
{

	TIM_HandleTypeDef*	htimx;
	
	htimx=timHandle;
	htimx->Instance = timBase->timx;
	htimx->Init=(TIM_Base_InitTypeDef)*(timBase->Init);

	HAL_TIM_PWM_MspInit(htimx);	
	
	if (HAL_TIM_Base_Init(htimx) != HAL_OK)
    {
        Error_Handler();
    }
	
    if (HAL_TIM_PWM_Init(htimx) != HAL_OK)
    {
        Error_Handler();
    }

	if(timBase->master)
	{
		if (HAL_TIMEx_MasterConfigSynchronization(htimx, timBase->master) != HAL_OK)
		{
        Error_Handler();
		}
	}
	
	if(timBase->slave)
	{
		if (HAL_TIM_SlaveConfigSynchro(htimx, timBase->slave) != HAL_OK)
		{
        Error_Handler();
		}
//		__HAL_TIM_ENABLE(htimx);
	}	
	
		if(timBase->OC)
		{	
                TIM_Base_OcGroupDef* ocGroup= (TIM_Base_OcGroupDef*)timBase->OC;
            if(timBase->channel==TIM_CHANNEL_ALL)
            {

                  
                  for(uint8_t i=0;i<ocGroup->size;i++)
                  {
			        if (HAL_TIM_PWM_ConfigChannel(htimx, ocGroup->ocChannel[i].OC, ocGroup->ocChannel[i].channel) != HAL_OK)
			        {
				        Error_Handler();
			        }


                  }  
									
							if (HAL_TIM_PWM_Start(htimx, ocGroup->ocChannel[0].channel) != HAL_OK)
			        {
				    /* PWM generation Error */
				        Error_Handler();
			        }					
									
									
									
									
            }
			else
			{   //单通道
				if (HAL_TIM_PWM_ConfigChannel(htimx, timBase->OC, timBase->channel) != HAL_OK)
				{
					Error_Handler();
				}
				if (HAL_TIM_PWM_Start(htimx, timBase->channel) != HAL_OK)
				{
				/* PWM generation Error */
				Error_Handler();
				}		
            }	


		}
		else
		{	
			
			if (HAL_TIM_Base_Start(htimx) != HAL_OK)
			{
				/* PWM Generation Error */
				Error_Handler();
			}
			
		}

		
		

		
//		__HAL_TIM_DISABLE(htimx);
		if(timBase->it_flag)			//有中断请求
		{	
			__HAL_TIM_CLEAR_FLAG(htimx, timBase->it_flag);
			__HAL_TIM_ENABLE_IT(htimx,timBase->it);
	
			HAL_NVIC_SetPriority(timBase->irq, timBase->priority, 1);
			HAL_NVIC_EnableIRQ(timBase->irq);	
		}
	    HAL_TIM_MspPostInit(htimx);	
		
}	








void	API_TIM_ZERO_Init(void)
{


  TIM_MasterConfigTypeDef sMasterConfig = {0};
	TIM_HandleTypeDef*	htimx;
	
	htimx=&htim_zero;
  htimx->Instance = TIM_ZERO;
	htimx->Init=TIMx_Init;

	HAL_TIM_PWM_MspInit(htimx);	
	
    if (HAL_TIM_PWM_Init(htimx) != HAL_OK)
    {
        Error_Handler();
    }
	
	
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    
    if (HAL_TIMEx_MasterConfigSynchronization(htimx, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
	
		TIM_OC_InitTypeDef	sConfigOC;
		sConfigOC.OCMode = 		TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 		htimx->Init.Period/2;
    sConfigOC.OCPolarity = 	TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode = 	TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    if (HAL_TIM_PWM_ConfigChannel(htimx, &sConfigOC, TIM_ZERO_OUTPUT_CHANNEL) != HAL_OK)
    {
        Error_Handler();
    }
	
		if (HAL_TIM_PWM_Start(htimx, TIM_ZERO_OUTPUT_CHANNEL) != HAL_OK)
		{
        /* PWM generation Error */
        Error_Handler();
		}	
	
//	__HAL_TIM_CLEAR_FLAG(&htimx, TIM_FLAG_UPDATE);
//	__HAL_TIM_ENABLE_IT(&htimx,TIM_IT_UPDATE);
	
//    HAL_NVIC_SetPriority(TIM8_UP_IRQn, 1, 1);
//    HAL_NVIC_EnableIRQ(TIM8_UP_IRQn);	

}	


void	API_TIM_PAN_RESET(void)
{
	HAL_TIM_GenerateEvent(&htim_pan,TIM_EVENTSOURCE_UPDATE);
	__HAL_TIM_ENABLE(&htim_pan);
}	

void	API_TIM_PAN_STOP(void)
{
//	__HAL_TIM_DISABLE(&htim_pan);
	CLEAR_BIT(htim_pan.Instance->CR1,TIM_CR1_CEN);
	
}	

void 	API_TIM_BLANKING_Init(void)
{
	API_TIM_BASE_Init(&htim_BLANKING,(TIM_Base_HandleTypeDef*)&TIM_BLANKING_Base);

}

void 	API_TIM_PAN_Init(void)
{
	API_TIM_BASE_Init(&htim_pan,(TIM_Base_HandleTypeDef*)&TIM_PAN_Base);
	__HAL_TIM_ENABLE_DMA(&htim_pan,TIM_DMA_UPDATE);
	__HAL_TIM_DISABLE(&htim_pan);	
}
void 	API_TIM_TXA_Init(void)
{
	API_TIM_BASE_Init(&htim_TXA,(TIM_Base_HandleTypeDef*)&TIM_TXA_Base);
	
	__HAL_TIM_SET_COMPARE(&htim_TXA,TIM_CHANNEL_1,TIM_TXA_CC1); //第一点在初始化时以赋值，	
	__HAL_TIM_SET_COMPARE(&htim_TXA,TIM_CHANNEL_2,TIM_TXA_CC2); //第一点在初始化时以赋值，
 	__HAL_TIM_SET_COMPARE(&htim_TXA,TIM_CHANNEL_3,TIM_TXA_CC3);   	
 	__HAL_TIM_SET_COMPARE(&htim_TXA,TIM_CHANNEL_4,TIM_TXA_CC4);   
	__HAL_TIM_ENABLE_DMA(&htim_TXA,TIM_DMA_CC1|TIM_DMA_CC2|TIM_DMA_CC3|TIM_DMA_CC4);//DMA用于HRTIM值
	API_TIM_TXA_STOP();
}
void 	API_TIM_FAN_Init(void)
{
	API_TIM_BASE_Init(&htim_FAN,(TIM_Base_HandleTypeDef*)&TIM_FAN_Base);
}
void 	API_TIM_SCR_Init(void)
{
	API_TIM_BASE_Init(&htim_SCR,(TIM_Base_HandleTypeDef*)&TIM_SCR_Base);
}


void API_TIM_100US_Init(void)
{

	
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim_100us.Instance = TIM_100US;
    htim_100us.Init.Prescaler = TIM_DIV200_Prescaler;
    htim_100us.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim_100us.Init.Period = TIM_DIV200_ARR_60hz-1;
	
    htim_100us.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim_100us.Init.RepetitionCounter = 0;
    htim_100us.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    
	HAL_TIM_PWM_MspInit(&htim_100us);
	
    if (HAL_TIM_PWM_Init(&htim_100us) != HAL_OK)
    {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    
    if (HAL_TIMEx_MasterConfigSynchronization(&htim_100us, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
	
	__HAL_TIM_CLEAR_FLAG(&htim_100us, TIM_FLAG_UPDATE);
	__HAL_TIM_ENABLE_IT(&htim_100us,TIM_IT_UPDATE);
    HAL_NVIC_SetPriority(TIM6_DAC_IRQn, IRQ_PRIORITY_TIM6_DAC, 1);
    HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);	
	if (HAL_TIM_Base_Start(&htim_100us) != HAL_OK)
	{
      /* PWM Generation Error */
      Error_Handler();
	}


}
#if 0
void	API_TIM_TGO_PPG_SINGLE_Start(void)
{
	
	if (HAL_TIM_Base_Start(&htim7) != HAL_OK)
	{
      /* PWM Generation Error */
      Error_Handler();
	}
	

}	

void API_TIM_TGO_PPG_SINGLE_Init(void)				//TIM7  触发PPG单脉冲
{
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim7.Instance = TIM7;
    htim7.Init.Prescaler = 19199;
    htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim7.Init.Period = 9999;
    htim7.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim7.Init.RepetitionCounter = 0;
    htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
       
//    __HAL_RCC_TIM7_CLK_ENABLE();
    
    if (HAL_TIM_PWM_Init(&htim7) != HAL_OK)
    {
        Error_Handler();
    }  
    
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
        
    if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
	


	
}
#endif


void	API_TIM5_IRQHandler(void)
{
//    __HAL_TIM_CLEAR_FLAG(&htim_TXA, TIM_FLAG_CC1);

}	

void	API_TIM3_IRQHandler(void)
{
//    __HAL_TIM_CLEAR_FLAG(&htim_TXA, TIM_FLAG_UPDATE);

}	
void	API_TIM6_IRQHandler(void)
{

  if(__HAL_TIM_GET_FLAG(&htim_100us, TIM_FLAG_UPDATE))
  {
    __HAL_TIM_CLEAR_FLAG(&htim_100us, TIM_FLAG_UPDATE);
	
	API_MCU_100US_IRQHandler();

  } 

}	

void		API_TIM_INIT(void)
{
	
#ifdef	SimZero	
		API_TIM_ZERO_Init();			//产生50HZ过零信号
#endif
 		API_TIM_SCR_Init();	     //  总继电器控制
		API_TIM_ScrSetSpeed(70);    //13v启动   
		API_TIM_TXA_Init();
		API_TIM_PAN_Init();	

		API_TIM_FAN_Init();	

		API_TIM_BLANKING_Init();
    API_TIM_100US_Init();	        //这个最后
//		API_TIM_GCC_DELAY(100000);
//		API_TIM_GCC_DELAY(100000);
//		API_TIM_GCC_DELAY(100000);
		API_TIM_ScrSetSpeed(55);    //13v启动   
//		API_TIM_TGO_PPG_SINGLE_Init();
}	


/********************************************************************************
    FileName    :  void	GCC_DELAY(uint32_t ns)				//ns延时
    Author      :  rsl
    Version     :  V1.0.1
    Brief       :  ns的延时，1个时钟是 1/152M=6.5ns   所以最小单位是6.5ns

    Date        :  2018-10-19
    Modify      :
                   2018-10-19 创建

    Copyright (c)    Foshan XinSun Electronic Technology CO.,Ltd
********************************************************************************/



void	API_TIM_GCC_DELAY(uint32_t ns)				//ns延时
{
	 uint32_t FsysStart;

	if( ns > SysTick_LOAD_RELOAD_Msk ) return;

	FsysStart=SysTick->VAL;			//得到当前计数值
	FsysStart-=ns;							//延时的值 	

	while(SysTick->VAL>FsysStart);//Wait for count flag set

}	

/********************************************************************************
    FileName    : void 	FanSetSpeed(uint8_t t_fandiv);				//风机转速取各炉头最大值
    Author      :  rsl
    Version     :  V1.0.1
    Brief       :  风机转速

    Date        :  2018-10-19
    Modify      :
                   2018-10-19 创建

    Copyright (c)Foshan XinSun Electronic Technology CO.,Ltd
********************************************************************************/
    

void		API_TIM_FanSetSpeed(uint8_t speed)
{

		uint8_t  div=speed>>4&0xf;
		uint8_t  pre=speed&0xf;
	
		uint32_t  	min_ccr=TIM_ARR_FAN/2;			//最小转数
		uint32_t	ccr=TIM_ARR_FAN-min_ccr;
		ccr*=div;
		if(ccr!=0)
		{	
			ccr/=pre;
			__HAL_TIM_SET_COMPARE(&htim_FAN,TIM_FAN_OUTPUT_CHANNEL,ccr+min_ccr);

		}
		else
		{
			__HAL_TIM_SET_COMPARE(&htim_FAN,TIM_FAN_OUTPUT_CHANNEL,0);

		}		
		

}	



static  uint8_t  ScrDelayCnt;


void		API_TIM_ScrSetSpeed(uint8_t speed)
{

		uint8_t  div=speed;
		uint8_t  pre=100;
	
        if(div>pre)
        {
           div= pre;    
        } 

		uint32_t	ccr=TIM_ARR_SCR;
		ccr*=div;
		if(ccr!=0)
		{	
			ccr/=pre;
			__HAL_TIM_SET_COMPARE(&htim_SCR,TIM_SCR_OUTPUT_CHANNEL,ccr);

		}
		else
		{
			__HAL_TIM_SET_COMPARE(&htim_SCR,TIM_SCR_OUTPUT_CHANNEL,0);

		}		
		

}	