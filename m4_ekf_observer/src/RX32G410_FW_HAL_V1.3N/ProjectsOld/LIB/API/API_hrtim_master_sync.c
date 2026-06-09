

#include "rx32g4xx_config_def.h"
#include "rx32g4xx_hal.h"
#include "system_init.h"
#include "system_bsp.h"


#include "API_hrtim.h"
#include "API_tim.h"
#include	"DRV_GPIO.H"


#include	"API_gpio.h"






typedef struct 
{
		uint8_t count;    /*PAN起振序号（代表检锅发生）*/
		uint8_t	ch;				/*检锅序号*/

}HrtimPanDef;

static	HrtimPanDef		hrtimPan;			//HRTIM 检锅通道


static uint32_t HrtimOutPutPinSave;			//保存以打开的OUTPIN





HRTIM_HandleTypeDef hhrtim1;
#define	HRTIM_PAN_NUM		HRTIM_TIMERINDEX_TIMER_A

typedef struct{		//炉头对应资源

	uint32_t 				HrtimCh;
	COMP_TypeDef*		COMPx;
	OPAMP_TypeDef*	OPAMPx;
	
}PotStrDef;
typedef	struct		//FAULT 设置差异项
{
	uint32_t 	num;			//序号	
	uint32_t 	Source;			//FAULT输入源
	uint32_t	Polarity;		//触发相位
}HRTIM_FALUT_DEF;	


typedef	struct				//FAULT 设置差异项
{
	uint32_t 	num;			//序号	
	uint32_t 	TIMERID;			//HRTIM_TIMERID_TIMER_F
	uint32_t 	FaultEnable;	//FAULT输入源
	uint32_t	DelayedProtectionMode;		// 延时保护模式
	uint32_t	outputPin;
}HRTIM_CFG_DEF;	


typedef	struct				//EVENT 设置差异项
{
	uint32_t 	num;			//序号	
	uint32_t 	Source;			//event输入源
	uint32_t	Polarity;		// 触发相位
	
}HRTIM_EVENT_DEF;	

typedef	struct				//EVENT 设置差异项
{
	uint32_t 	num;			//序号	
	uint32_t 	outputPin;		//	HRTIM_OUTPUT_TA1  0x00000001U


}HRTIM_OUTPUT_DEF;	












//-------------以下设置注意与通道对应-------------------------------------------------



enum{				//POT炉头对应的HRTIM INDEX 

	Pot1_TimerIndex=HRTIM_TIMERINDEX_TIMER_B,
	Pot2_TimerIndex=HRTIM_TIMERINDEX_TIMER_E,
	Pot3_TimerIndex=HRTIM_TIMERINDEX_TIMER_A,
	Pot4_TimerIndex=HRTIM_TIMERINDEX_TIMER_D,
	Test1_TimerIndex=HRTIM_TIMERINDEX_TIMER_C,		//始终保持占空比为50%的调试通道
	Test2_TimerIndex=HRTIM_TIMERINDEX_TIMER_F,		//始终保持占空比为50%的调试通道	
	
	Pot1_TIMERID=HRTIM_TIMERID_TIMER_B,
	Pot2_TIMERID=HRTIM_TIMERID_TIMER_E,	
	Pot3_TIMERID=HRTIM_TIMERID_TIMER_A,
	Pot4_TIMERID=HRTIM_TIMERID_TIMER_D,
	Test1_TIMERID=HRTIM_TIMERID_TIMER_C,
	Test2_TIMERID=HRTIM_TIMERID_TIMER_F,	
	
	
	Pot1_FaultEnable=HRTIM_TIMFAULTENABLE_FAULT4,
	Pot2_FaultEnable=HRTIM_TIMFAULTENABLE_FAULT1,	
	Pot3_FaultEnable=HRTIM_TIMFAULTENABLE_FAULT5,	
	Pot4_FaultEnable=HRTIM_TIMFAULTENABLE_FAULT2,
	Test1_FaultEnable=HRTIM_TIMFAULTENABLE_NONE,
	Test2_FaultEnable=HRTIM_TIMFAULTENABLE_NONE,
	
	Pot1_DelayP=HRTIM_TIMER_A_B_C_DELAYEDPROTECTION_DISABLED,
	Pot2_DelayP=HRTIM_TIMER_D_E_DELAYEDPROTECTION_DISABLED,	
	Pot3_DelayP=HRTIM_TIMER_A_B_C_DELAYEDPROTECTION_DISABLED,
	Pot4_DelayP=HRTIM_TIMER_D_E_DELAYEDPROTECTION_DISABLED,
	Test1_DelayP=HRTIM_TIMER_A_B_C_DELAYEDPROTECTION_DISABLED,
	Test2_DelayP=HRTIM_TIMER_F_DELAYEDPROTECTION_DISABLED,


	Pot1_OutPin=HRTIM_OUTPUT_TB1|HRTIM_OUTPUT_TB2,
	Pot2_OutPin=HRTIM_OUTPUT_TE1|HRTIM_OUTPUT_TE2,
	Pot3_OutPin=HRTIM_OUTPUT_TA1|HRTIM_OUTPUT_TA2,
	Pot4_OutPin=HRTIM_OUTPUT_TD1|HRTIM_OUTPUT_TD2,
	Test1_OutPin=0,
	Test2_OutPin=0,	
};

#if 0

HRTIM_CFG_DEF const 	HRTIM_CFG_NUM[PotMax]=
{
	{Pot1_TimerIndex,HRTIM_TIMFAULTENABLE_FAULT1|HRTIM_TIMFAULTENABLE_FAULT3,		HRTIM_TIMER_D_E_DELAYEDPROTECTION_DISABLED	,	},
	{Pot2_TimerIndex,HRTIM_TIMFAULTENABLE_FAULT4|HRTIM_TIMFAULTENABLE_FAULT3,		HRTIM_TIMER_A_B_C_DELAYEDPROTECTION_DISABLED	,	},
	{Pot3_TimerIndex,HRTIM_TIMFAULTENABLE_FAULT5|HRTIM_TIMFAULTENABLE_FAULT6,		HRTIM_TIMER_A_B_C_DELAYEDPROTECTION_DISABLED	,	},
	{Pot4_TimerIndex,HRTIM_TIMFAULTENABLE_FAULT2|HRTIM_TIMFAULTENABLE_FAULT6,		HRTIM_TIMER_D_E_DELAYEDPROTECTION_DISABLED	,	},
	{Test1_TimerIndex,HRTIM_TIMFAULTENABLE_NONE,	HRTIM_TIMER_A_B_C_DELAYEDPROTECTION_DISABLED,		},
	{Test2_TimerIndex,HRTIM_TIMFAULTENABLE_NONE,	HRTIM_TIMER_A_B_C_DELAYEDPROTECTION_DISABLED,		},

};	
#else
HRTIM_CFG_DEF const 	HRTIM_CFG_NUM[PotMax]=
{
	{Pot1_TimerIndex,Pot1_TIMERID,Pot1_FaultEnable,		Pot1_DelayP	,Pot1_OutPin,	},		//HRTIM_TIMERINDEX_TIMER_B
	{Pot2_TimerIndex,Pot2_TIMERID,Pot2_FaultEnable,		Pot2_DelayP	,	Pot2_OutPin,},			//HRTIM_TIMERINDEX_TIMER_E
	{Pot3_TimerIndex,Pot3_TIMERID,Pot3_FaultEnable,		Pot3_DelayP	,Pot3_OutPin,	},		//HRTIM_TIMERINDEX_TIMER_A
	{Pot4_TimerIndex,Pot4_TIMERID,Pot4_FaultEnable,		Pot4_DelayP	,Pot4_OutPin,	},			//HRTIM_TIMERINDEX_TIMER_D
	{Test1_TimerIndex,Test1_TIMERID,Test1_FaultEnable,	Test1_DelayP,		Test1_OutPin,},			//HRTIM_TIMERINDEX_TIMER_C
	{Test2_TimerIndex,Test2_TIMERID,Test2_FaultEnable,	Test2_DelayP,		Test2_OutPin,},				//HRTIM_TIMERINDEX_TIMER_F		
};


#endif

#if 0
HRTIM_OUTPUT_DEF	const	HRTIM_OUTPUT_NUM[PotMax]={

	{Pot1_TimerIndex,HRTIM_OUTPUT_TB1|HRTIM_OUTPUT_TB2},	//HRTIM_TIMERINDEX_TIMER_B
	{Pot2_TimerIndex,HRTIM_OUTPUT_TE1|HRTIM_OUTPUT_TE2},	//HRTIM_TIMERINDEX_TIMER_E
	{Pot3_TimerIndex,HRTIM_OUTPUT_TA1|HRTIM_OUTPUT_TA2},	//HRTIM_TIMERINDEX_TIMER_A
	{Pot4_TimerIndex,HRTIM_OUTPUT_TD1|HRTIM_OUTPUT_TD2},	//HRTIM_TIMERINDEX_TIMER_D
	{Test1_TimerIndex,0},									//HRTIM_TIMERINDEX_TIMER_C
	{Test2_TimerIndex,0},									//HRTIM_TIMERINDEX_TIMER_F
};
#endif

//--------------------------------------------------------------






PotStrDef	const PotTypeStr[PotMax]={
	
	{Pot1_TimerIndex, COMP1,OPAMP1,},			//pot1
	{Pot2_TimerIndex, COMP2,OPAMP2,},			//pot1
	{Pot3_TimerIndex, COMP3,OPAMP3,},			//pot1
	{Pot4_TimerIndex, COMP4,OPAMP4,},			//pot1	
	{Test1_TimerIndex, COMP4,OPAMP4,},			//pot1	
	{Test2_TimerIndex, COMP4,OPAMP4,},			//pot1		
};
__weak		void	Error_Handler(void)
{
}	
__weak	void	API_HRTIM1_TEST_UPD_IRQHandlerCallback(uint8_t sourse)		//HRTIM的UPD中断，开停ADC T34A EOC中断
{

}
__weak	void		API_HRTIM1_TEST_CMP1_IRQHandlerCallback(void)
{
	
}
__weak	void	API_HRTIM_PanOffCallBack(void)
{}	
void		DEBUG_PORT_INIT(void);
void		API_SET_OutputCfgTypeDef(uint8_t ch, HRTIM_OutputCfgTypeDef* str);

void	API_HRTIM_PAN_CLEAR_FLAG(uint32_t ch)
{	
	__HAL_HRTIM_TIMER_CLEAR_FLAG(&hhrtim1,HRTIM_CFG_NUM[ch].num,HRTIM1_PAN_ICR);
}



/**
* @brief HRTIM MSP Initialization
* This function configures the hardware resources used in this example
* @param hhrtim: HRTIM handle pointer
* @retval None
*/
void HAL_HRTIM_MspInit(HRTIM_HandleTypeDef* hhrtim)
{
  if(hhrtim->Instance==HRTIM1)
  {
  /* USER CODE BEGIN HRTIM1_MspInit 0 */

  /* USER CODE END HRTIM1_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_HRTIM1_CLK_ENABLE();
  /* USER CODE BEGIN HRTIM1_MspInit 1 */

  /* USER CODE END HRTIM1_MspInit 1 */
  }

}




void HAL_HRTIM_MspPostInit(HRTIM_HandleTypeDef* hhrtim)
{
	

}





HRTIM_TimeBaseCfgTypeDef	const  PPGTimeBaseCfg = {

 TIMD_PERIOD,						/*uint32_t Period;                   !< Specifies the timer period.
                                          The period value must be above 3 periods of the fHRTIM clock.
                                          Maximum value is = 0xFFDFU */
 0,							/* uint32_t RepetitionCounter;        !< Specifies the timer repetition period.
                                          This parameter must be a number between Min_Data = 0x00 and Max_Data = 0xFF. */
 HRTIM_PRESCALERRATIO_MUL4,/* uint32_t PrescalerRatio;           !< Specifies the timer clock prescaler ratio.
                                          This parameter can be any value of @ref HRTIM_Prescaler_Ratio   */
 HRTIM_MODE_CONTINUOUS,		/* uint32_t Mode;                     !< Specifies the counter operating mode.
                                          This parameter can be any value of @ref HRTIM_Counter_Operating_Mode   */
};

//单次模式
HRTIM_TimeBaseCfgTypeDef	const  PPGSingleTimeBaseCfg = {			


 TIMD_PERIOD,						/*uint32_t Period;                   !< Specifies the timer period.
                                          The period value must be above 3 periods of the fHRTIM clock.
                                          Maximum value is = 0xFFDFU */
 0,							/* uint32_t RepetitionCounter;        !< Specifies the timer repetition period.
                                          This parameter must be a number between Min_Data = 0x00 and Max_Data = 0xFF. */
 HRTIM_PRESCALERRATIO_MUL16,/* uint32_t PrescalerRatio;           !< Specifies the timer clock prescaler ratio.
                                          This parameter can be any value of @ref HRTIM_Prescaler_Ratio   */
 HRTIM_MODE_SINGLESHOT,		/* uint32_t Mode;                     !< Specifies the counter operating mode.
                                          This parameter can be any value of @ref HRTIM_Counter_Operating_Mode   */
};

		


