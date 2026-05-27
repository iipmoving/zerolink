/*********************************************************************************************
Copyright <2024> <Icore Technology (Nanjing)  Co.,Ltd>
All Rights Reserved,
Redistribution and use in source and binary forms, with or without modification, are permitted
provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this list of
conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or other materials provided
with the distribution.
3. Neither the name of the copyright holder nor the names of its contributors may be used to
endorse or promote products derived from this software without specific prior written
permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
OF THE POSSIBILITY OF SUCH DAMAGE.
**********************************************************************************************/

/*
*********************************************************************************************************
*                                              rx32g4xx
*                                           Library Function
*
*                                   Copyright 2024, RX Tech, Corp.
*                                        All Rights Reserved
*
*
* Project      : rx32g4xx
* File         : TIM_DEFINE.h
* By           : RX_DV_Team
*********************************************************************************************************
*/

#ifndef __TIM_DEFINE_H__
#define __TIM_DEFINE_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "rx32g4xx.h"


#include	"drv_tim.h"	
	
/* Exported typedef ----------------------------------------------------------*/



/* Exported define -----------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
#define PERIOD_VALUE_PP1 100
#define DUTY_VALUE_PP1 20
#define PERIOD_VALUE_PP2 200
#define DUTY_VALUE_PP2 30
#define DEADTIME_VALUE   10
#define PERIOD_VALUE_SYNCHRO    2000

/**
  * @brief synchro start łőĘĽ»Ż±í¸ń
  * @param ¶ŕ¸ö»Ą˛ąĘäłöTIMXÍ¬˛˝¶¨Ę±Ć÷łőĘĽ»Ż
  * @retval None
  */

#if 1

