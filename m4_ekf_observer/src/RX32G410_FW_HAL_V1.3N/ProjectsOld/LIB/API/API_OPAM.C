
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
* File         : API_OPAM.c
* By           : MOVING
*********************************************************************************************************
*/



/* Includes ------------------------------------------------------------------*/

#include "rx32g4xx_config_def.h"
#include "rx32g4xx_hal.h"
#include "system_init.h"
#include "system_bsp.h"

#include	"stdint.h"
#include	"API_OPAMP.H"
/* Private typedef -----------------------------------------------------------*/
typedef struct
{
   OPAMP_TypeDef           *Instance;                      /*!< OPAMP instance's registers base address   */
   OPAMP_InitTypeDef       *Init;                           /*!< OPAMP required parameters */

	
}OPAMP_TOTAL_InitTypeDef;	

typedef struct
{
	uint32_t CSR;
	uint32_t CR2;	
}OPAMP_SAVE_DEF;	


/* Private variables ---------------------------------------------------------*/

OPAMP_HandleTypeDef CUR1_OP;
OPAMP_HandleTypeDef CUR2_OP;
OPAMP_HandleTypeDef CUR3_OP;
OPAMP_HandleTypeDef CUR4_OP;

OPAMP_SAVE_DEF		OP_CUR1,OP_FAN;
//DAC_HandleTypeDef hdac1;
//DMA_HandleTypeDef hdma_dac1_ch1;


//uint32_t Trimoffset_N = 0;
//uint32_t Trimoffset_P = 0;

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/


OPAMP_InitTypeDef		OPAMP3_InitType={


    OPAMP_POWERMODE_HIGHSPEED,/*uint32_t PowerMode;                   !< Specifies the power mode Normal or High Speed.
                                                This parameter must be a value of @ref OPAMP_PowerMode */
    
    OPAMP_PGA_MODE,/*uint32_t Mode;                        !< Specifies the OPAMP mode
                                                This parameter must be a value of @ref OPAMP_Mode
                                                mode is either Follower or PGA */
                                                
    OPAMP_VPSEL_P1,/*uint32_t VP_SEL;                      !< Specifified the OPAMP VP selection
                                                This parameter must be a value of @ref OPAMP_NonInvertingInput */
                                                
    OPAMP_VMSEL_N1,/*uint32_t VM_SEL;                      !< Specifified the OPAMP VM selection
                                                This parameter must be a value of @ref OPAMP_InvertingInput */
    
    ENABLE,/*uint32_t BandgapBuffer;               !< Specifified the OPAMP bangap buffer mode
                                                This parameter must be a value of ENABLE or DISABLE */
    
    OPAMP_REFERENCE_FROM_VBG,	/*uint32_t BiasVolRef;                  !< Specifified the OPAMP Bias voltage reference
                                                This parameter must be a value of @ref OPAMP_VOL_REF */
    
    DISABLE,										/*FunctionalState InternalOutput;       !< Specifies the configuration of the internal output from OPAMP to ADC.
                                                This parameter can be ENABLE or DISABLE
                                                Note: When this output is enabled, regular output to I/O is disabled */
    
    OPAMP_PGA_GAIN_32,					/*uint32_t PgaGain;                     !< Specifies the gain in PGA mode
                                                i.e. when mode is OPAMP_PGA_MODE.
                                                This parameter must be a value of @ref OPAMP_PgaGain */
    
    OPAMP_TRIMMING_USER,				/*uint32_t UserTrimming;                !< Specifies the trimming mode
                                                This parameter must be a value of @ref OPAMP_UserTrimming
                                                UserTrimming is either factory or user trimming */
    
    0,												/*uint32_t TrimmingValueP;              !< Specifies the offset trimming value (PMOS)
                                                i.e. when UserTrimming is OPAMP_TRIMMING_USER.
                                                This parameter must be a number between Min_Data = 1 and Max_Data = 31 */
    
    0,												/*uint32_t TrimmingValueN;              !< Specifies the offset trimming value (NMOS)
                                                i.e. when UserTrimming is OPAMP_TRIMMING_USER.
                                                This parameter must be a number between Min_Data = 1 and Max_Data = 31 */
    
    OPAMP_CONNECT_TO_VINN1,			/*uint32_t OUTCONNECT; 				  !< Specifies the OUT CONNECT in PGA mode i.e. when mode is OPAMP_PGA_MODE.
                                                This parameter must be a value of @ref OPAMP_OUT_CONNECT*/
	


};