HRTIM_TimerCtlTypeDef PPGTimerCtl = {

  HRTIM_TIMERUPDOWNMODE_UP,		/*uint32_t UpDownMode;            !<  Relevant for Timer A to Timer F.
                                        Specifies whether or not counter is operating in up or up-down counting mode.
                                        This parameter can be a value of @ref HRTIM_Timer_UpDown_Mode */
  HRTIM_TIMERTRIGHALF_DISABLED,	/*uint32_t TrigHalf;              !<  Relevant for Timer A to Timer F.
                                        Specifies whether or not compare 2 is operating in Trigger half mode.
                                        This parameter can be a value of @ref HRTIM_Timer_TrigHalf_Mode */
  HRTIM_TIMERGTCMP3_EQUAL,		/*uint32_t GreaterCMP3;           !<  Relevant for Timer A to Timer F.
                                        Specifies whether or not compare 3 is operating in compare match or greater mode.
                                        This parameter can be a value of @ref HRTIM_Timer_GreaterCMP3_Mode */
  HRTIM_TIMERGTCMP1_EQUAL,		/*uint32_t GreaterCMP1;           !<  Relevant for Timer A to Timer F.
                                        Specifies whether or not compare 1 is operating in compare match or greater mode.
                                        This parameter can be a value of @ref HRTIM_Timer_GreaterCMP1_Mode */
  HRTIM_TIMER_DCDR_COUNTER,		/*uint32_t DualChannelDacReset;   !<  Relevant for Timer A to Timer F.
                                        Specifies how the hrtim_dac_reset_trgx trigger is generated.
                                        This parameter can be a value of @ref HRTIM_Timer_DualChannelDac_Reset */
  HRTIM_TIMER_DCDS_CMP2,		/*uint32_t DualChannelDacStep;    !<  Relevant for Timer A to Timer F.
                                        Specifies how the hrtim_dac_step_trgx trigger is generated.
                                        This parameter can be a value of @ref HRTIM_Timer_DualChannelDac_Step */
  HRTIM_TIMER_DCDE_DISABLED,	/*uint32_t DualChannelDacEnable;  !<  Relevant for Timer A to Timer F.
                                        Enables or not the dual channel DAC triggering mechanism.
                                        This parameter can be a value of @ref HRTIM_Timer_DualChannelDac_Enable */


};
HRTIM_TimerCfgTypeDef const PPGTimerCfg = {


  HRTIM_TIM_IT_NONE,			/*uint32_t InterruptRequests;      !< Relevant for all HRTIM timers, including the master.
                                       Specifies which interrupts requests must enabled for the timer.
                                       This parameter can be any combination of  @ref HRTIM_Master_Interrupt_Enable
                                       or @ref HRTIM_Timing_Unit_Interrupt_Enable */
  HRTIM_TIM_DMA_NONE,			    /*uint32_t DMARequests;            !< Relevant for all HRTIM timers, including the master.
                                       Specifies which DMA requests must be enabled for the timer.
                                       This parameter can be any combination of  @ref HRTIM_Master_DMA_Request_Enable
                                       or @ref HRTIM_Timing_Unit_DMA_Request_Enable */
  0x400200c,							    /*uint32_t DMASrcAddress;          !< Relevant for all HRTIM timers, including the master.
                                       Specifies the address of the source address of the DMA transfer */
  0x2003df0,							    /*uint32_t DMADstAddress;          !< Relevant for all HRTIM timers, including the master.
                                       Specifies the address of the destination address of the DMA transfer */
  20,							            /*uint32_t DMASize;                !< Relevant for all HRTIM timers, including the master.
                                       Specifies the size of the DMA transfer */
  HRTIM_HALFMODE_DISABLED,		/*uint32_t HalfModeEnable;         !< Relevant for all HRTIM timers, including the master.
                                        Specifies whether or not half mode is enabled
                                        This parameter can be any value of @ref HRTIM_Half_Mode_Enable  */
  HRTIM_INTERLEAVED_MODE_DISABLED,/*uint32_t InterleavedMode;         !< Relevant for all HRTIM timers, including the master.
                                        Specifies whether or not half mode is enabled
                                        This parameter can be any value of @ref HRTIM_Interleaved_Mode  */
  HRTIM_SYNCSTART_ENABLED,		/*uint32_t StartOnSync;            !< Relevant for all HRTIM timers, including the master.
                                       Specifies whether or not timer is reset by a rising edge on the synchronization input (when enabled).
                                        This parameter can be any value of @ref HRTIM_Start_On_Sync_Input_Event  */
  HRTIM_SYNCRESET_DISABLED,		/*uint32_t ResetOnSync;            !< Relevant for all HRTIM timers, including the master.
                                        Specifies whether or not timer is reset by a rising edge on the synchronization input (when enabled).
                                        This parameter can be any value of @ref HRTIM_Reset_On_Sync_Input_Event  */
  HRTIM_DACSYNC_NONE,			/*uint32_t DACSynchro;             !< Relevant for all HRTIM timers, including the master.
                                        Indicates whether or not the a DAC synchronization event is generated.
                                        This parameter can be any value of @ref HRTIM_DAC_Synchronization   */
  HRTIM_PRELOAD_ENABLED,		/*uint32_t PreloadEnable;          !< Relevant for all HRTIM timers, including the master.
                                        Specifies whether or not register preload is enabled.
                                        This parameter can be any value of @ref HRTIM_Register_Preload_Enable  */
  HRTIM_UPDATEGATING_INDEPENDENT,/*uint32_t UpdateGating;           !< Relevant for all HRTIM timers, including the master.
                                        Specifies how the update occurs with respect to a burst DMA transaction or
                                        update enable inputs (Slave timers only).
                                        This parameter can be any value of @ref HRTIM_Update_Gating   */
  HRTIM_TIMERBURSTMODE_MAINTAINCLOCK,/*uint32_t BurstMode;              !< Relevant for all HRTIM timers, including the master.
                                        Specifies how the timer behaves during a burst mode operation.
                                        This parameter can be any value of @ref HRTIM_Timer_Burst_Mode  */
  HRTIM_UPDATEONREPETITION_ENABLED,/*uint32_t RepetitionUpdate;       !< Relevant for all HRTIM timers, including the master.
                                        Specifies whether or not registers update is triggered by the repetition event.
                                        This parameter can be any value of @ref HRTIM_Timer_Repetition_Update */
  HRTIM_TIMPUSHPULLMODE_DISABLED,/*uint32_t PushPull;               !< Relevant for Timer A to Timer F.
                                        Specifies whether or not the push-pull mode is enabled.
                                        This parameter can be any value of @ref HRTIM_Timer_Push_Pull_Mode */
  HRTIM_TIMFAULTENABLE_FAULT4|HRTIM_TIMFAULTENABLE_FAULT3,/*uint32_t FaultEnable;            !< Relevant for Timer A to Timer F.
                                        Specifies which fault channels are enabled for the timer.
                                        This parameter can be a combination of @ref HRTIM_Timer_Fault_Enabling  */
  HRTIM_TIMFAULTLOCK_READWRITE,	/*uint32_t FaultLock;              !< Relevant for Timer A to Timer F.
                                        Specifies whether or not fault enabling status is write protected.
                                        This parameter can be a value of @ref HRTIM_Timer_Fault_Lock */
  HRTIM_TIMDEADTIMEINSERTION_ENABLED,/*uint32_t DeadTimeInsertion;      !< Relevant for Timer A to Timer F.
                                        Specifies whether or not dead-time insertion is enabled for the timer.
                                        This parameter can be a value of @ref HRTIM_Timer_Deadtime_Insertion */
  HRTIM_TIMER_A_B_C_DELAYEDPROTECTION_DISABLED,/*uint32_t DelayedProtectionMode;  !< Relevant for Timer A to Timer F.
                                        Specifies the delayed protection mode.
                                        This parameter can be a value of @ref HRTIM_Timer_Delayed_Protection_Mode */
  HRTIM_OUTPUTBIAR_DISABLED,/*uint32_t BalancedIdleAutomaticResume; !< Indicates whether or not outputs are automatically re-enabled after a balanced idle event.
                                             This parameters can be any value of @ref HRTIM_Output_Balanced_Idle_Auto_Resume */
  HRTIM_TIMUPDATETRIGGER_NONE,/*uint32_t UpdateTrigger;          !< Relevant for Timer A to Timer F.
                                        Specifies source(s) triggering the timer registers update.
                                        This parameter can be a combination of @ref HRTIM_Timer_Update_Trigger */
  HRTIM_TIMRESETTRIGGER_NONE,/*uint32_t ResetTrigger;           !< Relevant for Timer A to Timer F.
                                        Specifies source(s) triggering the timer counter reset.
                                        This parameter can be a combination of @ref HRTIM_Timer_Reset_Trigger */
  HRTIM_TIMUPDATEONRESET_DISABLED,/*uint32_t ResetUpdate;           !<  Relevant for Timer A to Timer F.
                                        Specifies whether or not registers update is triggered when the timer counter is reset.
                                        This parameter can be a value of @ref HRTIM_Timer_Reset_Update */
  HRTIM_TIMERESYNC_UPDATE_UNCONDITIONAL, /*uint32_t ReSyncUpdate;          !<  Relevant for Timer A to Timer F.
                                        Specifies whether update source is coming from the timing unit @ref HRTIM_Timer_ReSyncUpdate */

};



#if 0
HRTIM_TimerCtlTypeDef PPGSingleTimerCtl = {

  HRTIM_TIMERUPDOWNMODE_UP,		/*uint32_t UpDownMode;            !<  Relevant for Timer A to Timer F.
                                        Specifies whether or not counter is operating in up or up-down counting mode.
                                        This parameter can be a value of @ref HRTIM_Timer_UpDown_Mode */
  HRTIM_TIMERTRIGHALF_ENABLED,	/*uint32_t TrigHalf;              !<  Relevant for Timer A to Timer F.
                                        Specifies whether or not compare 2 is operating in Trigger half mode.
                                        This parameter can be a value of @ref HRTIM_Timer_TrigHalf_Mode */
  HRTIM_TIMERGTCMP3_EQUAL,		/*uint32_t GreaterCMP3;           !<  Relevant for Timer A to Timer F.
                                        Specifies whether or not compare 3 is operating in compare match or greater mode.
                                        This parameter can be a value of @ref HRTIM_Timer_GreaterCMP3_Mode */
  HRTIM_TIMERGTCMP1_EQUAL,		/*uint32_t GreaterCMP1;           !<  Relevant for Timer A to Timer F.
                                        Specifies whether or not compare 1 is operating in compare match or greater mode.
                                        This parameter can be a value of @ref HRTIM_Timer_GreaterCMP1_Mode */
  HRTIM_TIMER_DCDR_COUNTER,		/*uint32_t DualChannelDacReset;   !<  Relevant for Timer A to Timer F.
                                        Specifies how the hrtim_dac_reset_trgx trigger is generated.
                                        This parameter can be a value of @ref HRTIM_Timer_DualChannelDac_Reset */
  HRTIM_TIMER_DCDS_CMP2,		/*uint32_t DualChannelDacStep;    !<  Relevant for Timer A to Timer F.
                                        Specifies how the hrtim_dac_step_trgx trigger is generated.
                                        This parameter can be a value of @ref HRTIM_Timer_DualChannelDac_Step */
  HRTIM_TIMER_DCDE_DISABLED,	/*uint32_t DualChannelDacEnable;  !<  Relevant for Timer A to Timer F.
                                        Enables or not the dual channel DAC triggering mechanism.
                                        This parameter can be a value of @ref HRTIM_Timer_DualChannelDac_Enable */


};




