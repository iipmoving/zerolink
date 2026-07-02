#include "API_comp.h"
#include "rx32g4xx_config_def.h"
#include "rx32g4xx_hal.h"
#include "system_init.h"
#include "system_bsp.h"

#define BLANKING_SEL            COMP_CR_BLANKING_SEL_TIM1_OC5
#define BLANKING_DOUBLE_SEL     COMP_CR_BLANKING_SEL_TIM5_OC3

__attribute__((weak)) void	Error_Handler(void)
{
	
}	


COMP_HandleTypeDef hcomp1;
COMP_HandleTypeDef hcomp2;
COMP_HandleTypeDef hcomp3;
COMP_HandleTypeDef hcomp4;

void HAL_COMP_MspPostInit(COMP_HandleTypeDef *hcomp)
{
//  __HAL_RCC_GPIOA_CLK_ENABLE();
//  __HAL_RCC_GPIOB_CLK_ENABLE();
//  __HAL_RCC_GPIOC_CLK_ENABLE();
//  
  GPIO_InitTypeDef GPIO_InitStruct = {0};
//  
  if(hcomp->Instance == COMP1)
  {
#ifdef	DEBUG_POWER_OUT		  //COMP1O 比较器输出
//    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;		//com1o
//    GPIO_InitStruct.Pull = GPIO_PUPDR_Pullup;
//    GPIO_InitStruct.Speed = GPIO_OSPEEDR_Very_High;
//    GPIO_InitStruct.Alternate = GPIO_AF8;
//    GPIO_InitStruct.Pin = GPIO_PIN_6;         //COMP1_o -> PA6
//    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
#endif    

  }
  else if(hcomp->Instance == COMP2)
  {
	  
#ifdef	DEBUG_POWER_OUT		  //COMP1O 比较器输出	  
//    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;		//com2o
//    GPIO_InitStruct.Pull = GPIO_PUPDR_Pullup;
//    GPIO_InitStruct.Speed = GPIO_OSPEEDR_Very_High;
//    GPIO_InitStruct.Alternate = GPIO_AF8;
//    GPIO_InitStruct.Pin = GPIO_PIN_7;         //COMP2_o -> PA6
//    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
#endif 	  
  }
  else if(hcomp->Instance == COMP3)
  {
//    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;		//com2o
//    GPIO_InitStruct.Pull = GPIO_PUPDR_Pullup;
//    GPIO_InitStruct.Speed = GPIO_OSPEEDR_Very_High;
//    GPIO_InitStruct.Alternate = GPIO_AF8;
//    GPIO_InitStruct.Pin = GPIO_PIN_7;         //COMP1_o -> PA6
//    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  }
  else if(hcomp->Instance == COMP4)
  {
//    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;		//com2o
//    GPIO_InitStruct.Pull = GPIO_PUPDR_Pullup;
//    GPIO_InitStruct.Speed = GPIO_OSPEEDR_Very_High;
//    GPIO_InitStruct.Alternate = GPIO_AF8;
//    GPIO_InitStruct.Pin = GPIO_PIN_1;         //COMP1_o -> PA6
//    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  }
}

void HAL_COMP_MspInit(COMP_HandleTypeDef *hcomp)
{
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  
  HAL_COMP_MspPostInit(hcomp);
}


void API_COMP_Init(void)
{
  hcomp1.Instance = COMP1;
  hcomp1.Init.InputPlus = COMP_INPUT_PLUS_IO1;
  hcomp1.Init.InputMinus = COMP_INPUT_MINUS_DAC1_CH1;
  hcomp1.Init.OutputPol = COMP_OUTPUTPOL_NONINVERTED;
  hcomp1.Init.Falling_Hysteresis = COMP_CR_FHYST_20mV;
  hcomp1.Init.Rising_Hysteresis = COMP_CR_RHYST_0mV;
  hcomp1.Init.BlankingSrc = COMP_CR_BLANKING_SEL_TIM1_OC5;
  hcomp1.Init.TriggerMode = COMP_TRIGGERMODE_NONE;
  hcomp1.Init.IT = COMP_IT_NONE;
  hcomp1.Init.OFLT = COMP_CR_OFLT_VALUE;
  if (HAL_COMP_Init(&hcomp1) != HAL_OK)
  {
      Error_Handler();
  }
  
  hcomp2.Instance = COMP2;
  hcomp2.Init.InputPlus = COMP_INPUT_PLUS_IO1;
  hcomp2.Init.InputMinus = COMP_INPUT_MINUS_DAC1_CH2;
  hcomp2.Init.OutputPol = COMP_OUTPUTPOL_NONINVERTED;
  hcomp2.Init.Falling_Hysteresis = COMP_CR_FHYST_20mV;
  hcomp2.Init.Rising_Hysteresis = COMP_CR_RHYST_0mV;
  hcomp2.Init.BlankingSrc = COMP_CR_BLANKING_SEL_TIM1_OC5;
  hcomp2.Init.TriggerMode = COMP_TRIGGERMODE_NONE;
  hcomp2.Init.IT = COMP_IT_NONE;
  hcomp2.Init.OFLT = COMP_CR_OFLT_VALUE;
  if (HAL_COMP_Init(&hcomp2) != HAL_OK)
  {
      Error_Handler();
  }
  
  hcomp3.Instance = COMP3;
  hcomp3.Init.InputPlus = COMP_INPUT_PLUS_IO1;
  hcomp3.Init.InputMinus = COMP_INPUT_MINUS_DAC2_CH1;
  hcomp3.Init.OutputPol = COMP_OUTPUTPOL_NONINVERTED;
  hcomp3.Init.Falling_Hysteresis = COMP_CR_FHYST_20mV;
  hcomp3.Init.Rising_Hysteresis = COMP_CR_RHYST_0mV;
  hcomp3.Init.BlankingSrc = COMP_CR_BLANKING_SEL_TIM1_OC5;
  hcomp3.Init.TriggerMode = COMP_TRIGGERMODE_NONE;
  hcomp3.Init.IT = COMP_IT_NONE;
  hcomp3.Init.OFLT = COMP_CR_OFLT_VALUE;
  if (HAL_COMP_Init(&hcomp3) != HAL_OK)
  {
      Error_Handler();
  }
  
  hcomp4.Instance = COMP4;
  hcomp4.Init.InputPlus = COMP_INPUT_PLUS_IO1;
  hcomp4.Init.InputMinus = COMP_INPUT_MINUS_DAC2_CH2;
  hcomp4.Init.OutputPol = COMP_OUTPUTPOL_NONINVERTED;
  hcomp4.Init.Falling_Hysteresis = COMP_CR_FHYST_20mV;
  hcomp4.Init.Rising_Hysteresis = COMP_CR_RHYST_0mV;
  hcomp4.Init.BlankingSrc = COMP_CR_BLANKING_SEL_TIM1_OC5;
  hcomp4.Init.TriggerMode = COMP_TRIGGERMODE_NONE;
  hcomp4.Init.IT = COMP_IT_NONE;
  hcomp4.Init.OFLT = COMP_CR_OFLT_VALUE;
  if (HAL_COMP_Init(&hcomp4) != HAL_OK)
  {
      Error_Handler();
  }
  
  HAL_COMP_Start(&hcomp1);
  HAL_COMP_Start(&hcomp2);
  HAL_COMP_Start(&hcomp3);
  HAL_COMP_Start(&hcomp4); 
  
  
}