OPAMP_InitTypeDef	const	OPAMP1_InitType={


    OPAMP_POWERMODE_HIGHSPEED,/*uint32_t PowerMode;                   !< Specifies the power mode Normal or High Speed.
                                                This parameter must be a value of @ref OPAMP_PowerMode */
    
    OPAMP_OPA_MODE,/*uint32_t Mode;                        !< Specifies the OPAMP mode
                                                This parameter must be a value of @ref OPAMP_Mode
                                                mode is either Follower or PGA */
                                                
    OPAMP_VPSEL_P1,/*uint32_t VP_SEL;                      !< Specifified the OPAMP VP selection
                                                This parameter must be a value of @ref OPAMP_NonInvertingInput */
                                                
    OPAMP_VMSEL_N1,/*uint32_t VM_SEL;                      !< Specifified the OPAMP VM selection
                                                This parameter must be a value of @ref OPAMP_InvertingInput */
    
    ENABLE,/*uint32_t BandgapBuffer;               !< Specifified the OPAMP bangap buffer mode
                                                This parameter must be a value of ENABLE or DISABLE */
    
    OPAMP_REFERENCE_FROM_VBG,	/*uint32_t BiasVolRef;                  !< Specifified the OPAMP Bias voltage reference
                                                This parameter must be a value of @ref OPAMP_VOL_REF */
    
    DISABLE,										/*FunctionalState InternalOutput;       !< Specifies the configuration of the internal output from OPAMP to ADC.
                                                This parameter can be ENABLE or DISABLE
                                                Note: When this output is enabled, regular output to I/O is disabled */
    
    OPAMP_PGA_GAIN_16,					/*uint32_t PgaGain;                     !< Specifies the gain in PGA mode
                                                i.e. when mode is OPAMP_PGA_MODE.
                                                This parameter must be a value of @ref OPAMP_PgaGain */
    
    OPAMP_TRIMMING_USER,				/*uint32_t UserTrimming;                !< Specifies the trimming mode
                                                This parameter must be a value of @ref OPAMP_UserTrimming
                                                UserTrimming is either factory or user trimming */
    
    0,												/*uint32_t TrimmingValueP;              !< Specifies the offset trimming value (PMOS)
                                                i.e. when UserTrimming is OPAMP_TRIMMING_USER.
                                                This parameter must be a number between Min_Data = 1 and Max_Data = 31 */
    
    0,												/*uint32_t TrimmingValueN;              !< Specifies the offset trimming value (NMOS)
                                                i.e. when UserTrimming is OPAMP_TRIMMING_USER.
                                                This parameter must be a number between Min_Data = 1 and Max_Data = 31 */
    
    OPAMP_NO_CONNECT,						/*uint32_t OUTCONNECT; 				  !< Specifies the OUT CONNECT in PGA mode i.e. when mode is OPAMP_PGA_MODE.
                                                This parameter must be a value of @ref OPAMP_OUT_CONNECT*/
	
};