HRTIM_TimerCfgTypeDef const PPGSingleTimerCfg = {


  HRTIM_TIM_IT_UPD,			/*uint32_t InterruptRequests;      !< Relevant for all HRTIM timers, including the master.
                                       Specifies which interrupts requests must enabled for the timer.
                                       This parameter can be any combination of  @ref HRTIM_Master_Interrupt_Enable
                                       or @ref HRTIM_Timing_Unit_Interrupt_Enable */
  HRTIM_TIM_DMA_CPT1,			/*uint32_t DMARequests;            !< Relevant for all HRTIM timers, including the master.
                                       Specifies which DMA requests must be enabled for the timer.
                                       This parameter can be any combination of  @ref HRTIM_Master_DMA_Request_Enable
                                       or @ref HRTIM_Timing_Unit_DMA_Request_Enable */
  0,							/*uint32_t DMASrcAddress;          !< Relevant for all HRTIM timers, including the master.
                                       Specifies the address of the source address of the DMA transfer */
  0,							/*uint32_t DMADstAddress;          !< Relevant for all HRTIM timers, including the master.
                                       Specifies the address of the destination address of the DMA transfer */
  0,							/*uint32_t DMASize;                !< Relevant for all HRTIM timers, including the master.
                                       Specifies the size of the DMA transfer */
  HRTIM_HALFMODE_ENABLED,		/*uint32_t HalfModeEnable;         !< Relevant for all HRTIM timers, including the master.
                                        Specifies whether or not half mode is enabled
                                        This parameter can be any value of @ref HRTIM_Half_Mode_Enable  */
  HRTIM_INTERLEAVED_MODE_DISABLED,/*uint32_t InterleavedMode;         !< Relevant for all HRTIM timers, including the master.
                                        Specifies whether or not half mode is enabled
                                        This parameter can be any value of @ref HRTIM_Interleaved_Mode  */
  HRTIM_SYNCSTART_DISABLED,		/*uint32_t StartOnSync;            !< Relevant for all HRTIM timers, including the master.
                                       Specifies whether or not timer is reset by a rising edge on the synchronization input (when enabled).
                                        This parameter can be any value of @ref HRTIM_Start_On_Sync_Input_Event  */
  HRTIM_SYNCRESET_DISABLED,		/*uint32_t ResetOnSync;            !< Relevant for all HRTIM timers, including the master.
                                        Specifies whether or not timer is reset by a rising edge on the synchronization input (when enabled).
                                        This parameter can be any value of @ref HRTIM_Reset_On_Sync_Input_Event  */
  HRTIM_DACSYNC_NONE,			/*uint32_t DACSynchro;             !< Relevant for all HRTIM timers, including the master.
                                        Indicates whether or not the a DAC synchronization event is generated.
                                        This parameter can be any value of @ref HRTIM_DAC_Synchronization   */
  HRTIM_PRELOAD_DISABLED,		/*uint32_t PreloadEnable;          !< Relevant for all HRTIM timers, including the master.
                                        Specifies whether or not register preload is enabled.
                                        This parameter can be any value of @ref HRTIM_Register_Preload_Enable  */
  HRTIM_UPDATEGATING_INDEPENDENT,/*uint32_t UpdateGating;           !< Relevant for all HRTIM timers, including the master.
                                        Specifies how the update occurs with respect to a burst DMA transaction or
                                        update enable inputs (Slave timers only).
                                        This parameter can be any value of @ref HRTIM_Update_Gating   */
  HRTIM_TIMERBURSTMODE_MAINTAINCLOCK,/*uint32_t BurstMode;              !< Relevant for all HRTIM timers, including the master.
                                        Specifies how the timer behaves during a burst mode operation.
                                        This parameter can be any value of @ref HRTIM_Timer_Burst_Mode  */
  HRTIM_UPDATEONREPETITION_ENABLED,/*uint32_t RepetitionUpdate;       !< Relevant for all HRTIM timers, including the master.
                                        Specifies whether or not registers update is triggered by the repetition event.
                                        This parameter can be any value of @ref HRTIM_Timer_Repetition_Update */
  HRTIM_TIMPUSHPULLMODE_DISABLED,/*uint32_t PushPull;               !< Relevant for Timer A to Timer F.
                                        Specifies whether or not the push-pull mode is enabled.
                                        This parameter can be any value of @ref HRTIM_Timer_Push_Pull_Mode */
  HRTIM_TIMFAULTENABLE_NONE,		/*uint32_t FaultEnable;            !< Relevant for Timer A to Timer F.
                                        Specifies which fault channels are enabled for the timer.
                                        This parameter can be a combination of @ref HRTIM_Timer_Fault_Enabling  */
  HRTIM_TIMFAULTLOCK_READWRITE,	/*uint32_t FaultLock;              !< Relevant for Timer A to Timer F.
                                        Specifies whether or not fault enabling status is write protected.
                                        This parameter can be a value of @ref HRTIM_Timer_Fault_Lock */
  HRTIM_TIMDEADTIMEINSERTION_DISABLED,/*uint32_t DeadTimeInsertion;      !< Relevant for Timer A to Timer F.
                                        Specifies whether or not dead-time insertion is enabled for the timer.
                                        This parameter can be a value of @ref HRTIM_Timer_Deadtime_Insertion */
  HRTIM_TIMER_A_B_C_DELAYEDPROTECTION_DISABLED,/*uint32_t DelayedProtectionMode;  !< Relevant for Timer A to Timer F.
                                        Specifies the delayed protection mode.
                                        This parameter can be a value of @ref HRTIM_Timer_Delayed_Protection_Mode */
  HRTIM_OUTPUTBIAR_DISABLED,/*uint32_t BalancedIdleAutomaticResume; !< Indicates whether or not outputs are automatically re-enabled after a balanced idle event.
                                             This parameters can be any value of @ref HRTIM_Output_Balanced_Idle_Auto_Resume */
  HRTIM_TIMUPDATETRIGGER_NONE,/*uint32_t UpdateTrigger;          !< Relevant for Timer A to Timer F.
                                        Specifies source(s) triggering the timer registers update.
                                        This parameter can be a combination of @ref HRTIM_Timer_Update_Trigger */
  HRTIM_TIMRESETTRIGGER_EEV_7,/*uint32_t ResetTrigger;           !< Relevant for Timer A to Timer F.
                                        Specifies source(s) triggering the timer counter reset.
                                        This parameter can be a combination of @ref HRTIM_Timer_Reset_Trigger */
  HRTIM_TIMUPDATEONRESET_DISABLED,/*uint32_t ResetUpdate;           !<  Relevant for Timer A to Timer F.
                                        Specifies whether or not registers update is triggered when the timer counter is reset.
                                        This parameter can be a value of @ref HRTIM_Timer_Reset_Update */
  HRTIM_TIMERESYNC_UPDATE_UNCONDITIONAL, /*uint32_t ReSyncUpdate;          !<  Relevant for Timer A to Timer F.
                                        Specifies whether update source is coming from the timing unit @ref HRTIM_Timer_ReSyncUpdate */

};

#endif


HRTIM_CompareCfgTypeDef PPGCompareCfg = {

  TIMD_PERIOD*2/5,/*uint32_t CompareValue;         !< Specifies the compare value of the timer compare unit.
                                      The minimum value must be greater than or equal to 3 periods of the fHRTIM clock.
                                      The maximum value must be less than or equal to 0xFFFFU - 1 periods of the fHRTIM clock */
  HRTIM_AUTODELAYEDMODE_REGULAR,		/*uint32_t AutoDelayedMode;      !< Specifies the auto delayed mode for compare unit 2 or 4.
                                      This parameter can be a value of @ref HRTIM_Compare_Unit_Auto_Delayed_Mode */
  0,									/*uint32_t AutoDelayedTimeout;   !< Specifies compare value for timing unit 1 or 3 when auto delayed mode with time out is selected.*/
 

};




HRTIM_OutputCfgTypeDef PPGOutputCfg = {

  HRTIM_OUTPUTPOLARITY_HIGH,/*uint32_t Polarity;                    !< Specifies the output polarity.
                                             This parameter can be any value of @ref HRTIM_Output_Polarity */
  OUTPUTSETCHN,/*uint32_t SetSource;                   !< Specifies the event(s) transitioning the output from its inactive level to its active level.
                                             This parameter can be a combination of @ref HRTIM_Output_Set_Source */
  OUTPUTRESETCHN,/*uint32_t ResetSource;                 !< Specifies the event(s) transitioning the output from its active level to its inactive level.
                                             This parameter can be a combination of @ref HRTIM_Output_Reset_Source */
  HRTIM_OUTPUTIDLEMODE_NONE,/*uint32_t IdleMode;                    !< Specifies whether or not the output is affected by a burst mode operation.
                                             This parameter can be any value of @ref HRTIM_Output_Idle_Mode */
  HRTIM_OUTPUTIDLELEVEL_INACTIVE,/*uint32_t IdleLevel;                   !< Specifies whether the output level is active or inactive when in IDLE state.
                                             This parameter can be any value of @ref HRTIM_Output_IDLE_Level */
  HRTIM_OUTPUTFAULTLEVEL_INACTIVE,/*uint32_t FaultLevel;                  !< Specifies whether the output level is active or inactive when in FAULT state.
                                             This parameter can be any value of @ref HRTIM_Output_FAULT_Level */
  HRTIM_OUTPUTCHOPPERMODE_DISABLED,/*uint32_t ChopperModeEnable;           !< Indicates whether or not the chopper mode is enabled
                                             This parameter can be any value of @ref HRTIM_Output_Chopper_Mode_Enable */
  HRTIM_OUTPUTBURSTMODEENTRY_REGULAR,/*uint32_t BurstModeEntryDelayed;       !< Indicates whether or not dead-time is inserted when entering the IDLE state during a burst mode operation.
                                             This parameters can be any value of @ref HRTIM_Output_Burst_Mode_Entry_Delayed */

};


#if 0

HRTIM_OutputCfgTypeDef PPGSingleOutputCfg = {

  HRTIM_OUTPUTPOLARITY_HIGH,			/*uint32_t Polarity;                    !< Specifies the output polarity.
                                             This parameter can be any value of @ref HRTIM_Output_Polarity */
  HRTIM_OUTPUTSET_TIMPER,			/*uint32_t SetSource;                   !< Specifies the event(s) transitioning the output from its inactive level to its active level.
                                             This parameter can be a combination of @ref HRTIM_Output_Set_Source */
  HRTIM_OUTPUTRESET_TIMCMP1,			/*uint32_t ResetSource;                 !< Specifies the event(s) transitioning the output from its active level to its inactive level.
                                             This parameter can be a combination of @ref HRTIM_Output_Reset_Source */
  HRTIM_OUTPUTIDLEMODE_IDLE,			/*uint32_t IdleMode;                    !< Specifies whether or not the output is affected by a burst mode operation.
                                             This parameter can be any value of @ref HRTIM_Output_Idle_Mode */
  HRTIM_OUTPUTIDLELEVEL_ACTIVE,		/*uint32_t IdleLevel;                   !< Specifies whether the output level is active or inactive when in IDLE state.
                                             This parameter can be any value of @ref HRTIM_Output_IDLE_Level */
  HRTIM_OUTPUTFAULTLEVEL_INACTIVE,		/*uint32_t FaultLevel;                  !< Specifies whether the output level is active or inactive when in FAULT state.
                                             This parameter can be any value of @ref HRTIM_Output_FAULT_Level */
  HRTIM_OUTPUTCHOPPERMODE_DISABLED,		/*uint32_t ChopperModeEnable;           !< Indicates whether or not the chopper mode is enabled
                                             This parameter can be any value of @ref HRTIM_Output_Chopper_Mode_Enable */
  HRTIM_OUTPUTBURSTMODEENTRY_REGULAR,	/*uint32_t BurstModeEntryDelayed;       !< Indicates whether or not dead-time is inserted when entering the IDLE state during a burst mode operation.
                                             This parameters can be any value of @ref HRTIM_Output_Burst_Mode_Entry_Delayed */

};
#endif

//-----------------------
//DIV1  1.3
//DIV2		
//DIV4	0XFF 5.3us 48 1us   

HRTIM_DeadTimeCfgTypeDef PPGDeadTimeCfg={

  HRTIM_TIMDEADTIME_PRESCALERRATIO_DIV4,/*uint32_t Prescaler;        !< Specifies the dead-time prescaler.
                                  This parameter can be a value of @ref  HRTIM_Deadtime_Prescaler_Ratio */
  0,/*uint32_t RisingValue;      !< Specifies the dead-time following a rising edge.
                                  This parameter can be a number between 0x0 and 0x1FFU */
  HRTIM_TIMDEADTIME_RISINGSIGN_POSITIVE,/*uint32_t RisingSign;       !< Specifies whether the dead-time is positive or negative on rising edge.
                                  This parameter can be a value of @ref HRTIM_Deadtime_Rising_Sign */
  HRTIM_TIMDEADTIME_RISINGLOCK_WRITE,/*uint32_t RisingLock;       !< Specifies whether or not dead-time rising settings (value and sign) are write protected.
                                  This parameter can be a value of @ref HRTIM_Deadtime_Rising_Lock */
  HRTIM_TIMDEADTIME_RISINGSIGNLOCK_WRITE,/*uint32_t RisingSignLock;   !< Specifies whether or not dead-time rising sign is write protected.
                                  This parameter can be a value of @ref HRTIM_Deadtime_Rising_Sign_Lock */
  0,	/*uint32_t FallingValue;     !< Specifies the dead-time following a falling edge.
                                  This parameter can be a number between 0x0 and 0x1FFU */
  HRTIM_TIMDEADTIME_FALLINGSIGN_POSITIVE,/*uint32_t FallingSign;      !< Specifies whether the dead-time is positive or negative on falling edge.
                                  This parameter can be a value of @ref HRTIM_Deadtime_Falling_Sign */
	HRTIM_TIMDEADTIME_FALLINGLOCK_WRITE,/*uint32_t FallingLock;      !< Specifies whether or not dead-time falling settings (value and sign) are write protected.
                                  This parameter can be a value of @ref HRTIM_Deadtime_Falling_Lock */
  HRTIM_TIMDEADTIME_FALLINGSIGNLOCK_WRITE,/*uint32_t FallingSignLock;  !< Specifies whether or not dead-time falling sign is write protected.
                                  This parameter can be a value of @ref HRTIM_Deadtime_Falling_Sign_Lock */


};


HRTIM_EventCfgTypeDef PPGEventCfg={


  HRTIM_EEV6SRC_COMP1_OUT,				/*uint32_t Source;        !< Identifies the source of the external event.
											This parameter can be a value of @ref HRTIM_External_Event_Sources */
  HRTIM_EVENTPOLARITY_HIGH,				/*uint32_t Polarity;      !< Specifies the polarity of the external event (in case of level sensitivity).
											This parameter can be a value of @ref HRTIM_External_Event_Polarity */
  HRTIM_EVENTSENSITIVITY_RISINGEDGE,	/*uint32_t Sensitivity;   !< Specifies the sensitivity of the external event.
											This parameter can be a value of @ref HRTIM_External_Event_Sensitivity */
  HRTIM_EVENTFILTER_NONE,				/*uint32_t Filter;        !< Defines the frequency used to sample the External Event and the length of the digital filter.
											This parameter can be a value of @ref HRTIM_External_Event_Filter */
  HRTIM_EVENTFASTMODE_DISABLE,			/*uint32_t FastMode;      !< Indicates whether or not low latency mode is enabled for the external event.
											This parameter can be a value of @ref HRTIM_External_Event_Fast_Mode */

};

HRTIM_SimpleCaptureChannelCfgTypeDef PPGSimpleCaptureChannelCfg={

  /*uint32_t Event;             !< Specifies the external event triggering the capture.
                                   This parameter can be any 'EEVx' value of @ref HRTIM_External_Event_Channels */
  /*uint32_t EventPolarity;     !< Specifies the polarity of the external event (in case of level sensitivity).
                                   This parameter can be a value of @ref HRTIM_External_Event_Polarity */
  /*uint32_t EventSensitivity;  !< Specifies the sensitivity of the external event.
                                   This parameter can be a value of @ref HRTIM_External_Event_Sensitivity */
  /*uint32_t EventFilter;       !< Defines the frequency used to sample the External Event and the length of the digital filter.
                                   This parameter can be a value of @ref HRTIM_External_Event_Filter */





};

HRTIM_CaptureCfgTypeDef PPGCaptureCfg={
	
  HRTIM_CAPTURETRIGGER_EEV_4,/*uint64_t Trigger;          !< Specifies source(s) triggering the capture.
                                  This parameter can be a combination of @ref HRTIM_Capture_Unit_Trigger */
};