TIM_Base_InitTypeDef	SYNCHRO_BASE_INIT={
  
      0,	                            /*uint32_t Prescaler;         !< Specifies the prescaler value used to divide the TIM clock.
                                       This parameter can be a number between Min_Data = 0x0000 and Max_Data = 0xFFFF
                                       Macro __HAL_TIM_CALC_PSC() can be used to calculate prescaler value */
    
      TIM_COUNTERMODE_UP,	            /*uint32_t CounterMode;      !< Specifies the counter mode.
                                       This parameter can be a value of @ref TIM_Counter_Mode */
    
      PERIOD_VALUE_SYNCHRO, 	            /*uint32_t Period;            !< Specifies the period value to be loaded into the active
                                       Auto-Reload Register at the next update event.
                                       This parameter can be a number between Min_Data = 0x0000 and Max_Data = 0xFFFF
                                       (or 0xFFEF if dithering is activated)Macros __HAL_TIM_CALC_PERIOD(),
                                        __HAL_TIM_CALC_PERIOD_DITHER(),__HAL_TIM_CALC_PERIOD_BY_DELAY(),
                                        __HAL_TIM_CALC_PERIOD_DITHER_BY_DELAY()can be used to calculate Period value */
    
      TIM_CLOCKDIVISION_DIV1,          /*uint32_t ClockDivision;     !< Specifies the clock division.
                                       This parameter can be a value of @ref TIM_ClockDivision */
    
      0,							      /*uint32_t RepetitionCounter;  !< Specifies the repetition counter value. Each time the RCR downcounter
                                        reaches zero, an update event is generated and counting restarts
                                        from the RCR value (N).
                                                                                This means in PWM mode that (N+1) corresponds to:
                                            - the number of PWM periods in edge-aligned mode
                                            - the number of half PWM period in center-aligned mode
                                         GP timers: this parameter must be a number between Min_Data = 0x00 and
                                         Max_Data = 0xFF.
                                         Advanced timers: this parameter must be a number between Min_Data = 0x0000 and
                                         Max_Data = 0xFFFF. */
    
      TIM_AUTORELOAD_PRELOAD_ENABLE	    /*uint32_t AutoReloadPreload;  !< Specifies the auto-reload preload.
                                          This parameter can be a value of @ref TIM_AutoReloadPreload */
  };
    
  TIM_MasterConfigTypeDef		SYNCHRO_sMasterConfig={		
        
      TIM_TRGO_ENABLE,                    /*uint32_t  MasterOutputTrigger;   !< Trigger output (TRGO) selection
                                            This parameter can be a value of @ref TIM_Master_Mode_Selection */
      0,                                  /*uint32_t  MasterOutputTrigger2;  !< Trigger output2 (TRGO2) selection
                                            This parameter can be a value of @ref TIM_Master_Mode_Selection_2 */
      TIM_MASTERSLAVEMODE_ENABLE,         /*uint32_t  MasterSlaveMode;       !< Master/slave mode selection
                                            This parameter can be a value of @ref TIM_Master_Slave_Mode
                                            @note When the Master/slave mode is enabled, the effect of
                                            an event on the trigger input (TRGI) is delayed to allow a
                                            perfect synchronization between the current timer and its
                                            slaves (through TRGO). It is not mandatory in case of timer
                                            synchronization mode. */
  };															 
        
    TIM_SlaveConfigTypeDef SYNCHRO_sSlaveConfig = {
    
    
      TIM_SLAVEMODE_DISABLE,			    /*uint32_t  SlaveMode;         !< Slave mode selection
                                        This parameter can be a value of @ref TIM_Slave_Mode */
      TIM_TS_ITR0,										/*uint32_t  InputTrigger;      !< Input Trigger source
                                        This parameter can be a value of @ref TIM_Trigger_Selection */
      TIM_TRIGGERPOLARITY_RISING,			/*uint32_t  TriggerPolarity;   !< Input Trigger polarity
                                        This parameter can be a value of @ref TIM_Trigger_Polarity */
      TIM_TRIGGERPRESCALER_DIV1,			/*uint32_t  TriggerPrescaler;  !< Input trigger prescaler
                                        This parameter can be a value of @ref TIM_Trigger_Prescaler */
      0,															/*uint32_t  TriggerFilter;  0X0~0XF   !< Input trigger filter*/
    };	
        
        
        
        
    TIM_OC_InitTypeDef SYNCHRO_sConfigOC ={
        
      TIM_OCMODE_PWM1,			/*uint32_t OCMode;        !< Specifies the TIM mode.
                                   This parameter can be a value of @ref TIM_Output_Compare_and_PWM_modes */
    
      1000,							              /*uint32_t Pulse;         !< Specifies the pulse value to be loaded into the Capture Compare Register.
                                   This parameter can be a number between Min_Data = 0x0000 and Max_Data = 0xFFFF
                                   (or 0xFFEF if dithering is activated)
                                   Macros __HAL_TIM_CALC_PULSE(), __HAL_TIM_CALC_PULSE_DITHER() can be used to calculate
                                   Pulse value */
    
      TIM_OCPOLARITY_HIGH,					/*uint32_t OCPolarity;    !< Specifies the output polarity.
                                   This parameter can be a value of @ref TIM_Output_Compare_Polarity */
    
      TIM_OCNPOLARITY_HIGH,					/*uint32_t OCNPolarity;   !< Specifies the complementary output polarity.
                                   This parameter can be a value of @ref TIM_Output_Compare_N_Polarity
                                   @note This parameter is valid only for timer instances supporting break feature. */
    
      TIM_OCFAST_DISABLE,						/*!<uint32_t OCFastMode;     Specifies the Fast mode state.
                                   This parameter can be a value of @ref TIM_Output_Fast_State
                                   @note This parameter is valid only in PWM1 and PWM2 mode. */
    
    
      TIM_OCIDLESTATE_RESET,				/*uint32_t OCIdleState;  !< Specifies the TIM Output Compare pin state during Idle state.
                                   This parameter can be a value of @ref TIM_Output_Compare_Idle_State
                                   @note This parameter is valid only for timer instances supporting break feature. */
    
      TIM_OCNIDLESTATE_RESET,				/*uint32_t OCNIdleState;  !< Specifies the TIM Output Compare pin state during Idle state.
                                   This parameter can be a value of @ref TIM_Output_Compare_N_Idle_State
                                   @note This parameter is valid only for timer instances supporting break feature. */
    
    
    };	
    
 #if 0   
    TIMEx_BreakInputConfigTypeDef SYNCHRO_sBreakInputConfig = {
    
      TIM_BREAKINPUTSOURCE_BKIN,/* uint32_t Source;         !< Specifies the source of the timer break input.
                                    This parameter can be a value of @ref TIMEx_Break_Input_Source */
      TIM_BREAKINPUTSOURCE_DISABLE,/*uint32_t Enable;         !< Specifies whether or not the break input source is enabled.
                                    This parameter can be a value of @ref TIMEx_Break_Input_Source_Enable */
      TIM_BREAKINPUTSOURCE_POLARITY_HIGH,/*uint32_t Polarity;       !< Specifies the break input source polarity.
                                    This parameter can be a value of @ref TIMEx_Break_Input_Source_Polarity */
    
    };
    
    TIMEx_BreakInputConfigTypeDef SYNCHRO_sBreak2InputConfig = {
    
      TIM_BREAKINPUTSOURCE_BKIN,/* uint32_t Source;         !< Specifies the source of the timer break input.
                                    This parameter can be a value of @ref TIMEx_Break_Input_Source */
      TIM_BREAKINPUTSOURCE_DISABLE,/*uint32_t Enable;         !< Specifies whether or not the break input source is enabled.
                                    This parameter can be a value of @ref TIMEx_Break_Input_Source_Enable */
      TIM_BREAKINPUTSOURCE_POLARITY_HIGH,/*uint32_t Polarity;       !< Specifies the break input source polarity.
                                    This parameter can be a value of @ref TIMEx_Break_Input_Source_Polarity */
    
    };
      
    
    TIM_BreakDeadTimeConfigTypeDef SYNCHRO_sBreakDeadTimeConfig = {
    
      TIM_OSSR_DISABLE,								/*uint32_t OffStateRunMode;      !< TIM off state in run mode, This parameter can be a value of @ref TIM_OSSR_Off_State_Selection_for_Run_mode_state */
    
      TIM_OSSI_DISABLE,								/*uint32_t OffStateIDLEMode;     !< TIM off state in IDLE mode, This parameter can be a value of @ref TIM_OSSI_Off_State_Selection_for_Idle_mode_state */
    
      TIM_LOCKLEVEL_OFF,							/*uint32_t LockLevel;            !< TIM Lock level, This parameter can be a value of @ref TIM_Lock_level */
    
      0,															/*uint32_t DeadTime;             !< TIM dead Time, This parameter can be a number between Min_Data = 0x00 and Max_Data = 0xFF */
    
      TIM_BREAK_DISABLE,							/*uint32_t BreakState;           !< TIM Break State, This parameter can be a value of @ref TIM_Break_Input_enable_disable */
    
      TIM_BREAKPOLARITY_HIGH,					/*uint32_t BreakPolarity;        !< TIM Break input polarity, This parameter can be a value of @ref TIM_Break_Polarity */
    
      0,															/*uint32_t BreakFilter;          !< Specifies the break input filter.This parameter can be a number between Min_Data = 0x0 and Max_Data = 0xF */
    
      TIM_BREAK_AFMODE_INPUT,					/*uint32_t BreakAFMode;          !< Specifies the alternate function mode of the break input.This parameter can be a value of @ref TIM_Break_Input_AF_Mode */
    
      TIM_BREAK2_DISABLE,							/*uint32_t Break2State;          !< TIM Break2 State, This parameter can be a value of @ref TIM_Break2_Input_enable_disable */
    
      TIM_BREAK2POLARITY_HIGH,				/*uint32_t Break2Polarity;       !< TIM Break2 input polarity, This parameter can be a value of @ref TIM_Break2_Polarity */
    
      0,															/*uint32_t Break2Filter;         !< TIM break2 input filter.This parameter can be a number between Min_Data = 0x0 and Max_Data = 0xF */
    
      TIM_BREAK_AFMODE_INPUT,					/*uint32_t Break2AFMode;         !< Specifies the alternate function mode of the break2 input.This parameter can be a value of @ref TIM_Break2_Input_AF_Mode */
    
      TIM_AUTOMATICOUTPUT_DISABLE,		/*uint32_t AutomaticOutput;      !< TIM Automatic Output Enable state, This parameter can be a value of @ref TIM_AOE_Bit_Set_Reset */
    
    };