OPAMP_InitTypeDef	const	OPAMP_FLLOWER_InitType={


    OPAMP_POWERMODE_HIGHSPEED,/*uint32_t PowerMode;                   !< Specifies the power mode Normal or High Speed.
                                                This parameter must be a value of @ref OPAMP_PowerMode */
    
    OPAMP_FOLLOWER_MODE,/*uint32_t Mode;                        !< Specifies the OPAMP mode
                                                This parameter must be a value of @ref OPAMP_Mode
                                                mode is either Follower or PGA */
                                                
    OPAMP_VPSEL_P1,/*uint32_t VP_SEL;                      !< Specifified the OPAMP VP selection
                                                This parameter must be a value of @ref OPAMP_NonInvertingInput */
                                                
    OPAMP_VMSEL_N1,/*uint32_t VM_SEL;                      !< Specifified the OPAMP VM selection
                                                This parameter must be a value of @ref OPAMP_InvertingInput */
    
    ENABLE,/*uint32_t BandgapBuffer;               !< Specifified the OPAMP bangap buffer mode
                                                This parameter must be a value of ENABLE or DISABLE */
    
    OPAMP_REFERENCE_FROM_VBG,	/*uint32_t BiasVolRef;                  !< Specifified the OPAMP Bias voltage reference
                                                This parameter must be a value of @ref OPAMP_VOL_REF */
    
    ENABLE,										/*FunctionalState InternalOutput;  ąŘ±ŐĘäłö     !< Specifies the configuration of the internal output from OPAMP to ADC.
                                                This parameter can be ENABLE or DISABLE
                                                Note: When this output is enabled, regular output to I/O is disabled */
    
    OPAMP_PGA_GAIN_16,					/*uint32_t PgaGain;                     !< Specifies the gain in PGA mode
                                                i.e. when mode is OPAMP_PGA_MODE.
                                                This parameter must be a value of @ref OPAMP_PgaGain */
    
    OPAMP_TRIMMING_USER,				/*uint32_t UserTrimming;                !< Specifies the trimming mode
                                                This parameter must be a value of @ref OPAMP_UserTrimming
                                                UserTrimming is either factory or user trimming */
    
    0,												/*uint32_t TrimmingValueP;              !< Specifies the offset trimming value (PMOS)
                                                i.e. when UserTrimming is OPAMP_TRIMMING_USER.
                                                This parameter must be a number between Min_Data = 1 and Max_Data = 31 */
    
    0,												/*uint32_t TrimmingValueN;              !< Specifies the offset trimming value (NMOS)
                                                i.e. when UserTrimming is OPAMP_TRIMMING_USER.
                                                This parameter must be a number between Min_Data = 1 and Max_Data = 31 */
    
    OPAMP_NO_CONNECT,						/*uint32_t OUTCONNECT; 				  !< Specifies the OUT CONNECT in PGA mode i.e. when mode is OPAMP_PGA_MODE.
                                                This parameter must be a value of @ref OPAMP_OUT_CONNECT*/
	
};





OPAMP_InitTypeDef		FanAd_OpmapInit={


    OPAMP_POWERMODE_HIGHSPEED,/*uint32_t PowerMode;                   !< Specifies the power mode Normal or High Speed.
                                                This parameter must be a value of @ref OPAMP_PowerMode */
    
    OPAMP_PGA_MODE,/*uint32_t Mode;                        !< Specifies the OPAMP mode
                                                This parameter must be a value of @ref OPAMP_Mode
                                                mode is either Follower or PGA */
                                                
    OPAMP_VPSEL_P2,/*uint32_t VP_SEL;                      !< Specifified the OPAMP VP selection
                                                This parameter must be a value of @ref OPAMP_NonInvertingInput */
                                                
    OPAMP_VMSEL_N1,/*uint32_t VM_SEL;                      !< Specifified the OPAMP VM selection
                                                This parameter must be a value of @ref OPAMP_InvertingInput */
    
    ENABLE,/*uint32_t BandgapBuffer;               !< Specifified the OPAMP bangap buffer mode
                                                This parameter must be a value of ENABLE or DISABLE */
    
    OPAMP_REFERENCE_FROM_VBG,	/*uint32_t BiasVolRef;                  !< Specifified the OPAMP Bias voltage reference
                                                This parameter must be a value of @ref OPAMP_VOL_REF */
    
    DISABLE,										/*FunctionalState InternalOutput;       !< Specifies the configuration of the internal output from OPAMP to ADC.
                                                This parameter can be ENABLE or DISABLE
                                                Note: When this output is enabled, regular output to I/O is disabled */
    
    OPAMP_PGA_GAIN_16,					/*uint32_t PgaGain;                     !< Specifies the gain in PGA mode
                                                i.e. when mode is OPAMP_PGA_MODE.
                                                This parameter must be a value of @ref OPAMP_PgaGain */
    
    OPAMP_TRIMMING_USER,				/*uint32_t UserTrimming;                !< Specifies the trimming mode
                                                This parameter must be a value of @ref OPAMP_UserTrimming
                                                UserTrimming is either factory or user trimming */
    
    0,												/*uint32_t TrimmingValueP;              !< Specifies the offset trimming value (PMOS)
                                                i.e. when UserTrimming is OPAMP_TRIMMING_USER.
                                                This parameter must be a number between Min_Data = 1 and Max_Data = 31 */
    
    0,												/*uint32_t TrimmingValueN;              !< Specifies the offset trimming value (NMOS)
                                                i.e. when UserTrimming is OPAMP_TRIMMING_USER.
                                                This parameter must be a number between Min_Data = 1 and Max_Data = 31 */
    
    OPAMP_CONNECT_TO_VINN1,			/*uint32_t OUTCONNECT; 				  !< Specifies the OUT CONNECT in PGA mode i.e. when mode is OPAMP_PGA_MODE.
                                                This parameter must be a value of @ref OPAMP_OUT_CONNECT*/
	


};