HRTIM_FaultCfgTypeDef PPGFaultCfg={

  HRTIM_FAULTSOURCE_INTERNAL,	/*uint32_t Source;        !< Identifies the source of the fault.
                               This parameter can be a value of @ref HRTIM_Fault_Sources */
  HRTIM_FAULTPOLARITY_LOW,		/*uint32_t Polarity;      !< Specifies the polarity of the fault event.
                               This parameter can be a value of @ref HRTIM_Fault_Polarity */
  HRTIM_FAULTFILTER_VALUE,			/*uint32_t Filter;        !< Defines the frequency used to sample the Fault input and the length of the digital filter.
                               This parameter can be a value of @ref HRTIM_Fault_Filter */
  HRTIM_FAULTLOCK_READWRITE,	/*uint32_t Lock;          !< Indicates whether or not fault programming bits are write protected.
                               This parameter can be a value of @ref HRTIM_Fault_Lock */



};

HRTIM_ADCTriggerCfgTypeDef	PPGADCTriggerCfg={

  HRTIM_ADCTRIGGEREVENT13_TIMERB_CMP3,/*uint32_t UpdateSource;  !< Specifies the ADC trigger update source.
                               This parameter can be a value of @ref HRTIM_ADC_Trigger_Update_Source  */
  HRTIM_ADCTRIGGERUPDATE_TIMER_B,/*uint32_t Trigger;       !< Specifies the event(s) triggering the ADC conversion.
                               This parameter can be a combination of @ref HRTIM_ADC_Trigger_Event  */



};


/**
  * @brief HRTIM1 Initialization Function
  * @param None
  * @retval None
  */






//typedef	struct				//EVENT 设置差异项
//{
//	uint32_t 			num;			//序号	
//	HRTIM_CFG_DEF		cfg;			//CFG差异项
//	HRTIM_OUTPUT_DEF	output;			//output差异项
//	
//}HRTIM_SETUP_DEF;	


//HRTIM_SETUP_DEF		const 	HRTIM_SETUP_NUM[6]={
//	
//	{
//		Pot1_TimerIndex,
//		{HRTIM_TIMFAULTENABLE_FAULT1|HRTIM_TIMFAULTENABLE_FAULT3,		HRTIM_TIMER_D_E_DELAYEDPROTECTION_DISABLED},
//		{HRTIM_OUTPUT_TE1,HRTIM_OUTPUT_TE1,0,0,}
//	},
//	
//	
//};	







	HRTIM_FALUT_DEF const 	HRTIM_FAULT_NUM[6]=
{
	{	HRTIM_FAULT_POT1,HRTIM_FAULTSOURCE_INTERNAL,		HRTIM_FAULTPOLARITY_HIGH	,	},		//comp1
	{	HRTIM_FAULT_POT2,HRTIM_FAULTSOURCE_INTERNAL,		HRTIM_FAULTPOLARITY_HIGH	,	},		//comp2
	{	HRTIM_FAULT_POT3,HRTIM_FAULTSOURCE_INTERNAL,		HRTIM_FAULTPOLARITY_HIGH	,	},		//COMP3
	{	HRTIM_FAULT_POT4,HRTIM_FAULTSOURCE_INTERNAL,		HRTIM_FAULTPOLARITY_HIGH	,	},		//COMP4
	{	HRTIM_FAULT_6,HRTIM_FAULTSOURCE_DIGITALINPUT,	HRTIM_FAULTPOLARITY_HIGH,	},		//pc10
	{	HRTIM_FAULT_3,HRTIM_FAULTSOURCE_DIGITALINPUT,	HRTIM_FAULTPOLARITY_HIGH,	},		//pe6


};	



		
HRTIM_EVENT_DEF const 	HRTIM_EVENT_NUM[2]=
{
	{	HRTIM_EVENT_6,HRTIM_EEV6SRC_COMP1_OUT,		HRTIM_EVENTPOLARITY_HIGH	,	},
	{	HRTIM_EVENT_4,HRTIM_EEV4SRC_COMP1_OUT,		HRTIM_EVENTSENSITIVITY_FALLINGEDGE	,	},
};




void API_SystemClocks_Init(void)
{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* System Clock configure */
  HAL_SystemClocks_Config(FW_SYSCLK);
  
  /* Configure ADCx clock prescaler */
	RCC_PeriphCLKInitTypeDef  PeriphClkInit={0};	
	
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC123;
  PeriphClkInit.Adc123ClockSelection = RCC_ADC123CLKSOURCE_PLL;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
  
  if (HAL_RCC_SetPLLPClock(RCC_PLLP_DIV6) != HAL_OK)
  {
      while(1);
  }
  
     /* UART configure (for printf function) */
    HAL_UART_Config();
    
    /* Default UART output */
		char buffer[256];
		int age=100;
    snprintf(buffer, sizeof(buffer),"START%d\n",age); 
	
		printf("%s",buffer);
  
	HAL_NVIC_SetPriority(PendSV_IRQn, IRQ_PRIORITY_PendSV, 0);
//		HAL_NVIC_EnableIRQ(PendSV_IRQn);



}  
DMA_HandleTypeDef    test_dma;

void API_HRTIM1_Init(void)
{

    HRTIM_TimerCtlTypeDef pTimerCtl = {0};

    HRTIM_CompareCfgTypeDef pCompareCfg = {0};
    HRTIM_OutputCfgTypeDef pOutputCfg = {0};
    HRTIM_DeadTimeCfgTypeDef pDeadTimeCfg={0};
    HRTIM_EventCfgTypeDef pEventCfg={0};
    HRTIM_SimpleCaptureChannelCfgTypeDef pSimpleCaptureChannelCfg={0};
    HRTIM_CaptureCfgTypeDef pCaptureCfg={0};
    HRTIM_FaultCfgTypeDef pFaultCfg={0};
    HRTIM_ADCTriggerCfgTypeDef pADCTriggerCfg={0};

    hhrtim1.Instance = HRTIM1;
    hhrtim1.Init.HRTIMInterruptResquests = HRTIM_IT_NONE;
    hhrtim1.Init.SyncOptions = HRTIM_SYNCOPTION_SLAVE|HRTIM_SYNCOPTION_MASTER;												//触发从模式							
    hhrtim1.Init.SyncInputSource = HRTIM_SYNCINPUTSOURCE_EXTERNALEVENT;				//外部触发(IO口触发同步  PB6定义为触发口HRTIM_SCIN)
//    hhrtim1.Init.SyncOutputSource = HRTIM_SYNCOUTPUTSOURCE_TIMA_START;
    hhrtim1.Init.SyncOutputSource = HRTIM_SYNCOUTPUTSOURCE_TIMA_CMP1;       //与hrtim_out_sync2关联，触发TIM_BLANKING,
    hhrtim1.Init.SyncOutputPolarity = HRTIM_SYNCOUTPUTPOLARITY_POSITIVE;
//    hhrtim1.Init.SyncOutputPolarity = HRTIM_SYNCOUTPUTPOLARITY_NEGATIVE;	
	

	
		

//		test_dma.Init.Request=DMA_REQUEST_HRTIM1_B;
//		test_dma.Init.Direction	=DMA_PERIPH_TO_MEMORY;
//		test_dma.Init.PeriphInc	=DMA_PINC_DISABLE;
//		test_dma.Init.MemInc	=DMA_MINC_ENABLE;
//		test_dma.Init.PeriphDataAlignment	=DMA_PDATAALIGN_HALFWORD;
//		test_dma.Init.MemDataAlignment	=DMA_MDATAALIGN_HALFWORD;
//		test_dma.Init.Mode	=DMA_CIRCULAR;
//		test_dma.Init.Priority	=DMA_PRIORITY_LOW;		
//		test_dma.Instance=DMA1_Channel7;


//		HAL_DMA_Init(&test_dma);		
		
		

	
    if (HAL_HRTIM_Init(&hhrtim1) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_HRTIM_DLLCalibrationStart(&hhrtim1, HRTIM_CALIBRATIONRATE_3) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_HRTIM_PollForDLLCalibration(&hhrtim1, 10) != HAL_OK)
    {
        Error_Handler();
    }

/* MASTER统一时钟源配置 START */
    // 配置MASTER定时器作为统一时钟源
    HRTIM_TimeBaseCfgTypeDef MasterTimeBaseCfg = {0};
    MasterTimeBaseCfg.Period = PPGTimeBaseCfg.Period;           // 与Slave相同周期
    MasterTimeBaseCfg.RepetitionCounter = 0;
    MasterTimeBaseCfg.PrescalerRatio = HRTIM_PRESCALERRATIO_MUL4;  // 4倍频 = 768MHz
    MasterTimeBaseCfg.Mode = HRTIM_MODE_CONTINUOUS;
    
    if (HAL_HRTIM_TimeBaseConfig(&hhrtim1, HRTIM_TIMERINDEX_MASTER, &MasterTimeBaseCfg) != HAL_OK)
    {
        Error_Handler();
    }
    
    // 配置MASTER比较寄存器CMP1（用于移相控制预留）
    HRTIM_CompareCfgTypeDef MasterCompareCfg = {0};
    MasterCompareCfg.CompareValue = PPGTimeBaseCfg.Period / 4;  // 默认90度移相点
    
    if (HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_MASTER, HRTIM_COMPAREUNIT_1, &MasterCompareCfg) != HAL_OK)
    {
        Error_Handler();
    }
    
    // ★ 关键：使能MASTER计数器
    SET_BIT(HRTIM1->sMasterRegs.MCR, HRTIM_MCR_MCEN);
/* MASTER统一时钟源配置 END */

/*FAULT SET START*/  

	
	pFaultCfg=PPGFaultCfg;			//统一设置值 
	for(uint8_t i=0;i<6;i++)
	{

		pFaultCfg.Polarity = HRTIM_FAULT_NUM[i].Polarity;
		pFaultCfg.Source= HRTIM_FAULT_NUM[i].Source;
		HAL_HRTIM_FaultConfig(&hhrtim1, HRTIM_FAULT_NUM[i].num, &pFaultCfg);
//		HAL_HRTIM_FaultModeCtl(&hhrtim1,HRTIM_FAULT_NUM[i].num,  HRTIM_FAULTMODECTL_DISABLED);
		HAL_HRTIM_FaultModeCtl(&hhrtim1,HRTIM_FAULT_NUM[i].num,  HRTIM_FAULTMODECTL_ENABLED);		
		
	}	

/*FAULT SET END*/   


    /* ****************** PPG1 TB start****************** */
	if (HAL_HRTIM_PollForDLLCalibration(&hhrtim1, 10) != HAL_OK)
    {
        Error_Handler();
    }

/* MASTER统一时钟源配置 START */
    // 配置MASTER定时器作为统一时钟源
    HRTIM_TimeBaseCfgTypeDef MasterTimeBaseCfg = {0};
    MasterTimeBaseCfg.Period = PPGTimeBaseCfg.Period;           // 与Slave相同周期
    MasterTimeBaseCfg.RepetitionCounter = 0;
    MasterTimeBaseCfg.PrescalerRatio = HRTIM_PRESCALERRATIO_MUL4;  // 4倍频 = 768MHz
    MasterTimeBaseCfg.Mode = HRTIM_MODE_CONTINUOUS;
    
    if (HAL_HRTIM_TimeBaseConfig(&hhrtim1, HRTIM_TIMERINDEX_MASTER, &MasterTimeBaseCfg) != HAL_OK)
    {
        Error_Handler();
    }
    
    // 配置MASTER比较寄存器CMP1（用于移相控制预留）
    HRTIM_CompareCfgTypeDef MasterCompareCfg = {0};
    MasterCompareCfg.CompareValue = PPGTimeBaseCfg.Period / 4;  // 默认90度移相点
    MasterCompareCfg.AutoDelayed = HRTIM_AUTODELAYEDMODE_REGULAR;
    MasterCompareCfg.AutoDelayedTimeout = 0;
    
    if (HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_MASTER, HRTIM_COMPAREUNIT_1, &MasterCompareCfg) != HAL_OK)
    {
        Error_Handler();
    }
    
    // ★ 关键：使能MASTER计数器
    SET_BIT(HRTIM1->sMasterRegs.MCR, HRTIM_MCR_MCEN);
/* MASTER统一时钟源配置 END */

/*FAULT SET START*/  
		
		for(i=0;i<FaultMax;i++)
	{
		HAL_HRTIM_FaultModeCtl(&hhrtim1,HRTIM_FAULT_NUM[i].num,  HRTIM_FAULTMODECTL_ENABLED);		
		
	}	

/*FAULT SET END*/   


    /* ****************** PPG1 TB start****************** */
		

	

	
	uint8_t ch;
	
	for(ch=0;ch<PotMax;ch++)
	{

	
		HRTIM_TimeBaseCfgTypeDef	pTimeBaseCfg=PPGTimeBaseCfg;	
		if (HAL_HRTIM_TimeBaseConfig(&hhrtim1, HRTIM_CFG_NUM[ch].num, &pTimeBaseCfg) != HAL_OK)
		{
			Error_Handler();
		}
		
		// ★ 关键修改：配置Slave定时器使用MASTER统一时钟源
		HRTIM_TimerCfgTypeDef pTimerCfg = PPGTimerCfg;  // 复制原始配置
		pTimerCfg.UpdateTrigger = HRTIM_TIMUPDATETRIGGER_MASTER;  // MASTER更新触发
		pTimerCfg.ResetTrigger = HRTIM_TIMRESETTRIGGER_MASTER_PER; // MASTER周期复位
		
		if (HAL_HRTIM_WaveformTimerConfig(&hhrtim1, HRTIM_CFG_NUM[ch].num, &pTimerCfg) != HAL_OK)
		{
			Error_Handler();
		}
		
		pTimerCtl=PPGTimerCtl;
		if (HAL_HRTIM_WaveformTimerControl(&hhrtim1, HRTIM_CFG_NUM[ch].num, &pTimerCtl) != HAL_OK)
		{
			Error_Handler();
		}
		pCompareCfg = PPGCompareCfg;
		if (HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_CFG_NUM[ch].num, COMPAREUNIT_REST, &pCompareCfg) != HAL_OK)
		{
			Error_Handler();
		}	