#endif
    
TIMsetDef	 SYNCHRO_set={
        
  
      TIM3CH,/*TIMCH_ENUM_DEF  timch;                  //ÓëTIMX¶ÔÓ¦µÄĐňşĹ*/
      1,/*uint8_t         res1;                   	   //±ŁÁô*/
      0,/*uint8_t         res2;                        //±ŁÁô*/
      0,/*uint8_t         res3;                        //±ŁÁô*/
      TIM3,						/*TIM_TypeDef*    timx;                     //TIM1,~17*/   
      TIM_CHANNEL_1,			/*int	            channel;                //ĘäłöµÄÍ¨µŔ    @arg TIM_CHANNEL_1: TIM Channel 1 selected*/
      0,						/*IRQn_Type	    irqx;                   //ÖĐ¶ĎşĹ*/
    };	
    
    
    
    
GPIO_BaseInitTypeDef SYNCHRO_GPIO_InitStruct={
        
        GPIOE,
        
        {
            GPIO_PIN_6,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_AF_PP,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PULLUP,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF2_TIM3,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
    };
    

    
    
GPIOsetGroupDef	 SYNCHRO_Gpio={
        1,
        sizeof(int)*2,
        0,
        0,  
        &SYNCHRO_GPIO_InitStruct,	/*int	channel;*/
       
};	
  
  TIMstrDef		SYNCHRO_str={
  
      &SYNCHRO_set,                  /*TIMsetDef*						timSet;                 //¶¨Ę±Ć÷˛îŇěµăÉčÖĂ*/ 
      &SYNCHRO_BASE_INIT,            /*TIM_Base_InitTypeDef*				initBase;               //¶¨Ę±Ć÷Ę±»ůÉčÖĂ*/ 
      &SYNCHRO_sMasterConfig,        /*TIM_MasterConfigTypeDef*			masterConfig;           //Ö÷Éč±¸ÉčÖĂ*/
      &SYNCHRO_sSlaveConfig,         /*TIM_SlaveConfigTypeDef*			slaveConfig;            //´ÓÉč±¸ÉčÖĂ*/ 
      &SYNCHRO_sConfigOC,            /*TIM_OC_InitTypeDef*				ocInit;					//TIMEĘäłöÉčÖĂ*/	
		0,	//      &SYNCHRO_sBreakInputConfig,    /*TIMEx_BreakInputConfigTypeDef*	breakInput;				//BREAKÔ´ÉčÖĂ*/	
		0,	//      &SYNCHRO_sBreak2InputConfig,   /*TIMEx_BreakInputConfigTypeDef*	break2Input;			//BREAK2Ô´ÉčÖĂ*/		
		0,	//&SYNCHRO_sBreakDeadTimeConfig, /*TIM_BreakDeadTimeConfigTypeDef*	breakDeadTime;		    //BREAKÉčÖĂ*/
      &SYNCHRO_Gpio,                 /*GPIOsetGroupDef*					gpioGroup;				//TIM GPIOÉčÖĂ*/
      0,													/*TIM_HandleTypeDef*			ppg									//łĚĐňŔďłőĘĽ»ŻŁ¬Ö¸ĎňĘµĚĺ*/
   };	
    
  
  
  
  
  #endif
    
    /* @brief PPG1 end łőĘĽ»Ż±í¸ń*/ 
  





/**
  * @brief PPG1 start łőĘĽ»Ż±í¸ń
  * @param None
  * @retval None
  */

#if 1

TIM_Base_InitTypeDef	PPG1_BASE_INIT={

    0,	                            /*uint32_t Prescaler;         !< Specifies the prescaler value used to divide the TIM clock.
                                     This parameter can be a number between Min_Data = 0x0000 and Max_Data = 0xFFFF
                                     Macro __HAL_TIM_CALC_PSC() can be used to calculate prescaler value */
  
    TIM_COUNTERMODE_UP,	            /*uint32_t CounterMode;      !< Specifies the counter mode.
                                     This parameter can be a value of @ref TIM_Counter_Mode */
  
    PERIOD_VALUE_PP1, 	            /*uint32_t Period;            !< Specifies the period value to be loaded into the active
                                     Auto-Reload Register at the next update event.
                                     This parameter can be a number between Min_Data = 0x0000 and Max_Data = 0xFFFF
                                     (or 0xFFEF if dithering is activated)Macros __HAL_TIM_CALC_PERIOD(),
                                      __HAL_TIM_CALC_PERIOD_DITHER(),__HAL_TIM_CALC_PERIOD_BY_DELAY(),
                                      __HAL_TIM_CALC_PERIOD_DITHER_BY_DELAY()can be used to calculate Period value */
  
    TIM_CLOCKDIVISION_DIV1,          /*uint32_t ClockDivision;     !< Specifies the clock division.
                                     This parameter can be a value of @ref TIM_ClockDivision */
  
    0,							      /*uint32_t RepetitionCounter;  !< Specifies the repetition counter value. Each time the RCR downcounter
                                      reaches zero, an update event is generated and counting restarts
                                      from the RCR value (N).
                                                                              This means in PWM mode that (N+1) corresponds to:
                                          - the number of PWM periods in edge-aligned mode
                                          - the number of half PWM period in center-aligned mode
                                       GP timers: this parameter must be a number between Min_Data = 0x00 and
                                       Max_Data = 0xFF.
                                       Advanced timers: this parameter must be a number between Min_Data = 0x0000 and
                                       Max_Data = 0xFFFF. */
  
    TIM_AUTORELOAD_PRELOAD_ENABLE	    /*uint32_t AutoReloadPreload;  !< Specifies the auto-reload preload.
                                        This parameter can be a value of @ref TIM_AutoReloadPreload */
};
  
TIM_MasterConfigTypeDef		PPG1_sMasterConfig={		
      
    TIM_TRGO_ENABLE,                    /*uint32_t  MasterOutputTrigger;   !< Trigger output (TRGO) selection
                                          This parameter can be a value of @ref TIM_Master_Mode_Selection */
    0,                                  /*uint32_t  MasterOutputTrigger2;  !< Trigger output2 (TRGO2) selection
                                          This parameter can be a value of @ref TIM_Master_Mode_Selection_2 */
    TIM_MASTERSLAVEMODE_DISABLE,         /*uint32_t  MasterSlaveMode;       !< Master/slave mode selection
                                          This parameter can be a value of @ref TIM_Master_Slave_Mode
                                          @note When the Master/slave mode is enabled, the effect of
                                          an event on the trigger input (TRGI) is delayed to allow a
                                          perfect synchronization between the current timer and its
                                          slaves (through TRGO). It is not mandatory in case of timer
                                          synchronization mode. */
};															 
      
  TIM_SlaveConfigTypeDef PPG1_sSlaveConfig = {
  
  
    TIM_SLAVEMODE_DISABLE,			    /*uint32_t  SlaveMode;         !< Slave mode selection
                                      This parameter can be a value of @ref TIM_Slave_Mode */
    TIM_TS_ITR2,										/*uint32_t  InputTrigger;      !< Input Trigger source
                                      This parameter can be a value of @ref TIM_Trigger_Selection */
    TIM_TRIGGERPOLARITY_RISING,			/*uint32_t  TriggerPolarity;   !< Input Trigger polarity
                                      This parameter can be a value of @ref TIM_Trigger_Polarity */
    TIM_TRIGGERPRESCALER_DIV1,			/*uint32_t  TriggerPrescaler;  !< Input trigger prescaler
                                      This parameter can be a value of @ref TIM_Trigger_Prescaler */
    0,															/*uint32_t  TriggerFilter;  0X0~0XF   !< Input trigger filter*/
  };	
      
      
      
      
  TIM_OC_InitTypeDef PPG1_sConfigOC ={
      
    TIM_OCMODE_PWM1,							/*uint32_t OCMode;        !< Specifies the TIM mode.
                                 This parameter can be a value of @ref TIM_Output_Compare_and_PWM_modes */
  
    DUTY_VALUE_PP2,							              /*uint32_t Pulse;         !< Specifies the pulse value to be loaded into the Capture Compare Register.
                                 This parameter can be a number between Min_Data = 0x0000 and Max_Data = 0xFFFF
                                 (or 0xFFEF if dithering is activated)
                                 Macros __HAL_TIM_CALC_PULSE(), __HAL_TIM_CALC_PULSE_DITHER() can be used to calculate
                                 Pulse value */
  
    TIM_OCPOLARITY_HIGH,					/*uint32_t OCPolarity;    !< Specifies the output polarity.
                                 This parameter can be a value of @ref TIM_Output_Compare_Polarity */
  
    TIM_OCNPOLARITY_HIGH,					/*uint32_t OCNPolarity;   !< Specifies the complementary output polarity.
                                 This parameter can be a value of @ref TIM_Output_Compare_N_Polarity
                                 @note This parameter is valid only for timer instances supporting break feature. */
  
    TIM_OCFAST_DISABLE,						/*!<uint32_t OCFastMode;     Specifies the Fast mode state.
                                 This parameter can be a value of @ref TIM_Output_Fast_State
                                 @note This parameter is valid only in PWM1 and PWM2 mode. */
  
  
     TIM_OCIDLESTATE_RESET,				/*uint32_t OCIdleState;  !< Specifies the TIM Output Compare pin state during Idle state.
                                 This parameter can be a value of @ref TIM_Output_Compare_Idle_State
                                 @note This parameter is valid only for timer instances supporting break feature. */
  
    TIM_OCNIDLESTATE_RESET,				/*uint32_t OCNIdleState;  !< Specifies the TIM Output Compare pin state during Idle state.
                                 This parameter can be a value of @ref TIM_Output_Compare_N_Idle_State
                                 @note This parameter is valid only for timer instances supporting break feature. */
  
  
  };	
  
  
  TIMEx_BreakInputConfigTypeDef PPG1_sBreakInputConfig = {
  
    TIM_BREAKINPUTSOURCE_BKIN,/* uint32_t Source;         !< Specifies the source of the timer break input.
                                  This parameter can be a value of @ref TIMEx_Break_Input_Source */
    TIM_BREAKINPUTSOURCE_ENABLE,/*uint32_t Enable;         !< Specifies whether or not the break input source is enabled.
                                  This parameter can be a value of @ref TIMEx_Break_Input_Source_Enable */
    TIM_BREAKINPUTSOURCE_POLARITY_HIGH,/*uint32_t Polarity;       !< Specifies the break input source polarity.
                                  This parameter can be a value of @ref TIMEx_Break_Input_Source_Polarity */
  
  };
  
  TIMEx_BreakInputConfigTypeDef PPG1_sBreak2InputConfig = {
  
    TIM_BREAKINPUTSOURCE_BKIN,/* uint32_t Source;         !< Specifies the source of the timer break input.
                                  This parameter can be a value of @ref TIMEx_Break_Input_Source */
    TIM_BREAKINPUTSOURCE_ENABLE,/*uint32_t Enable;         !< Specifies whether or not the break input source is enabled.
                                  This parameter can be a value of @ref TIMEx_Break_Input_Source_Enable */
    TIM_BREAKINPUTSOURCE_POLARITY_HIGH,/*uint32_t Polarity;       !< Specifies the break input source polarity.
                                  This parameter can be a value of @ref TIMEx_Break_Input_Source_Polarity */
  
  };
    
  
  TIM_BreakDeadTimeConfigTypeDef PPG1_sBreakDeadTimeConfig = {
  
    TIM_OSSR_DISABLE,								/*uint32_t OffStateRunMode;      !< TIM off state in run mode, This parameter can be a value of @ref TIM_OSSR_Off_State_Selection_for_Run_mode_state */
  
    TIM_OSSI_DISABLE,								/*uint32_t OffStateIDLEMode;     !< TIM off state in IDLE mode, This parameter can be a value of @ref TIM_OSSI_Off_State_Selection_for_Idle_mode_state */
  
    TIM_LOCKLEVEL_OFF,							/*uint32_t LockLevel;            !< TIM Lock level, This parameter can be a value of @ref TIM_Lock_level */
  
    DEADTIME_VALUE,									/*uint32_t DeadTime;             !< TIM dead Time, This parameter can be a number between Min_Data = 0x00 and Max_Data = 0xFF */
  
    TIM_BREAK_DISABLE,							/*uint32_t BreakState;           !< TIM Break State, This parameter can be a value of @ref TIM_Break_Input_enable_disable */
  
    TIM_BREAKPOLARITY_HIGH,					/*uint32_t BreakPolarity;        !< TIM Break input polarity, This parameter can be a value of @ref TIM_Break_Polarity */
  
    5,															/*uint32_t BreakFilter;          !< Specifies the break input filter.This parameter can be a number between Min_Data = 0x0 and Max_Data = 0xF */
  
    TIM_BREAK_AFMODE_INPUT,					/*uint32_t BreakAFMode;          !< Specifies the alternate function mode of the break input.This parameter can be a value of @ref TIM_Break_Input_AF_Mode */
  
    TIM_BREAK2_ENABLE,							/*uint32_t Break2State;          !< TIM Break2 State, This parameter can be a value of @ref TIM_Break2_Input_enable_disable */
  
    TIM_BREAK2POLARITY_HIGH,				/*uint32_t Break2Polarity;       !< TIM Break2 input polarity, This parameter can be a value of @ref TIM_Break2_Polarity */
  
    5,															/*uint32_t Break2Filter;         !< TIM break2 input filter.This parameter can be a number between Min_Data = 0x0 and Max_Data = 0xF */
  
    TIM_BREAK_AFMODE_INPUT,					/*uint32_t Break2AFMode;         !< Specifies the alternate function mode of the break2 input.This parameter can be a value of @ref TIM_Break2_Input_AF_Mode */
  
    TIM_AUTOMATICOUTPUT_DISABLE,		/*uint32_t AutomaticOutput;      !< TIM Automatic Output Enable state, This parameter can be a value of @ref TIM_AOE_Bit_Set_Reset */
  
  };
  
  TIMsetDef	 PPG1_set={
      

    TIM1CH,/*TIMCH_ENUM_DEF  timch;                  //ÓëTIMX¶ÔÓ¦µÄĐňşĹ*/
    0,/*uint8_t         res1;                   //±ŁÁô*/
    0,/*uint8_t         res2;                   //±ŁÁô*/
    0,/*uint8_t         res3;                   //±ŁÁô*/
    TIM1,/*TIM_TypeDef*    timx;                   //TIM1,~17*/   
		TIM_CHANNEL_3,/*int	            channel;                //ĘäłöµÄÍ¨µŔ    @arg TIM_CHANNEL_1: TIM Channel 1 selected*/
		0,/*IRQn_Type	    irqx;                   //ÖĐ¶ĎşĹ*/
  };	
  
  
  
  
GPIO_BaseInitTypeDef PPG1_GPIOL_InitStruct={
      
      GPIOF,
      
      {
          GPIO_PIN_0,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                      This parameter can be any value of @ref GPIO_pins */
  
          GPIO_MODE_AF_PP,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                      This parameter can be a value of @ref GPIO_mode */
  
          GPIO_PULLUP,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                      This parameter can be a value of @ref GPIO_pull */
  
          GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                      This parameter can be a value of @ref GPIO_speed */
  
          GPIO_AF6_TIM1,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                      This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
      }
  };
  
  GPIO_BaseInitTypeDef PPG1_GPIOH_InitStruct={
      
      GPIOF,
      
      {
          GPIO_PIN_1,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                      This parameter can be any value of @ref GPIO_pins */
  
          GPIO_MODE_AF_PP,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                      This parameter can be a value of @ref GPIO_mode */
  
          GPIO_PULLUP,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                      This parameter can be a value of @ref GPIO_pull */
  
          GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                      This parameter can be a value of @ref GPIO_speed */
  
          GPIO_AF3_TIM1,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                      This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
      }
  };
  
  
  GPIOsetGroupDef	 PPG1_Gpio={
      2,
      sizeof(int)*3,
      0,
      0,  
      &PPG1_GPIOL_InitStruct,	/*int	channel;*/
      &PPG1_GPIOH_InitStruct,   /*int	channel;*/
      
  };	

TIMstrDef		PPG1_str={

    &PPG1_set,                  /*TIMsetDef*						timSet;                 //¶¨Ę±Ć÷˛îŇěµăÉčÖĂ*/ 
    &PPG1_BASE_INIT,            /*TIM_Base_InitTypeDef*				initBase;               //¶¨Ę±Ć÷Ę±»ůÉčÖĂ*/ 
    &PPG1_sMasterConfig,        /*TIM_MasterConfigTypeDef*			masterConfig;           //Ö÷Éč±¸ÉčÖĂ*/
    &PPG1_sSlaveConfig,         /*TIM_SlaveConfigTypeDef*			slaveConfig;            //´ÓÉč±¸ÉčÖĂ*/ 
    &PPG1_sConfigOC,            /*TIM_OC_InitTypeDef*				ocInit;					//TIMEĘäłöÉčÖĂ*/	
    &PPG1_sBreakInputConfig,    /*TIMEx_BreakInputConfigTypeDef*	breakInput;				//BREAKÔ´ÉčÖĂ*/	
    &PPG1_sBreak2InputConfig,   /*TIMEx_BreakInputConfigTypeDef*	break2Input;			//BREAK2Ô´ÉčÖĂ*/
    &PPG1_sBreakDeadTimeConfig, /*TIM_BreakDeadTimeConfigTypeDef*	breakDeadTime;		    //BREAKÉčÖĂ*/
    &PPG1_Gpio,                 /*GPIOsetGroupDef*					gpioGroup;				//TIM GPIOÉčÖĂ*/
		0,													/*TIM_HandleTypeDef*			ppg									//łĚĐňŔďłőĘĽ»ŻŁ¬Ö¸ĎňĘµĚĺ*/
 };	
  




#endif
	
  /* @brief PPG1 end łőĘĽ»Ż±í¸ń*/ 


/**
  * @brief PPG2 start łőĘĽ»Ż±í¸ń
  * @param None
  * @retval None
  */

  #if 1

  TIM_Base_InitTypeDef	PPG2_BASE_INIT={
  
      0,	                            /*uint32_t Prescaler;         !< Specifies the prescaler value used to divide the TIM clock.
                                       This parameter can be a number between Min_Data = 0x0000 and Max_Data = 0xFFFF
                                       Macro __HAL_TIM_CALC_PSC() can be used to calculate prescaler value */
    
      TIM_COUNTERMODE_UP,	            /*uint32_t CounterMode;      !< Specifies the counter mode.
                                       This parameter can be a value of @ref TIM_Counter_Mode */
    
      PERIOD_VALUE_PP1, 	            /*uint32_t Period;            !< Specifies the period value to be loaded into the active
                                       Auto-Reload Register at the next update event.
                                       This parameter can be a number between Min_Data = 0x0000 and Max_Data = 0xFFFF
                                       (or 0xFFEF if dithering is activated)Macros __HAL_TIM_CALC_PERIOD(),
                                        __HAL_TIM_CALC_PERIOD_DITHER(),__HAL_TIM_CALC_PERIOD_BY_DELAY(),
                                        __HAL_TIM_CALC_PERIOD_DITHER_BY_DELAY()can be used to calculate Period value */
    
      TIM_CLOCKDIVISION_DIV1,          /*uint32_t ClockDivision;     !< Specifies the clock division.
                                       This parameter can be a value of @ref TIM_ClockDivision */
    
      0,							      /*uint32_t RepetitionCounter;  !< Specifies the repetition counter value. Each time the RCR downcounter
                                        reaches zero, an update event is generated and counting restarts
                                        from the RCR value (N).
                                                                                This means in PWM mode that (N+1) corresponds to:
                                            - the number of PWM periods in edge-aligned mode
                                            - the number of half PWM period in center-aligned mode
                                         GP timers: this parameter must be a number between Min_Data = 0x00 and
                                         Max_Data = 0xFF.
                                         Advanced timers: this parameter must be a number between Min_Data = 0x0000 and
                                         Max_Data = 0xFFFF. */
    
      TIM_AUTORELOAD_PRELOAD_ENABLE	    /*uint32_t AutoReloadPreload;  !< Specifies the auto-reload preload.
                                          This parameter can be a value of @ref TIM_AutoReloadPreload */
  };
    
  TIM_MasterConfigTypeDef		PPG2_sMasterConfig={		
        
      TIM_TRGO_ENABLE,                    /*uint32_t  MasterOutputTrigger;   !< Trigger output (TRGO) selection
                                            This parameter can be a value of @ref TIM_Master_Mode_Selection */
      0,                                  /*uint32_t  MasterOutputTrigger2;  !< Trigger output2 (TRGO2) selection
                                            This parameter can be a value of @ref TIM_Master_Mode_Selection_2 */
      TIM_MASTERSLAVEMODE_ENABLE,         /*uint32_t  MasterSlaveMode;       !< Master/slave mode selection
                                            This parameter can be a value of @ref TIM_Master_Slave_Mode
                                            @note When the Master/slave mode is enabled, the effect of
                                            an event on the trigger input (TRGI) is delayed to allow a
                                            perfect synchronization between the current timer and its
                                            slaves (through TRGO). It is not mandatory in case of timer
                                            synchronization mode. */
  };															 
        
    TIM_SlaveConfigTypeDef PPG2_sSlaveConfig = {
    
    
      TIM_SLAVEMODE_DISABLE,			    /*uint32_t  SlaveMode;         !< Slave mode selection
                                        This parameter can be a value of @ref TIM_Slave_Mode */
      TIM_TS_ITR1,										/*uint32_t  InputTrigger;      !< Input Trigger source
                                        This parameter can be a value of @ref TIM_Trigger_Selection */
      TIM_TRIGGERPOLARITY_RISING,			/*uint32_t  TriggerPolarity;   !< Input Trigger polarity
                                        This parameter can be a value of @ref TIM_Trigger_Polarity */
      TIM_TRIGGERPRESCALER_DIV1,			/*uint32_t  TriggerPrescaler;  !< Input trigger prescaler
                                        This parameter can be a value of @ref TIM_Trigger_Prescaler */
      0,															/*uint32_t  TriggerFilter;  0X0~0XF   !< Input trigger filter*/
    };	
        
        
        
        
    TIM_OC_InitTypeDef PPG2_sConfigOC ={
        
      TIM_OCMODE_PWM1,							/*uint32_t OCMode;        !< Specifies the TIM mode.
                                   This parameter can be a value of @ref TIM_Output_Compare_and_PWM_modes */
    
      DUTY_VALUE_PP2,							/*uint32_t Pulse;         !< Specifies the pulse value to be loaded into the Capture Compare Register.
                                   This parameter can be a number between Min_Data = 0x0000 and Max_Data = 0xFFFF
                                   (or 0xFFEF if dithering is activated)
                                   Macros __HAL_TIM_CALC_PULSE(), __HAL_TIM_CALC_PULSE_DITHER() can be used to calculate
                                   Pulse value */
    
      TIM_OCPOLARITY_HIGH,					/*uint32_t OCPolarity;    !< Specifies the output polarity.
                                   This parameter can be a value of @ref TIM_Output_Compare_Polarity */
    
      TIM_OCNPOLARITY_HIGH,					/*uint32_t OCNPolarity;   !< Specifies the complementary output polarity.
                                   This parameter can be a value of @ref TIM_Output_Compare_N_Polarity
                                   @note This parameter is valid only for timer instances supporting break feature. */
    
      TIM_OCFAST_DISABLE,						/*!<uint32_t OCFastMode;     Specifies the Fast mode state.
                                   This parameter can be a value of @ref TIM_Output_Fast_State
                                   @note This parameter is valid only in PWM1 and PWM2 mode. */
    
    
       TIM_OCIDLESTATE_RESET,				/*uint32_t OCIdleState;  !< Specifies the TIM Output Compare pin state during Idle state.
                                   This parameter can be a value of @ref TIM_Output_Compare_Idle_State
                                   @note This parameter is valid only for timer instances supporting break feature. */
    
      TIM_OCNIDLESTATE_RESET,				/*uint32_t OCNIdleState;  !< Specifies the TIM Output Compare pin state during Idle state.
                                   This parameter can be a value of @ref TIM_Output_Compare_N_Idle_State
                                   @note This parameter is valid only for timer instances supporting break feature. */
    
    
    };	
    
    
    TIMEx_BreakInputConfigTypeDef PPG2_sBreakInputConfig = {
    
      TIM_BREAKINPUTSOURCE_BKIN,/* uint32_t Source;         !< Specifies the source of the timer break input.
                                    This parameter can be a value of @ref TIMEx_Break_Input_Source */
      TIM_BREAKINPUTSOURCE_ENABLE,/*uint32_t Enable;         !< Specifies whether or not the break input source is enabled.
                                    This parameter can be a value of @ref TIMEx_Break_Input_Source_Enable */
      TIM_BREAKINPUTSOURCE_POLARITY_HIGH,/*uint32_t Polarity;       !< Specifies the break input source polarity.
                                    This parameter can be a value of @ref TIMEx_Break_Input_Source_Polarity */
    
    };
    
    TIMEx_BreakInputConfigTypeDef PPG2_sBreak2InputConfig = {
    
      TIM_BREAKINPUTSOURCE_BKIN,/* uint32_t Source;         !< Specifies the source of the timer break input.
                                    This parameter can be a value of @ref TIMEx_Break_Input_Source */
      TIM_BREAKINPUTSOURCE_ENABLE,/*uint32_t Enable;         !< Specifies whether or not the break input source is enabled.
                                    This parameter can be a value of @ref TIMEx_Break_Input_Source_Enable */
      TIM_BREAKINPUTSOURCE_POLARITY_HIGH,/*uint32_t Polarity;       !< Specifies the break input source polarity.
                                    This parameter can be a value of @ref TIMEx_Break_Input_Source_Polarity */
    
    };
      
    
    TIM_BreakDeadTimeConfigTypeDef PPG2_sBreakDeadTimeConfig = {
    
      TIM_OSSR_DISABLE,								/*uint32_t OffStateRunMode;      !< TIM off state in run mode, This parameter can be a value of @ref TIM_OSSR_Off_State_Selection_for_Run_mode_state */
    
      TIM_OSSI_DISABLE,								/*uint32_t OffStateIDLEMode;     !< TIM off state in IDLE mode, This parameter can be a value of @ref TIM_OSSI_Off_State_Selection_for_Idle_mode_state */
    
      TIM_LOCKLEVEL_OFF,							/*uint32_t LockLevel;            !< TIM Lock level, This parameter can be a value of @ref TIM_Lock_level */
    
      DEADTIME_VALUE,															/*uint32_t DeadTime;             !< TIM dead Time, This parameter can be a number between Min_Data = 0x00 and Max_Data = 0xFF */
    
      TIM_BREAK_DISABLE,							/*uint32_t BreakState;           !< TIM Break State, This parameter can be a value of @ref TIM_Break_Input_enable_disable */
    
      TIM_BREAKPOLARITY_HIGH,					/*uint32_t BreakPolarity;        !< TIM Break input polarity, This parameter can be a value of @ref TIM_Break_Polarity */
    
      5,															/*uint32_t BreakFilter;          !< Specifies the break input filter.This parameter can be a number between Min_Data = 0x0 and Max_Data = 0xF */
    
      TIM_BREAK_AFMODE_INPUT,					/*uint32_t BreakAFMode;          !< Specifies the alternate function mode of the break input.This parameter can be a value of @ref TIM_Break_Input_AF_Mode */
    
      TIM_BREAK2_ENABLE,							/*uint32_t Break2State;          !< TIM Break2 State, This parameter can be a value of @ref TIM_Break2_Input_enable_disable */
    
      TIM_BREAK2POLARITY_HIGH,				/*uint32_t Break2Polarity;       !< TIM Break2 input polarity, This parameter can be a value of @ref TIM_Break2_Polarity */
    
      5,															/*uint32_t Break2Filter;         !< TIM break2 input filter.This parameter can be a number between Min_Data = 0x0 and Max_Data = 0xF */
    
      TIM_BREAK_AFMODE_INPUT,					/*uint32_t Break2AFMode;         !< Specifies the alternate function mode of the break2 input.This parameter can be a value of @ref TIM_Break2_Input_AF_Mode */
    
      TIM_AUTOMATICOUTPUT_DISABLE,		/*uint32_t AutomaticOutput;      !< TIM Automatic Output Enable state, This parameter can be a value of @ref TIM_AOE_Bit_Set_Reset */
    
    };
    
    TIMsetDef	 PPG2_set={
        
  
      TIM8CH,/*TIMCH_ENUM_DEF  timch;                  //ÓëTIMX¶ÔÓ¦µÄĐňşĹ*/
      0,/*uint8_t         res1;                   //±ŁÁô*/
      0,/*uint8_t         res2;                   //±ŁÁô*/
      0,/*uint8_t         res3;                   //±ŁÁô*/
      TIM8,/*TIM_TypeDef*    timx;                   //TIM1,~17*/   
      TIM_CHANNEL_2,							/*int	            channel;                //ĘäłöµÄÍ¨µŔ    @arg TIM_CHANNEL_1: TIM Channel 1 selected*/
      0,			/*IRQn_Type	    irqx;                   //ÖĐ¶ĎşĹ*/
    };	
    
    
    
    
  GPIO_BaseInitTypeDef PPG2_GPIOL_InitStruct={
        
        GPIOC,
        
        {
            GPIO_PIN_9,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_AF_PP,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PULLUP,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF6_TIM8,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
    };
    
    GPIO_BaseInitTypeDef PPG2_GPIOH_InitStruct={
        
        GPIOA,
        
        {
            GPIO_PIN_8,									/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_AF_PP,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PULLUP,								/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		            /*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF5_TIM8,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
    };
    
    
    GPIOsetGroupDef	 PPG2_Gpio={
        2,
        sizeof(int)*3,
        0,
        0,  
        &PPG2_GPIOL_InitStruct,	/*int	channel;*/
        &PPG2_GPIOH_InitStruct,   /*int	channel;*/
        
    };	


TIMstrDef		PPG2_str={
    
    &PPG2_set,                  /*TIMsetDef*						timSet;                 //¶¨Ę±Ć÷˛îŇěµăÉčÖĂ*/ 
    &PPG2_BASE_INIT,            /*TIM_Base_InitTypeDef*				initBase;               //¶¨Ę±Ć÷Ę±»ůÉčÖĂ*/ 
    &PPG2_sMasterConfig,        /*TIM_MasterConfigTypeDef*			masterConfig;           //Ö÷Éč±¸ÉčÖĂ*/
    &PPG2_sSlaveConfig,         /*TIM_SlaveConfigTypeDef*			slaveConfig;            //´ÓÉč±¸ÉčÖĂ*/ 
    &PPG2_sConfigOC,            /*TIM_OC_InitTypeDef*				ocInit;					//TIMEĘäłöÉčÖĂ*/	
    &PPG2_sBreakInputConfig,    /*TIMEx_BreakInputConfigTypeDef*	breakInput;				//BREAKÔ´ÉčÖĂ*/	
    &PPG2_sBreak2InputConfig,   /*TIMEx_BreakInputConfigTypeDef*	break2Input;			//BREAK2Ô´ÉčÖĂ*/
    &PPG2_sBreakDeadTimeConfig, /*TIM_BreakDeadTimeConfigTypeDef*	breakDeadTime;		    //BREAKÉčÖĂ*/
    &PPG2_Gpio,                 /*GPIOsetGroupDef*					gpioGroup;				//TIM GPIOÉčÖĂ*/
	{0},													/*TIM_HandleTypeDef*			ppg									//łĚĐňŔďłőĘĽ»ŻŁ¬Ö¸ĎňĘµĚĺ*/
 };	
  	


  #endif
    /* @brief PPG2 end łőĘĽ»Ż±í¸ń*/ 
  










/* Exported function prototypes ----------------------------------------------*/
void Error_Handler(void);



#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H__ */