#define	CUR1_OpmapInit	OPAMP_FLLOWER_InitType
#define	CUR2_OpmapInit	OPAMP_FLLOWER_InitType
#define	CUR3_OpmapInit	OPAMP_FLLOWER_InitType
#define	CUR4_OpmapInit	OPAMP_FLLOWER_InitType



OPAMP_TOTAL_InitTypeDef	OPAMP1_Type={

	OPAMP1,
	(OPAMP_InitTypeDef*)&CUR1_OpmapInit,
};

OPAMP_TOTAL_InitTypeDef	FanAd_Type={

	OPAMP1,
	(OPAMP_InitTypeDef*)&FanAd_OpmapInit,
};


OPAMP_TOTAL_InitTypeDef	OPAMP2_Type={

	OPAMP2,
	(OPAMP_InitTypeDef*)&CUR2_OpmapInit,
};

OPAMP_TOTAL_InitTypeDef	OPAMP3_Type={

	OPAMP3,
	(OPAMP_InitTypeDef*)&CUR3_OpmapInit,
};
OPAMP_TOTAL_InitTypeDef	OPAMP4_Type={

	OPAMP4,
	(OPAMP_InitTypeDef*)&CUR4_OpmapInit,
};

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Exported variables --------------------------------------------------------*/

uint32_t Trimoffset_N = 0;
uint32_t Trimoffset_P = 0;
/* Exported functions --------------------------------------------------------*/
//static void MX_GPIO_Init(void);
//static void MX_DMA_Init(void);
//static void MX_DAC1_Init(void);
//static void MX_TIM2_Init(void);
//static void MX_OPAMP3_Init(void);

/* Private functions ---------------------------------------------------------*/

__weak	void	Error_Handler(void)
{

}	




/**
  * @brief DAC1 Initialization Function
  * @param None
  * @retval None
  */


uint32_t    API_GetAvgMaxMin(uint32_t* value,uint8_t size)
{
    uint32_t    maxValue=0;
    uint32_t    minValue=0xffffffff; 
    uint32_t    nowValue; 
    uint32_t    sumValue=0;
    for(uint8_t i=0;i<size;i++)
    {
          nowValue=  value[i];
        if(nowValue>maxValue) 
        {
           maxValue=nowValue; 
        }   
        if(nowValue<minValue) 
        {
           minValue=nowValue; 
        }
        sumValue+=nowValue;
        
    }
    sumValue-=minValue;
    sumValue-=maxValue;   
    sumValue/=(size-2);
    return   sumValue;  
}





static void	API_OPAMP_InitByStr(OPAMP_HandleTypeDef* opampHandle,OPAMP_TOTAL_InitTypeDef* opampStr)
{
		opampHandle->Instance=opampStr->Instance;
	
       
#if 0
    uint32_t     trimmingValueN[5];  
    uint32_t     trimmingValueP[5];  


	for(uint8_t i=0;i<5;i++)                //5´ÎČĄµô×î´ó×îĐˇÖµŁ¬ĘŁÓŕČˇĆ˝ľů
        {
           HAL_OPAMP_selfCalibrate(opampHandle->Instance);
					trimmingValueN[i] = Trimoffset_N;
					trimmingValueP[i]=  Trimoffset_P;           
        } 

        
		opampHandle->Init=*(opampStr->Init);
		opampHandle->Init.TrimmingValueN = API_GetAvgMaxMin(trimmingValueN,5);    
		opampHandle->Init.TrimmingValueP = API_GetAvgMaxMin(trimmingValueP,5);    
#else
           HAL_OPAMP_selfCalibrate(opampHandle->Instance);
		opampHandle->Init=*(opampStr->Init);
		opampHandle->Init.TrimmingValueN = Trimoffset_N;    
		opampHandle->Init.TrimmingValueP = Trimoffset_P;    
	
	
#endif	
	
    if (HAL_OPAMP_Init(opampHandle) != HAL_OK)
    {
        Error_Handler();
    }
    /* Enable OPAMP */
    if(HAL_OK != HAL_OPAMP_Start(opampHandle))
    {
        Error_Handler();
    }
}