//    pCompareCfg.CompareValue = TIMD_PERIOD/5;
//    if (HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_CFG_NUM[ch].num, HRTIM_COMPAREUNIT_3, &pCompareCfg) != HAL_OK)
//    {
//        Error_Handler();
//    }	

		
		pDeadTimeCfg=PPGDeadTimeCfg;
		if (HAL_HRTIM_DeadTimeConfig(&hhrtim1, HRTIM_CFG_NUM[ch].num, &pDeadTimeCfg) != HAL_OK)
		{
			Error_Handler();
		}
		
		API_PPG_SET_CONTINUOUS(ch);		
		API_SET_OutputCfgTypeDef(ch,&PPGOutputCfg);
		
		HAL_HRTIM_SoftwareUpdate(&hhrtim1, HRTIM_CFG_NUM[ch].num);

	}
//		API_PPG_SET_SINGLE(PotChWork);


    pEventCfg.Source = HRTIM_EEV7SRC_TIM7_TRGO;
    pEventCfg.Sensitivity = HRTIM_EVENTSENSITIVITY_RISINGEDGE;
    pEventCfg.Polarity = HRTIM_EVENTPOLARITY_HIGH;
    pEventCfg.Filter = HRTIM_EVENTFILTER_NONE;
    pEventCfg.FastMode = HRTIM_EVENTFASTMODE_DISABLE;
    if (HAL_HRTIM_EventConfig(&hhrtim1, HRTIM_EVENT_7, &pEventCfg) != HAL_OK)
    {
        Error_Handler();
    }
    
    pEventCfg.Source = HRTIM_EEV4SRC_COMP1_OUT;
    pEventCfg.Sensitivity = HRTIM_EVENTSENSITIVITY_FALLINGEDGE;
    if (HAL_HRTIM_EventConfig(&hhrtim1, HRTIM_EVENT_4, &pEventCfg) != HAL_OK)
    {
        Error_Handler();
    }
    
    pCaptureCfg.Trigger = HRTIM_CAPTURETRIGGER_EEV_4;
    if(HAL_HRTIM_WaveformCaptureConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_C, HRTIM_CAPTUREUNIT_1, &pCaptureCfg) != HAL_OK)
    {
      Error_Handler();
    }	
	
	
	
	

	//HRTIM_ODISR	输出停止（先停止所有输出）
	HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TF1|HRTIM_OUTPUT_TF2|HRTIM_OUTPUT_TC1|HRTIM_OUTPUT_TC2|HRTIM_OUTPUT_TB1|HRTIM_OUTPUT_TB2|HRTIM_OUTPUT_TE1|HRTIM_OUTPUT_TE2|HRTIM_OUTPUT_TA1|HRTIM_OUTPUT_TA2|HRTIM_OUTPUT_TD1|HRTIM_OUTPUT_TD2);

	//HRTIM_OENR  输出使能（开启所有炉头输出）
	HAL_HRTIM_WaveformOutputStart(&hhrtim1, 
		HRTIM_OUTPUT_TA1|HRTIM_OUTPUT_TA2|  // TimerA (PotCh2超前臂)
		HRTIM_OUTPUT_TB1|HRTIM_OUTPUT_TB2|  // TimerB (PotCh1超前臂)
		HRTIM_OUTPUT_TC1|HRTIM_OUTPUT_TC2|  // TimerC (测试通道)
		HRTIM_OUTPUT_TD1|HRTIM_OUTPUT_TD2|  // TimerD (PotCh2滞后臂)
		HRTIM_OUTPUT_TE1|HRTIM_OUTPUT_TE2|  // TimerE (PotCh1滞后臂)
		HRTIM_OUTPUT_TF1|HRTIM_OUTPUT_TF2); // TimerF (测试通道)
		

	//sMasterRegs.MCR  计数器使能（MASTER + 所有Slave同时启动）
	HAL_HRTIM_WaveformCountStart(&hhrtim1,  
		HRTIM_TIMERID_MASTER |                          // ★ 新增：MASTER定时器
		HRTIM_TIMERID_TIMER_F|HRTIM_TIMERID_TIMER_B|
		HRTIM_TIMERID_TIMER_E|HRTIM_TIMERID_TIMER_A|
		HRTIM_TIMERID_TIMER_D|HRTIM_TIMERID_TIMER_C);
	__HAL_HRTIM_TIMER_ENABLE_DMA(&hhrtim1, HRTIM_TIMERINDEX_TIMER_F, HRTIM_TIM_DMA_UPD);
	

//		hhrtim1.hdmaTimerB=&test_dma;
//	HAL_HRTIM_WaveformCountStart_DMA(&hhrtim1,  HRTIM_TIMERID_TIMER_B);
//	

	
    /* ****************** ADC Trigger ****************** */
    pADCTriggerCfg.Trigger = HRTIM_ADCTRIGGEREVENT13_TIMERB_CMP3;
    pADCTriggerCfg.UpdateSource = HRTIM_ADCTRIGGERUPDATE_TIMER_B;
    HAL_HRTIM_ADCTriggerConfig(&hhrtim1, HRTIM_ADCTRIGGER_3, &pADCTriggerCfg);
    
    HAL_HRTIM_ADCPostScalerConfig(&hhrtim1, HRTIM_ADCTRIGGER_3, 0);
    
    pADCTriggerCfg.Trigger = HRTIM_ADCTRIGGEREVENT579_TIMERB_PERIOD;
    pADCTriggerCfg.UpdateSource = HRTIM_ADCTRIGGERUPDATE_TIMER_B;
    HAL_HRTIM_ADCTriggerConfig(&hhrtim1, HRTIM_ADCTRIGGER_5, &pADCTriggerCfg);
    
    HAL_HRTIM_ADCPostScalerConfig(&hhrtim1, HRTIM_ADCTRIGGER_5, 0);
    
//    HAL_HRTIM_SoftwareUpdate(&hhrtim1, HRTIM_TIMERUPDATE_B);
//    HAL_HRTIM_SoftwareUpdate(&hhrtim1, HRTIM_TIMERUPDATE_E);
//    HAL_HRTIM_SoftwareUpdate(&hhrtim1, HRTIM_TIMERUPDATE_A);
//    HAL_HRTIM_SoftwareUpdate(&hhrtim1, HRTIM_TIMERUPDATE_D);
//    HAL_HRTIM_SoftwareUpdate(&hhrtim1, HRTIM_TIMERUPDATE_C);

//	__HAL_HRTIM_TIMER_ENABLE_IT(&hhrtim1, HRTIM_TIMERINDEX_TIMER_D, HRTIM_TIM_IT_UPD);
#if 1
//		__HAL_HRTIM_TIMER_ENABLE_IT(&hhrtim1, HRTIM_CFG_NUM[PotChWork].num, HRTIM_TIM_IT_UPD);	
//		__HAL_HRTIM_TIMER_CLEAR_FLAG(&hhrtim1,HRTIM_CFG_NUM[PotChWork].num,HRTIM_TIMICR_UPDC);
//		HAL_NVIC_SetPriority(HRTIM1_CH3_IRQn, IRQ_PRIORITY_HRTIM_CH3, 1);
//    HAL_NVIC_EnableIRQ(HRTIM1_CH3_IRQn);	
	

	
		HAL_NVIC_SetPriority(HRTIM1_PAN_IRQn, IRQ_PRIORITY_HRTIM_PAN, 1);
    HAL_NVIC_EnableIRQ(HRTIM1_PAN_IRQn);			//检锅四个通道都有可能
		
		HAL_NVIC_SetPriority(HRTIM1_TIMB_IRQn, IRQ_PRIORITY_HRTIM_TIMB, 1);
    HAL_NVIC_EnableIRQ(HRTIM1_TIMB_IRQn);	
		
		HAL_NVIC_SetPriority(HRTIM1_TIMD_IRQn, IRQ_PRIORITY_HRTIM_TIMD, 1);
    HAL_NVIC_EnableIRQ(HRTIM1_TIMD_IRQn);	
		
		HAL_NVIC_SetPriority(HRTIM1_TIME_IRQn, IRQ_PRIORITY_HRTIM_TIME, 1);
    HAL_NVIC_EnableIRQ(HRTIM1_TIME_IRQn);	
		
		
		HAL_NVIC_SetPriority(HRTIM1_TEST1_IRQn, IRQ_PRIORITY_HRTIM_TEST1, 1);
    HAL_NVIC_EnableIRQ(HRTIM1_TEST1_IRQn);	 
	
		HAL_NVIC_SetPriority(HRTIM1_TEST2_IRQn, IRQ_PRIORITY_HRTIM_TEST2, 1);
    HAL_NVIC_EnableIRQ(HRTIM1_TEST2_IRQn);	
#endif	
	
    HAL_HRTIM_MspPostInit(&hhrtim1);

}


void	API_PPG_DeadTime(uint8_t ppgCh,uint8_t upDts,uint8_t downDts)		//死区时间设置 
{
		HRTIM_DeadTimeCfgTypeDef	pDeadTimeCfg=PPGDeadTimeCfg;
		pDeadTimeCfg.RisingValue=upDts;
		pDeadTimeCfg.FallingValue=downDts;
		if (HAL_HRTIM_DeadTimeConfig(&hhrtim1, HRTIM_CFG_NUM[ppgCh].num, &pDeadTimeCfg) != HAL_OK)
		{
			Error_Handler();
		}	

}


void				API_PPG_setValue(uint8_t ppgCh,PPGvalueDef value)			//设置PWM输出周期
{
		uint32_t ch=HRTIM_CFG_NUM[ppgCh].num;
	
	__HAL_HRTIM_SETPERIOD(&hhrtim1,ch,value.prioed);
	__HAL_HRTIM_SETCOMPARE(&hhrtim1,ch,COMPAREUNIT_REST,value.duty);
//	__HAL_HRTIM_SETCOMPARE(&hhrtim1,HRTIM_CFG_NUM[ppgCh].num,COMPAREUNIT_BLKS_START,value.duty-BLKS_DIV);	
	__HAL_HRTIM_SETCOMPARE(&hhrtim1,ch,COMPAREUNIT_BLKS_END,value.duty+BLKS_DIV);		
//	__HAL_HRTIM_SETCOMPARE(&hhrtim1,HRTIM_CFG_NUM[ppgCh].num,COMPAREUNIT_SYNC,value.prioed-BLKS_DIV);

}

 void		API_PPG_setPluse(uint8_t ppgCh ,uint16_t value)	
{
	uint32_t ch=HRTIM_CFG_NUM[ppgCh].num;
	
	__HAL_HRTIM_SETCOMPARE(&hhrtim1,ch,COMPAREUNIT_REST,value);
	__HAL_HRTIM_SETCOMPARE(&hhrtim1,ch,COMPAREUNIT_BLKS_END,value+BLKS_DIV);		
//	__HAL_HRTIM_SETCOMPARE(&hhrtim1,HRTIM_CFG_NUM[ppgCh].num,COMPAREUNIT_SYNC,value);		
}

inline void		API_PPG_setValueChx(uint8_t ppgCh ,uint16_t period,uint16_t duty)	
{
	uint32_t ch=HRTIM_CFG_NUM[ppgCh].num;
	HRTIM_Timerx_TypeDef*  timer=&hhrtim1.Instance->sTimerxRegs[ch];
	timer->PERxR=period;
	
	timer->CMP4xR=duty;
	timer->CMP3xR=duty+BLKS_DIV;
}

inline void		API_PPG_setPeriodChx(uint8_t ppgCh ,uint16_t period)	
{
	uint32_t ch=HRTIM_CFG_NUM[ppgCh].num;
	HRTIM_Timerx_TypeDef*  timer=&hhrtim1.Instance->sTimerxRegs[ch];
	timer->PERxR=period;
}


void		API_PPG_setPeriod(uint16_t value)			//设置PWM输出周期
{
	__HAL_HRTIM_SETPERIOD(&hhrtim1,HRTIM_TIMERINDEX_TIMER_A,value);
	__HAL_HRTIM_SETPERIOD(&hhrtim1,HRTIM_TIMERINDEX_TIMER_B,value);	
	__HAL_HRTIM_SETPERIOD(&hhrtim1,HRTIM_TIMERINDEX_TIMER_C,value);	
	__HAL_HRTIM_SETPERIOD(&hhrtim1,HRTIM_TIMERINDEX_TIMER_D,value);
	__HAL_HRTIM_SETPERIOD(&hhrtim1,HRTIM_TIMERINDEX_TIMER_E,value);	
	__HAL_HRTIM_SETPERIOD(&hhrtim1,HRTIM_TIMERINDEX_TIMER_F,value);		
}


uint32_t    API_PPG_GetPeroid(uint8_t ppgCh)
{
  uint32_t 	xReturn=__HAL_HRTIM_GETPERIOD(&hhrtim1,HRTIM_CFG_NUM[ppgCh].num);
	return	xReturn;	
}

PPGvalueDef			API_PPG_getValue(uint8_t ppgCh)					//PWM周期返回
{
	PPGvalueDef	xReturn;
	xReturn.duty=__HAL_HRTIM_GETCOMPARE(&hhrtim1,HRTIM_CFG_NUM[ppgCh].num,COMPAREUNIT_REST);
	xReturn.prioed=__HAL_HRTIM_GETPERIOD(&hhrtim1,HRTIM_CFG_NUM[ppgCh].num);
	return	xReturn;	
}


