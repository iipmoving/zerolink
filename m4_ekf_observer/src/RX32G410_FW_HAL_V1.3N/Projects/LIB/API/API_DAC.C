
#include "rx32g4xx_config_def.h"
#include "rx32g4xx_hal.h"






#include	"API_DAC.h"


#define		DAC_INI_VALUE		0XFF


DAC_HandleTypeDef hdac1;
DAC_HandleTypeDef hdac2;

__weak		void Error_Handler(void)
{}	

/**
  * @brief DAC1 Initialization Function
  * @param None
  * @retval None
  */
void API_DAC_Init(void)
{
    DAC_ChannelConfTypeDef sConfig = {0};
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    /** DAC Initialization
    */
    hdac1.Instance = DAC1;
    if (HAL_DAC_Init(&hdac1) != HAL_OK)
    {
        Error_Handler();
    }
    hdac2.Instance = DAC2;
    if (HAL_DAC_Init(&hdac2) != HAL_OK)
    {
        Error_Handler();
    }
		
		
    /** DAC channel OUT1 config
    */
    sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
    sConfig.DAC_Trigger2 = DAC_TRIGGER_NONE;
    sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_DISABLE;
    sConfig.DAC_UserTrimming = DAC_TRIMMING_FACTORY;
    if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_1) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_2) != HAL_OK)
    {
        Error_Handler();
    }		
		
    if (HAL_DAC_ConfigChannel(&hdac2, &sConfig, DAC_CHANNEL_1) != HAL_OK)
    {
        Error_Handler();
    }	
    if (HAL_DAC_ConfigChannel(&hdac2, &sConfig, DAC_CHANNEL_2) != HAL_OK)
    {
        Error_Handler();
    }	

    /* Set value & Start DAC1 */
    if (HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_8B_R, DAC_INI_VALUE) != HAL_OK)
    {
        /* Setting value Error */
        Error_Handler();
    }
    /* Set value & Start DAC1 */
    if (HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_2, DAC_ALIGN_8B_R, DAC_INI_VALUE) != HAL_OK)
    {
        /* Setting value Error */
        Error_Handler();
    }
    /* Set value & Start DAC1 */
    if (HAL_DAC_SetValue(&hdac2, DAC_CHANNEL_1, DAC_ALIGN_8B_R, DAC_INI_VALUE) != HAL_OK)
    {
        /* Setting value Error */
        Error_Handler();
    }		
    /* Set value & Start DAC1 */
    if (HAL_DAC_SetValue(&hdac2, DAC_CHANNEL_2, DAC_ALIGN_8B_R, DAC_INI_VALUE) != HAL_OK)
    {
        /* Setting value Error */
        Error_Handler();
    }		
		
    if (HAL_DAC_Start(&hdac1, DAC_CHANNEL_1) != HAL_OK)
    {
        /* Start Error */
        Error_Handler();
    }
    if (HAL_DAC_Start(&hdac1, DAC_CHANNEL_2) != HAL_OK)
    {
        /* Start Error */
        Error_Handler();
    }
    if (HAL_DAC_Start(&hdac2, DAC_CHANNEL_1) != HAL_OK)
    {
        /* Start Error */
        Error_Handler();
    }
    if (HAL_DAC_Start(&hdac2, DAC_CHANNEL_2) != HAL_OK)
    {
        /* Start Error */
        Error_Handler();
    }		
}




void	API_DAC_COMP_SetValue(uint8_t ch, uint8_t value)
{	
		DAC_HandleTypeDef* hdacx;
		uint32_t  channel;
	
		switch (ch)
		{
			case	0:		
			hdacx=&hdac1;								//comp1
			channel=DAC_CHANNEL_1;
			
			break;
			case 1:
			hdacx=&hdac1;								//comp2
			channel=DAC_CHANNEL_2;			
					
			break;	
			case	2:		
			hdacx=&hdac2;							//comp3
			channel=DAC_CHANNEL_1;
			
			break;
			case 3:
			hdacx=&hdac2;							//comp4
			channel=DAC_CHANNEL_2;			
					
			break;		
			
			Error_Handler();
			
		}

    if (HAL_DAC_SetValue(hdacx, channel, DAC_ALIGN_8B_R, value) != HAL_OK)
    {
        /* Setting value Error */
        Error_Handler();
    }
	
	 if (HAL_DAC_Start(&hdac1, DAC_CHANNEL_1) != HAL_OK)
    {
        /* Start Error */
        Error_Handler();
    }
	
}

void	API_DAC_COMP_SetValueAll(uint8_t value)
{

		API_DAC_COMP_SetValue(0,value);
		API_DAC_COMP_SetValue(1,value);
		API_DAC_COMP_SetValue(2,value);	
		API_DAC_COMP_SetValue(3,value);	
	

	
}



/**
* @brief DAC MSP Initialization
* This function configures the hardware resources used in this example
* @param hdac: DAC handle pointer
* @retval None
*/
void HAL_DAC_MspInit(DAC_HandleTypeDef* hdac)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(hdac->Instance==DAC1)
    {
        /* Peripheral clock enable */
        __HAL_RCC_DAC1_CLK_ENABLE();
    
//        __HAL_RCC_GPIOA_CLK_ENABLE();
//        /**DAC1 GPIO Configuration
//        PA4     ------> DAC1_OUT1
//        */
//        GPIO_InitStruct.Pin = GPIO_PIN_4;
//        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
//        GPIO_InitStruct.Pull = GPIO_NOPULL;
//        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
    if(hdac->Instance==DAC2)
    {
        /* Peripheral clock enable */
        __HAL_RCC_DAC2_CLK_ENABLE();
    
//        __HAL_RCC_GPIOA_CLK_ENABLE();
//        /**DAC1 GPIO Configuration
//        PA4     ------> DAC1_OUT1
//        */
//        GPIO_InitStruct.Pin = GPIO_PIN_4;
//        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
//        GPIO_InitStruct.Pull = GPIO_NOPULL;
//        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}

/**
* @brief DAC MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hdac: DAC handle pointer
* @retval None
*/
void HAL_DAC_MspDeInit(DAC_HandleTypeDef* hdac)
{
  if(hdac->Instance==DAC1)
  {
        /* Peripheral clock disable */
        __HAL_RCC_DAC1_CLK_DISABLE();
    
        /**DAC1 GPIO Configuration
        PA4     ------> DAC1_OUT1
        */
//        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4);
  }

}