//#define SET_BIT(REG, BIT)       ((REG) |= (BIT))
//#define CLEAR_BIT(REG, BIT)     ((REG) &= ~(BIT))
//#define READ_BIT(REG, BIT)      ((REG) & (BIT))



/**
  * @brief OPAMP3 Initialization Function
  * @param None
  * @retval None
  */
void API_OPAMP_Init(void)
{
	
	API_OPAMP_InitByStr(&CUR1_OP,&OPAMP1_Type);
	API_OPAMP_InitByStr(&CUR2_OP,&OPAMP2_Type);
	API_OPAMP_InitByStr(&CUR3_OP,&OPAMP3_Type);
	API_OPAMP_InitByStr(&CUR4_OP,&OPAMP4_Type);
	OP_CUR1.CR2=CUR1_OP.Instance->CR2;
	OP_CUR1.CSR=CUR1_OP.Instance->CSR;	
//	API_OPAMP_InitByStr(&CUR1_OP,&FanAd_Type);	
//	OP_FAN.CR2=CUR1_OP.Instance->CR2;
//	OP_FAN.CSR=CUR1_OP.Instance->CSR;		
}

void	API_OPAMP1_Select(uint8_t ch)		//0:OP1P1 CUR1  1:OP1P2  FANAD
{
	if(ch)
	{
//		CUR1_OP.Instance->CR2	=	OP_FAN.CR2;
//		CUR1_OP.Instance->CSR	=	OP_FAN.CSR;
//		SET_BIT(CUR1_OP.Instance->CSR,OPAMP_CSR_VPSEL);//ÇĐ»»OP1P2
	}		
	else
	{


//		CUR1_OP.Instance->CR2=	OP_CUR1.CR2;
//		CUR1_OP.Instance->CSR	=OP_CUR1.CSR ;	
		
//		CLEAR_BIT(CUR1_OP.Instance->CSR,OPAMP_CSR_VPSEL);	//ÇĐ»»OP1P1
	}	


}	



#if 0
/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 0;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 0xFFFF;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
    {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
    {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
}
#endif


///**
//  * Enable DMA controller clock
//  */
//static void MX_DMA_Init(void)
//{

//    /* DMA controller clock enable */
//    __HAL_RCC_DMAMUX1_CLK_ENABLE();
//    __HAL_RCC_DMA1_CLK_ENABLE();

//    /* DMA interrupt init */
//    /* DMA1_Channel1_IRQn interrupt configuration */
//    HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, IRQ_PRIORITY_DMA1_Channel1, 0);
//    HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

//}

/**
* @brief OPAMP MSP Initialization
* This function configures the hardware resources used in this example
* @param hopamp: OPAMP handle pointer
* @retval None
*/
void HAL_OPAMP_MspInit(OPAMP_HandleTypeDef* hopamp)
{
//    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    if(hopamp->Instance==OPAMP3)
    {
//        __HAL_RCC_GPIOA_CLK_ENABLE();
//        __HAL_RCC_GPIOB_CLK_ENABLE();
//        /**OPAMP3 GPIO Configuration
//        PB2     ------> OPAMP3_VINP
//        PB0     ------> OPAMP3_VOUT
//        */
//        GPIO_InitStruct.Pin = GPIO_PIN_2;
//        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
//        GPIO_InitStruct.Pull = GPIO_NOPULL;
//        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
//    
//        GPIO_InitStruct.Pin = GPIO_PIN_0;
//        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
//        GPIO_InitStruct.Pull = GPIO_NOPULL;
//        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    }

}

/**
* @brief OPAMP MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hopamp: OPAMP handle pointer
* @retval None
*/
void HAL_OPAMP_MspDeInit(OPAMP_HandleTypeDef* hopamp)
{
    if(hopamp->Instance==OPAMP3)
    {
    
        /**OPAMP3 GPIO Configuration
        PB2     ------> OPAMP3_VINP
        PB0     ------> OPAMP3_VOUT
        */
//        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_2);
//        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_0);
    }

}