PPGpointDef			API_PPG_getValueDeadTime(uint8_t ppgCh)		//减去死区的DUTY	
{

	uint32_t	deadHigh,deadLow;     //死区高低端值
  PPGpointDef xReturn;

	xReturn.highOff=__HAL_HRTIM_GETCOMPARE(&hhrtim1,HRTIM_CFG_NUM[ppgCh].num,COMPAREUNIT_REST);
	xReturn.lowOff=__HAL_HRTIM_GETPERIOD(&hhrtim1,HRTIM_CFG_NUM[ppgCh].num);
 
  deadHigh = hhrtim1.Instance->sTimerxRegs[HRTIM_CFG_NUM[ppgCh].num].DTxR;
	deadHigh&=0xff;
	deadHigh*=16;
  deadLow = hhrtim1.Instance->sTimerxRegs[HRTIM_CFG_NUM[ppgCh].num].DTxR>>16;
	deadLow&=0xff;
	deadLow*=16;	
  xReturn.highOn=deadHigh;
  xReturn.lowOn=xReturn.highOff+deadLow;

	return	xReturn;	
}
	
uint32_t API_PPG_getDeadTime(uint8_t ppgCh)
{
	uint32_t		  hrtim_dtr = hhrtim1.Instance->sTimerxRegs[HRTIM_CFG_NUM[ppgCh].num].DTxR;
	hrtim_dtr&=0xff;
	hrtim_dtr*=16;			//这个需与分频系数对应
	return	hrtim_dtr;

}




void	API_PPG_FaultMode(uint8_t ppgCh,uint8_t flag)
{	
	uint32_t faultMode=HRTIM_FAULTMODECTL_DISABLED;
	if(flag)
	{
#ifdef	DEBUG_POWER_OUT					//关闭FAULT

#else		
		faultMode=HRTIM_FAULTMODECTL_ENABLED;
#endif	

		
		
		
	}	
	
	HAL_HRTIM_FaultModeCtl(&hhrtim1,HRTIM_FAULT_NUM[ppgCh].num,  faultMode);		
}


#include	"API_TIM.H"
	#include	"API_adc.h"

void	API_PPG_OnOff(uint8_t ppgCh,uint8_t flag)				//PWM输出开始
{
	if(flag)
	{
		HrtimOutPutPinSave|=HRTIM_CFG_NUM[ppgCh].outputPin;

	}
	else
	{
		
		HrtimOutPutPinSave&=~HRTIM_CFG_NUM[ppgCh].outputPin;

		HAL_HRTIM_WaveformOutputStop(&hhrtim1,HRTIM_CFG_NUM[ppgCh].outputPin);

	}
	

	// API_PPG_BkFlag(ppgCh);			//清除BK标志	
	API_PPG_FaultMode(ppgCh,flag);
}


uint8_t 	API_PPG_ClearFaultFlagCh(uint8_t ch)
{
	uint8_t xRet=0;
	switch(ch)
	{
		case 0:
		xRet=	API_PPG_BkFlag_Pot1();
		break;
		case 1:
		xRet=	API_PPG_BkFlag_Pot2();
		break;
		case 2:
		xRet=	API_PPG_BkFlag_Pot3();
		break;		
		case 3:
		xRet=	API_PPG_BkFlag_Pot4();
		break;
	}
	return	xRet;

}




void	API_PPG_OnOff_NoFault(uint8_t ppgCh,uint8_t flag)				//PWM输出开始
{


	API_PPG_ClearFaultFlagCh(ppgCh);			//清除Fault 标志

	HAL_HRTIM_FaultModeCtl(&hhrtim1,HRTIM_FAULT_NUM[ppgCh].num,  HRTIM_FAULTMODECTL_DISABLED);		
	
	if(flag)
	{
		HAL_HRTIM_WaveformOutputStart(&hhrtim1,HRTIM_CFG_NUM[ppgCh].outputPin);
	}
	else
	{
		HAL_HRTIM_WaveformOutputStop(&hhrtim1,HRTIM_CFG_NUM[ppgCh].outputPin);
	}	

}


// uint8_t		API_PPG_BkFlag(uint8_t ppgCh)						//得到BKFLAG
// {
	
// 	uint8_t xReturn=__HAL_HRTIM_GET_FLAG(&hhrtim1,HRTIM_ISR_FLT1);
// 	__HAL_HRTIM_CLEAR_FLAG(&hhrtim1,HRTIM_ICR_FLT1C);

// 	return xReturn;
// }



uint8_t		API_PPG_BkFlag_Pot1(void)						//得到BKFLAG
{
	
	uint8_t xReturn=__HAL_HRTIM_GET_FLAG(&hhrtim1,HRTIM_ISR_POT1);
	
	// if(xReturn)
	// {
	// 	__HAL_HRTIM_CLEAR_FLAG(&hhrtim1,HRTIM_ICR_FLT1C);
	// }	
	
	__HAL_HRTIM_CLEAR_FLAG(&hhrtim1,HRTIM_ICR_POT1);

	return xReturn;
}
uint8_t				API_PPG_BkFlag_Pot2(void)						//得到BKFLAG
{
	
	uint8_t xReturn=__HAL_HRTIM_GET_FLAG(&hhrtim1,HRTIM_ISR_POT2);
	__HAL_HRTIM_CLEAR_FLAG(&hhrtim1,HRTIM_ICR_POT2);

	return xReturn;
}
uint8_t				API_PPG_BkFlag_Pot3(void)						//得到BKFLAG
{
	
	uint8_t xReturn=__HAL_HRTIM_GET_FLAG(&hhrtim1,HRTIM_ISR_POT3);
	__HAL_HRTIM_CLEAR_FLAG(&hhrtim1,HRTIM_ICR_POT3);

	return xReturn;
}
uint8_t				API_PPG_BkFlag_Pot4(void)						//得到BKFLAG
{
	
	uint8_t xReturn=__HAL_HRTIM_GET_FLAG(&hhrtim1,HRTIM_ISR_POT4);
	__HAL_HRTIM_CLEAR_FLAG(&hhrtim1,HRTIM_ICR_POT4);

	return xReturn;
}




	void	API_HRTIM1_TEST2_IRQHandler(void)
	{
		uint32_t adcSR=hhrtim1.Instance->sTimerxRegs[HRTIM_CFG_NUM[PotChBase].num].TIMxISR;
		adcSR&=hhrtim1.Instance->sTimerxRegs[HRTIM_CFG_NUM[PotChBase].num].TIMxDIER;

    if(adcSR&HRTIM_TIM_IT_UPD)
    {
			
		  __HAL_HRTIM_TIMER_CLEAR_FLAG(&hhrtim1,HRTIM_CFG_NUM[PotChBase].num,HRTIM_TIMICR_UPDC);
 		  __HAL_HRTIM_TIMER_DISABLE_IT(&hhrtim1, HRTIM_CFG_NUM[PotChBase].num, HRTIM_TIM_IT_UPD);	     
			API_HRTIM1_TEST_UPD_IRQHandlerCallback(1);  //upd中断
			return;
    }		
		
    if(adcSR&HRTIM1_TEST2_IT)
    {
			__HAL_HRTIM_TIMER_CLEAR_FLAG(&hhrtim1,HRTIM_CFG_NUM[PotChBase].num,HRTIM1_TEST2_ICR);
			API_HRTIM1_TEST_UPD_IRQHandlerCallback(0);    //CMP4中断
			return;
    }  


	}			
		



void	API_HRTIM1_TEST1_IRQHandler(void)
{
//		uint32_t adcSR=hhrtim1.Instance->sTimerxRegs[HRTIM_CFG_NUM[PotChTest1].num].TIMxISR;
//		adcSR&=hhrtim1.Instance->sTimerxRegs[HRTIM_CFG_NUM[PotChTest1].num].TIMxDIER;
//  if(adcSR&HRTIM_TIMISR_CMP1)


		__HAL_HRTIM_TIMER_CLEAR_FLAG(&hhrtim1,HRTIM_CFG_NUM[PotChTest1].num,HRTIM1_TEST1_ICR);
	
		API_HRTIM1_TEST_CMP1_IRQHandlerCallback();

	
}

void	API_HRTIM1_PAN_IRQHandler(uint8_t ch)		//检锅起振
{
	static uint8_t count=0;

//	__HAL_HRTIM_TIMER_DISABLE_IT(&hhrtim1, HRTIM_CFG_NUM[ch].num, HRTIM1_PAN_IT);	
	__HAL_HRTIM_TIMER_CLEAR_FLAG(&hhrtim1,	HRTIM_CFG_NUM[ch].num,HRTIM1_PAN_ICR); 
 
		hrtimPan.count++;
		#if 0
    if(hrtimPan.count>PAN_START_PPG_COUNT)
    {
		//
			__HAL_HRTIM_TIMER_DISABLE_IT(&hhrtim1, HRTIM_PAN_NUM, HRTIM1_PAN_IT);	
			API_PPG_OnOff_NoFault(ch,0);	
			API_HRTIM_PanOffCallBack();		
    }

    #else
		switch(hrtimPan.count)
		{
			case	PAN_INIT:
				

			API_PPG_OnOff_NoFault(ch,1);
      				//开始起振
			break;
			
/*
 * [原始版本 — 已废弃]
 * 修改原因: 检锅结束后需恢复MASTER同步，在下一个零点与其他通道统一重新同步。
 * 修改日期: 2026-05-28
 *
			case	PAN_END+1:

			HAL_HRTIM_WaveformOutputStart(&hhrtim1, HrtimOutPutPinSave);
			__HAL_HRTIM_TIMER_DISABLE_IT(&hhrtim1, HRTIM_CFG_NUM[ch].num, HRTIM1_PAN_IT);


			break;
*/

// [新版本] 检锅结束 — 恢复输出 + 恢复MASTER同步
// 修改原因: 检锅完成，该通道需要重新加入MASTER同步体系。
//           恢复 UpdateTrigger=MASTER, ResetTrigger=MASTER_PER。
//           下一次GPIO同步脉冲(API_GPIO_PinPull翻转SYN引脚)时，
//           该通道在零点与其他MASTER通道统一重同步。
// 修改日期: 2026-05-28
			case	PAN_END+1:

			HAL_HRTIM_WaveformOutputStart(&hhrtim1, HrtimOutPutPinSave);
			__HAL_HRTIM_TIMER_DISABLE_IT(&hhrtim1, HRTIM_CFG_NUM[ch].num, HRTIM1_PAN_IT);

			// ★ 修改: 恢复MASTER同步 — 重新配置为MASTER触发更新+MASTER周期复位
			{
				HRTIM_TimerCfgTypeDef pTimerCfg = PPGTimerCfg;
				pTimerCfg.UpdateTrigger = HRTIM_TIMUPDATETRIGGER_MASTER;
				pTimerCfg.ResetTrigger = HRTIM_TIMRESETTRIGGER_MASTER_PER;
				pTimerCfg.FaultEnable = HRTIM_CFG_NUM[ch].FaultEnable;
				pTimerCfg.DelayedProtectionMode = HRTIM_CFG_NUM[ch].DelayedProtectionMode;
				HAL_HRTIM_WaveformTimerConfig(&hhrtim1, HRTIM_CFG_NUM[ch].num, &pTimerCfg);
			}

			break;
			case	PAN_START_PPG_COUNT+1:

													// APP_ADC_PanSwChange(PanPluse.ch);
													// API_ADC_DMA_RecoverPan(PanPluse.ch);				//恢复DMA	
			API_HRTIM_PanOffCallBack();				//开始计数
			__HAL_HRTIM_TIMER_DISABLE_IT(&hhrtim1, HRTIM_CFG_NUM[ch].num, HRTIM1_PAN_IT);	
			API_PPG_OnOff_NoFault(ch,0);				//关闭脉冲
			break;
	
		}	
    #endif  

}



void	API_SET_OutputCfgTypeDef(uint8_t ch, HRTIM_OutputCfgTypeDef* str)
{

		uint32_t outPutPin;

//HRTIM_TIMERINDEX_TIMER_A向HRTIM_OUTPUT_TA1转换  *2+1
//#define HRTIM_TIMERINDEX_TIMER_A    0x0U   /*!< Index used to access timer A registers 
//#define HRTIM_TIMERINDEX_TIMER_B    0x1U   /*!< Index used to access timer B registers 		
//		
//#define HRTIM_OUTPUT_TA1  0x00000001U  /*!< Timer A - Output 1 identifier 
//#define HRTIM_OUTPUT_TA2  0x00000002U  /*!< Timer A - Output 2 identifier 
		
//		outPutPin<<=HRTIM_CFG_NUM[ch].num*2;		
		
		
		outPutPin=HRTIM_CFG_NUM[ch].outputPin&0x55555555U;		//取奇数位
		
		if(outPutPin)
		{	
			if (HAL_HRTIM_WaveformOutputConfig(&hhrtim1, HRTIM_CFG_NUM[ch].num, outPutPin, str) != HAL_OK)
			{
				Error_Handler();
			}
		}
		outPutPin=HRTIM_CFG_NUM[ch].outputPin&0xAAAAAAAAU;		//取偶数位
		if(outPutPin)
		{	
			if (HAL_HRTIM_WaveformOutputConfig(&hhrtim1, HRTIM_CFG_NUM[ch].num, outPutPin, str) != HAL_OK)
			{
			Error_Handler();
			}
		}	


}	

#if 0

void	API_SET_OutputCfgTypeDef_Single(uint8_t ch)
{
		HRTIM_OutputCfgTypeDef	    pOutputCfg={0};
		uint32_t outPutPin;	
		outPutPin=HRTIM_OUTPUT_NUM[ch].outputPin&0x55555555U;		//取奇数位
		
		if(outPutPin)
		{	
			
				pOutputCfg.Polarity = HRTIM_OUTPUTPOLARITY_HIGH;    
				pOutputCfg.SetSource = HRTIM_OUTPUTSET_TIMCMP1;
				pOutputCfg.ResetSource = HRTIM_OUTPUTRESET_TIMCMP2; 
				pOutputCfg.IdleMode = HRTIM_OUTPUTIDLEMODE_NONE;
				pOutputCfg.IdleLevel = HRTIM_OUTPUTIDLELEVEL_INACTIVE;
				pOutputCfg.FaultLevel = HRTIM_OUTPUTFAULTLEVEL_INACTIVE;
				pOutputCfg.ChopperModeEnable = HRTIM_OUTPUTCHOPPERMODE_DISABLED;
				pOutputCfg.BurstModeEntryDelayed = HRTIM_OUTPUTBURSTMODEENTRY_REGULAR;
			
			
			if (HAL_HRTIM_WaveformOutputConfig(&hhrtim1, HRTIM_CFG_NUM[ch].num, outPutPin, &pOutputCfg) != HAL_OK)
			{
				Error_Handler();
			}
		}
		outPutPin=HRTIM_OUTPUT_NUM[ch].outputPin&0xAAAAAAAAU;		//取偶数位
		

		if(outPutPin)				
		{	
			
			
    pOutputCfg.Polarity = HRTIM_OUTPUTPOLARITY_HIGH;    
    pOutputCfg.SetSource = HRTIM_OUTPUTSET_TIMCMP3;
    pOutputCfg.ResetSource = HRTIM_OUTPUTRESET_TIMCMP4; 
    pOutputCfg.IdleMode = HRTIM_OUTPUTIDLEMODE_NONE;
    pOutputCfg.IdleLevel = HRTIM_OUTPUTIDLELEVEL_INACTIVE;
    pOutputCfg.FaultLevel = HRTIM_OUTPUTFAULTLEVEL_INACTIVE;
    pOutputCfg.ChopperModeEnable = HRTIM_OUTPUTCHOPPERMODE_DISABLED;
    pOutputCfg.BurstModeEntryDelayed = HRTIM_OUTPUTBURSTMODEENTRY_REGULAR; 		
		
					
			
			if (HAL_HRTIM_WaveformOutputConfig(&hhrtim1, HRTIM_CFG_NUM[ch].num, outPutPin, &pOutputCfg) != HAL_OK)
			{
			Error_Handler();
			}
		}	

	HAL_HRTIM_SoftwareUpdate(&hhrtim1,HRTIM_CFG_NUM[ch].num);


}


void	API_PPG_SET_ComparaValueSingle(void)
{

	HRTIM_CompareCfgTypeDef	pCompareCfg=PPGCompareCfg;
    //TA_CMP1
    pCompareCfg.CompareValue = 38400/5*1;
    if (HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, COMPAREUNIT_REST, &pCompareCfg) != HAL_OK)
    {
        Error_Handler();
    }
    //TA_CMP2
    pCompareCfg.CompareValue = 38400/5*2;
    if (HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_2, &pCompareCfg) != HAL_OK)
    {
        Error_Handler();
    }
    //TA_CMP3
    pCompareCfg.CompareValue = 38400/5*3;
    if (HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_3, &pCompareCfg) != HAL_OK)
    {
        Error_Handler();
    }
    //TA_CMP4
    pCompareCfg.CompareValue = 38400/5*4;
    if (HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_4, &pCompareCfg) != HAL_OK)
    {
        Error_Handler();
    }	


}	
#endif

//void	API_PPG_SET_SINGLE(uint8_t ch)
//{

//	__HAL_HRTIM_SETCOUNTER(&hhrtim1,HRTIM_CFG_NUM[ch].num,0);
//	
////		HRTIM_CompareCfgTypeDef	pCompareCfg=PPGCompareCfg;
////    
////		pCompareCfg.CompareValue = 38400/2;
////    if (HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_CFG_NUM[ch].num, COMPAREUNIT_REST, &pCompareCfg) != HAL_OK)
////    {
////        Error_Handler();
////    }
//		
//	HRTIM_TimerCfgTypeDef	pTimerCfg=PPGTimerCfg;

//	pTimerCfg.StartOnSync		=		HRTIM_SYNCSTART_DISABLED;
//	pTimerCfg.PreloadEnable	=		HRTIM_PRELOAD_DISABLED;
//	pTimerCfg.FaultEnable		=		HRTIM_TIMFAULTENABLE_NONE;
//	pTimerCfg.ResetTrigger	=		HRTIM_TIMRESETTRIGGER_EEV_7;
//	
//	if (HAL_HRTIM_WaveformTimerConfig(&hhrtim1, HRTIM_CFG_NUM[ch].num, &pTimerCfg) != HAL_OK)
//	{
//			Error_Handler();
//	}
//	

//}	

void	API_PPG_SET_CONTINUOUS(uint8_t ch)
{
	
	
		HRTIM_TimerCfgTypeDef	pTimerCfg=PPGTimerCfg;
		pTimerCfg.FaultEnable = HRTIM_CFG_NUM[ch].FaultEnable;          //Enable FLT1 -->CMP2
		pTimerCfg.DelayedProtectionMode =HRTIM_CFG_NUM[ch].DelayedProtectionMode ;

		if (HAL_HRTIM_WaveformTimerConfig(&hhrtim1, HRTIM_CFG_NUM[ch].num, &pTimerCfg) != HAL_OK)
		{
			Error_Handler();
		}

}	

uint8_t	 API_HRTIM_GET_IT_UPD(uint8_t ch)				//开启ADC WDT 数据读取    开启HRTIM更新中断，更新中断发生后开始M2M ADC EOC BUFF
{
	return 	__HAL_HRTIM_TIMER_GET_ITSTATUS(&hhrtim1, HRTIM_CFG_NUM[ch].num, HRTIM_TIM_IT_UPD);	
}	
void	 API_HRTIM_CLEAR_IT_UPD(uint8_t ch)				//开启ADC WDT 数据读取    开启HRTIM更新中断，更新中断发生后开始M2M ADC EOC BUFF
{
	__HAL_HRTIM_TIMER_CLEAR_FLAG(&hhrtim1,HRTIM_CFG_NUM[PotChBase].num,HRTIM_TIMICR_UPDC);	
}		
uint32_t 	API_HRTIM_GetAddressTestCnt(void)
{
	
	return	(uint32_t)&(hhrtim1.Instance->sTimerxRegs[HRTIM_CFG_NUM[PotChTest1].num].CNTxR);

}	
uint32_t 	API_HRTIM_GetAddressTxaCnt(uint8_t ch)
{
	
	return	(uint32_t)&(hhrtim1.Instance->sTimerxRegs[HRTIM_CFG_NUM[ch].num].CNTxR);

}	
uint32_t 	API_HRTIM_GetTxaCnt(uint8_t ch)
{
	
	return	hhrtim1.Instance->sTimerxRegs[HRTIM_CFG_NUM[ch].num].CNTxR;

}			
void API_HRTIM_BASE_ENABLE_IT_CMP(void)				//开启ADC WDT 数据读取    开启HRTIM更新中断，更新中断发生后开始M2M ADC EOC BUFF
{
//		__HAL_HRTIM_TIMER_CLEAR_FLAG(&hhrtim1,HRTIM_CFG_NUM[PotChTest1].num,HRTIM_TIMICR_CMP3C);
		__HAL_HRTIM_TIMER_CLEAR_FLAG(&hhrtim1,HRTIM_CFG_NUM[PotChBase].num,HRTIM1_TEST2_ICR);
		__HAL_HRTIM_TIMER_ENABLE_IT(&hhrtim1, HRTIM_CFG_NUM[PotChBase].num, HRTIM1_TEST2_IT);	//F

}	



void API_HRTIM_BASE_DISABLE_IT_CMP(void)				//
{		__HAL_HRTIM_TIMER_CLEAR_FLAG(&hhrtim1,HRTIM_CFG_NUM[PotChBase].num,HRTIM1_TEST2_ICR);
		__HAL_HRTIM_TIMER_DISABLE_IT(&hhrtim1, HRTIM_CFG_NUM[PotChBase].num, HRTIM1_TEST2_IT);	
		
}	


void API_HRTIM_BASE_ENABLE_IT_UPD(void)				//开启ADC WDT 数据读取    开启HRTIM更新中断，更新中断发生后开始M2M ADC EOC BUFF
{
//		__HAL_HRTIM_TIMER_CLEAR_FLAG(&hhrtim1,HRTIM_CFG_NUM[PotChTest1].num,HRTIM_TIMICR_CMP3C);
		__HAL_HRTIM_TIMER_CLEAR_FLAG(&hhrtim1,HRTIM_CFG_NUM[PotChBase].num,HRTIM_TIMICR_UPDC);
		__HAL_HRTIM_TIMER_ENABLE_IT(&hhrtim1, HRTIM_CFG_NUM[PotChBase].num, HRTIM_TIM_IT_UPD);	//F

}	

void API_HRTIM_BASE_DISABLE_IT_UPD(void)				//
{		__HAL_HRTIM_TIMER_CLEAR_FLAG(&hhrtim1,HRTIM_CFG_NUM[PotChBase].num,HRTIM_TIMICR_UPDC);
		__HAL_HRTIM_TIMER_DISABLE_IT(&hhrtim1, HRTIM_CFG_NUM[PotChBase].num, HRTIM_TIM_IT_UPD);	
		
}	

void API_HRTIM_ENABLE_IT_REST(void)				//开启ADC WDT 数据读取    开启HRTIM更新中断，更新中断发生后开始M2M ADC EOC BUFF
{
		__HAL_HRTIM_TIMER_CLEAR_FLAG(&hhrtim1,HRTIM_CFG_NUM[PotChTest1].num,HRTIM1_TEST1_ICR);
		__HAL_HRTIM_TIMER_ENABLE_IT(&hhrtim1, HRTIM_CFG_NUM[PotChTest1].num, HRTIM1_TEST1_IT);	

}	
void API_HRTIM_DISABLE_IT_REST(void)				//
{
		__HAL_HRTIM_TIMER_CLEAR_FLAG(&hhrtim1,HRTIM_CFG_NUM[PotChTest1].num,HRTIM1_TEST1_ICR);
		__HAL_HRTIM_TIMER_DISABLE_IT(&hhrtim1, HRTIM_CFG_NUM[PotChTest1].num, HRTIM1_TEST1_IT);	

}			
	
void	API_HRTIM_CountRest(uint8_t ch )
{
	
	HAL_HRTIM_WaveformCountStop(&hhrtim1,	HRTIM_CFG_NUM[ch].TIMERID);
	__HAL_HRTIM_SETCOUNTER(&hhrtim1, HRTIM_CFG_NUM[ch].num,0);	
	HAL_HRTIM_WaveformCountStart(&hhrtim1,	HRTIM_CFG_NUM[ch].TIMERID);
}

#define		SYN_BY_INT		1			//零点同步用中断保持上管先开通
void 	TIMsynchronous(void)			//HRTIM同步
{

	uint32_t  preiod;
  uint32_t  duty;
	uint8_t 	ch=PotCh1;
	hrtimPan.ch=ch;

		preiod=START_FRE_PWM*2;

#if SYN_BY_INT	
	hrtimPan.count=PAN_END;
	duty=preiod*95/100;
#else
	duty=preiod*80/100;
#endif	

//		MX_HRTIM1_Init();
//	API_GPIO_PinPull(HRTIM_SYN_pin,PUPDR_Pulldown);
//	hhrtim1.Instance->sMasterRegs.MCR &= ~(HRTIM_MCR_SYNC_SRC);
//  hhrtim1.Instance->sMasterRegs.MCR |= HRTIM_SYNCINPUTSOURCE_EXTERNALEVENT & HRTIM_MCR_SYNC_SRC;
	hhrtim1.Instance->sTimerxRegs[0].TIMxCR&=	~HRTIM_PRELOAD_ENABLED;
	hhrtim1.Instance->sTimerxRegs[1].TIMxCR&=	~HRTIM_PRELOAD_ENABLED;
	hhrtim1.Instance->sTimerxRegs[2].TIMxCR&=	~HRTIM_PRELOAD_ENABLED;
	hhrtim1.Instance->sTimerxRegs[3].TIMxCR&=	~HRTIM_PRELOAD_ENABLED;
	hhrtim1.Instance->sTimerxRegs[4].TIMxCR&=	~HRTIM_PRELOAD_ENABLED;
	hhrtim1.Instance->sTimerxRegs[5].TIMxCR&=	~HRTIM_PRELOAD_ENABLED;

	__HAL_HRTIM_MASTER_CLEAR_IT(&hhrtim1,HRTIM_MASTER_IT_SYNC);
	HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TB1|HRTIM_OUTPUT_TB2|HRTIM_OUTPUT_TE1|HRTIM_OUTPUT_TE2|HRTIM_OUTPUT_TA1|HRTIM_OUTPUT_TA2|HRTIM_OUTPUT_TD1|HRTIM_OUTPUT_TD2);
	HAL_HRTIM_WaveformCountStop(&hhrtim1, HRTIM_TIMERID_TIMER_F|HRTIM_TIMERID_TIMER_B|HRTIM_TIMERID_TIMER_E|HRTIM_TIMERID_TIMER_A|HRTIM_TIMERID_TIMER_D|HRTIM_TIMERID_TIMER_C);
#if SYN_BY_INT		
	__HAL_HRTIM_SETCOMPARE(&hhrtim1,HRTIM_TIMERINDEX_TIMER_A,COMPAREUNIT_PAN,duty);		
	__HAL_HRTIM_TIMER_CLEAR_FLAG(&hhrtim1,	HRTIM_TIMERINDEX_TIMER_A,HRTIM1_PAN_ICR);
	__HAL_HRTIM_TIMER_ENABLE_IT(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM1_PAN_IT);	
#endif

	__HAL_HRTIM_SETPERIOD(&hhrtim1,HRTIM_TIMERINDEX_TIMER_A,preiod);
	__HAL_HRTIM_SETPERIOD(&hhrtim1,HRTIM_TIMERINDEX_TIMER_B,preiod);
	__HAL_HRTIM_SETPERIOD(&hhrtim1,HRTIM_TIMERINDEX_TIMER_C,preiod);
	__HAL_HRTIM_SETPERIOD(&hhrtim1,HRTIM_TIMERINDEX_TIMER_D,preiod);
	__HAL_HRTIM_SETPERIOD(&hhrtim1,HRTIM_TIMERINDEX_TIMER_E,preiod);
	__HAL_HRTIM_SETPERIOD(&hhrtim1,HRTIM_TIMERINDEX_TIMER_F,preiod);


	__HAL_HRTIM_SETCOUNTER(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A,duty);
	__HAL_HRTIM_SETCOUNTER(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B,duty);	
	__HAL_HRTIM_SETCOUNTER(&hhrtim1, HRTIM_TIMERINDEX_TIMER_C,duty);			
	__HAL_HRTIM_SETCOUNTER(&hhrtim1, HRTIM_TIMERINDEX_TIMER_D,duty);	
	__HAL_HRTIM_SETCOUNTER(&hhrtim1, HRTIM_TIMERINDEX_TIMER_E,duty);			
	__HAL_HRTIM_SETCOUNTER(&hhrtim1, HRTIM_TIMERINDEX_TIMER_F,duty);	


	HAL_HRTIM_WaveformCountStart(&hhrtim1, HRTIM_TIMERID_TIMER_F|HRTIM_TIMERID_TIMER_B|HRTIM_TIMERID_TIMER_E|HRTIM_TIMERID_TIMER_A|HRTIM_TIMERID_TIMER_D|HRTIM_TIMERID_TIMER_C);
 
#if SYN_BY_INT	
	API_GPIO_PinPull(HRTIM_SYN_pin,PUPDR_Pulldown);
	API_GPIO_PinPull(HRTIM_SYN_pin,PUPDR_Pullup);
	HAL_HRTIM_WaveformOutputStart(&hhrtim1, HrtimOutPutPinSave|HRTIM_OUTPUT_TC1|HRTIM_OUTPUT_TC2);
	
#else
	__disable_irq();	
	API_GPIO_PinPull(HRTIM_SYN_pin,PUPDR_Pulldown);
	API_GPIO_PinPull(HRTIM_SYN_pin,PUPDR_Pullup);
	HAL_HRTIM_WaveformOutputStart(&hhrtim1, HrtimOutPutPinSave|HRTIM_OUTPUT_TE1|HRTIM_OUTPUT_TE2);
	
  __enable_irq();
#endif	
	hhrtim1.Instance->sTimerxRegs[HRTIM_CFG_NUM[0].num].TIMxCR|=	HRTIM_PRELOAD_ENABLED;
	hhrtim1.Instance->sTimerxRegs[HRTIM_CFG_NUM[1].num].TIMxCR|=	HRTIM_PRELOAD_ENABLED;
	hhrtim1.Instance->sTimerxRegs[HRTIM_CFG_NUM[2].num].TIMxCR|=	HRTIM_PRELOAD_ENABLED;
	hhrtim1.Instance->sTimerxRegs[HRTIM_CFG_NUM[3].num].TIMxCR|=	HRTIM_PRELOAD_ENABLED;
	hhrtim1.Instance->sTimerxRegs[HRTIM_CFG_NUM[4].num].TIMxCR|=	HRTIM_PRELOAD_ENABLED;
	hhrtim1.Instance->sTimerxRegs[HRTIM_CFG_NUM[5].num].TIMxCR|=	HRTIM_PRELOAD_ENABLED;


}	


void 	TIMsynchronousPower(void)			//HRTIM同步
{

#if 0	
	uint32_t  period;
	uint8_t 	ch=PotCh1;
	hrtimPan.ch=ch;

	period=0XFFFF;


//		MX_HRTIM1_Init();
//	API_GPIO_PinPull(HRTIM_SYN_pin,PUPDR_Pulldown);
//	hhrtim1.Instance->sMasterRegs.MCR &= ~(HRTIM_MCR_SYNC_SRC);
//  hhrtim1.Instance->sMasterRegs.MCR |= HRTIM_SYNCINPUTSOURCE_EXTERNALEVENT & HRTIM_MCR_SYNC_SRC;
	hhrtim1.Instance->sTimerxRegs[0].TIMxCR&=	~HRTIM_PRELOAD_ENABLED;
	hhrtim1.Instance->sTimerxRegs[1].TIMxCR&=	~HRTIM_PRELOAD_ENABLED;
	hhrtim1.Instance->sTimerxRegs[2].TIMxCR&=	~HRTIM_PRELOAD_ENABLED;
	hhrtim1.Instance->sTimerxRegs[3].TIMxCR&=	~HRTIM_PRELOAD_ENABLED;
	hhrtim1.Instance->sTimerxRegs[4].TIMxCR&=	~HRTIM_PRELOAD_ENABLED;
	hhrtim1.Instance->sTimerxRegs[5].TIMxCR&=	~HRTIM_PRELOAD_ENABLED;

	__HAL_HRTIM_MASTER_CLEAR_IT(&hhrtim1,HRTIM_MASTER_IT_SYNC);
	HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TB1|HRTIM_OUTPUT_TB2|HRTIM_OUTPUT_TE1|HRTIM_OUTPUT_TE2|HRTIM_OUTPUT_TA1|HRTIM_OUTPUT_TA2|HRTIM_OUTPUT_TD1|HRTIM_OUTPUT_TD2);
	HAL_HRTIM_WaveformCountStop(&hhrtim1, HRTIM_TIMERID_TIMER_F|HRTIM_TIMERID_TIMER_B|HRTIM_TIMERID_TIMER_E|HRTIM_TIMERID_TIMER_A|HRTIM_TIMERID_TIMER_D|HRTIM_TIMERID_TIMER_C);

	__HAL_HRTIM_SETCOUNTER(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A,period);	
	__HAL_HRTIM_SETCOUNTER(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B,period);	
	__HAL_HRTIM_SETCOUNTER(&hhrtim1, HRTIM_TIMERINDEX_TIMER_C,period);	
	__HAL_HRTIM_SETCOUNTER(&hhrtim1, HRTIM_TIMERINDEX_TIMER_D,period);	
	__HAL_HRTIM_SETCOUNTER(&hhrtim1, HRTIM_TIMERINDEX_TIMER_E,period);	
	__HAL_HRTIM_SETCOUNTER(&hhrtim1, HRTIM_TIMERINDEX_TIMER_F,period);		

	HAL_HRTIM_WaveformCountStart(&hhrtim1, HRTIM_TIMERID_TIMER_F|HRTIM_TIMERID_TIMER_B|HRTIM_TIMERID_TIMER_E|HRTIM_TIMERID_TIMER_A|HRTIM_TIMERID_TIMER_D|HRTIM_TIMERID_TIMER_C);
 

	API_GPIO_PinPull(HRTIM_SYN_pin,PUPDR_Pulldown);
	API_GPIO_PinPull(HRTIM_SYN_pin,PUPDR_Pullup);
	HAL_HRTIM_WaveformOutputStart(&hhrtim1, HrtimOutPutPinSave|HRTIM_OUTPUT_TC1|HRTIM_OUTPUT_TC2);
	

	hhrtim1.Instance->sTimerxRegs[0].TIMxCR|=	HRTIM_PRELOAD_ENABLED;
	hhrtim1.Instance->sTimerxRegs[1].TIMxCR|=	HRTIM_PRELOAD_ENABLED;
	hhrtim1.Instance->sTimerxRegs[2].TIMxCR|=	HRTIM_PRELOAD_ENABLED;
	hhrtim1.Instance->sTimerxRegs[3].TIMxCR|=	HRTIM_PRELOAD_ENABLED;
	hhrtim1.Instance->sTimerxRegs[4].TIMxCR|=	HRTIM_PRELOAD_ENABLED;
	hhrtim1.Instance->sTimerxRegs[5].TIMxCR|=	HRTIM_PRELOAD_ENABLED;
#endif

}	



void	API_HRTIM_WaitCom(uint8_t ch)
{
	__HAL_HRTIM_TIMER_CLEAR_FLAG(&hhrtim1,HRTIM_CFG_NUM[ch].num,HRTIM1_PAN_ICR);	

	while(__HAL_HRTIM_TIMER_GET_FLAG(&hhrtim1, HRTIM_CFG_NUM[ch].num, HRTIM1_PAN_ISR)==0)
	{};	

}

/*
 * [原始版本 — 已废弃]
 * 修改原因: 检锅炉头需要以独立频率发检锅脉冲，必须临时脱离MASTER同步约束。
 *          检锅完成后在 PAN_END+1 中断中恢复MASTER同步。
 * 修改日期: 2026-05-28
 *
void	API_HRTIM_CHECK_PAN_PLUSE(uint8_t ch )
{
	uint32_t  period;
	hrtimPan.ch=ch;
	hrtimPan.count=0;
	period=__HAL_HRTIM_GETPERIOD(&hhrtim1,HRTIM_CFG_NUM[ch].num);
	period=period*6/10;
  HAL_HRTIM_WaveformCountStop(&hhrtim1, HRTIM_CFG_NUM[ch].TIMERID);

	__HAL_HRTIM_SETCOMPARE(&hhrtim1,HRTIM_CFG_NUM[ch].num,COMPAREUNIT_PAN,period);
	__HAL_HRTIM_TIMER_CLEAR_FLAG(&hhrtim1,	HRTIM_CFG_NUM[ch].num,HRTIM1_PAN_ICR);
	__HAL_HRTIM_TIMER_ENABLE_IT(&hhrtim1, HRTIM_CFG_NUM[ch].num, HRTIM1_PAN_IT);
  __HAL_HRTIM_SETCOUNTER(&hhrtim1, HRTIM_CFG_NUM[ch].num,0);

	HAL_HRTIM_WaveformCountStart(&hhrtim1, HRTIM_CFG_NUM[ch].TIMERID);

	API_GPIO_PinPull(HRTIM_SYN_pin,PUPDR_Pulldown);          //等待HRTIM中断停止
	API_GPIO_PinPull(HRTIM_SYN_pin,PUPDR_Pullup);
//	API_PPG_OnOff_NoFault(ch,1);


}
*/

// [新版本] 检锅脉冲检查 — 临时脱离MASTER同步
// 修改原因: 检锅炉头需以独立频率发脉冲，必须临时解除MASTER同步约束。
//           WaveformCountStop后立即调用 API_PPG_SET_CONTINUOUS 恢复独立连续模式，
//           使该通道的 UpdateTrigger/ResetTrigger 回到 NONE，不受后续GPIO同步脉冲影响。
//           PAN_END+1 中断中恢复MASTER同步配置，下一个零点与其他通道统一重同步。
// 修改日期: 2026-05-28
void	API_HRTIM_CHECK_PAN_PLUSE(uint8_t ch )
{
	uint32_t  period;
	hrtimPan.ch=ch;
	hrtimPan.count=0;
	period=__HAL_HRTIM_GETPERIOD(&hhrtim1,HRTIM_CFG_NUM[ch].num);
	period=period*6/10;
  HAL_HRTIM_WaveformCountStop(&hhrtim1, HRTIM_CFG_NUM[ch].TIMERID);

	// ★ 修改: 解除MASTER同步，恢复独立连续模式 (UpdateTrigger/ResetTrigger → NONE)
	//         使检锅脉冲以独立频率运行，不受MASTER同步脉冲影响
	API_PPG_SET_CONTINUOUS(ch);

	__HAL_HRTIM_SETCOMPARE(&hhrtim1,HRTIM_CFG_NUM[ch].num,COMPAREUNIT_PAN,period);
	__HAL_HRTIM_TIMER_CLEAR_FLAG(&hhrtim1,	HRTIM_CFG_NUM[ch].num,HRTIM1_PAN_ICR);
	__HAL_HRTIM_TIMER_ENABLE_IT(&hhrtim1, HRTIM_CFG_NUM[ch].num, HRTIM1_PAN_IT);
  __HAL_HRTIM_SETCOUNTER(&hhrtim1, HRTIM_CFG_NUM[ch].num,0);

	HAL_HRTIM_WaveformCountStart(&hhrtim1, HRTIM_CFG_NUM[ch].TIMERID);

	// GPIO同步脉冲 — 检锅炉头已脱离MASTER，不受此脉冲影响。
	// 其他MASTER同步通道正常响应此同步事件。
	API_GPIO_PinPull(HRTIM_SYN_pin,PUPDR_Pulldown);          //等待HRTIM中断停止
	API_GPIO_PinPull(HRTIM_SYN_pin,PUPDR_Pullup);
//	API_PPG_OnOff_NoFault(ch,1);


}

//-----------多通道设置-------------------------------------------------

//void				API_PPG_DeadTime(uint8_t ppgCh,uint8_t upDts,uint8_t downDts);		//死区时间设置 
//void				API_PPG_setValue(uint8_t ppgCh,PPGvalueDef value);			//设置PWM输出周期
//PPGvalueDef		API_PPG_getValue(uint8_t ppgCh);					//PWM周期返回
//void				API_PPG_OnOff(uint8_t ppgCh,uint8_t flag);						//PWM输出开始
//uint8_t			API_PPG_BkFlag(uint8_t ppgCh);						//得到BKFLAG

void		API_HRTIM_SetDmaHandle(uint32_t* addr)
{	
	  hhrtim1.hdmaTimerB = (DMA_HandleTypeDef *)addr;
}	