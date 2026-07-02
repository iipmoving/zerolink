/*************************************************************************************************
* Copyright <2024> <Icore Technology (Nanjing) Co.,Ltd>
* All Rights Reserved,
*
* Redistribution and use in source and binary forms, with or without modification, are permitted
* provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice, this list of
*    conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright notice, this list of
*    conditions and the following disclaimer in the documentation and/or other materials provided
*    with the distribution.
* 3. Neither the name of the copyright holder nor the names of its contributors may be used to
*    endorse or promote products derived from this software without specific prior written
*    permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
* GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
**************************************************************************************************
@file
**************************************************************************************************
*                                              rx32g4xx
*                                          Library Function
*
*                                   Copyright 2024, RX Tech, Corp.
*                                        All Rights Reserved
*
*
*   Project : rx32g4xx
*   File    : rx32g4xx_hal_clc.h
*   By      : RX_DV_Team
**************************************************************************************************
*/



#ifndef _RX32G4XX_HAL_CLC_H_
#define _RX32G4XX_HAL_CLC_H_

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_common.h"
#include "rx32g4xx_hal_def.h"
#include "rx32g4xx_hal_conf.h"

/* Exported types ------------------------------------------------------------*/

/** @addtogroup CLC
  * @{
  */

/******************************************************************************/
/*                               CLC Structures                               */
/******************************************************************************/
  
/** @defgroup CLC_Structure_Definitions Template Structure Definitions
  * @{
  */
  
/** @defgroup CLC_InitTypeDef CLC InitTypeDef
  * @ingroup  CLC_Structure_Definitions 
  * @brief    CLC initial structure definition.
  * @{
  */  
typedef struct
{
    uint32_t Mode;                        /*!< CLC logic mode
                                               The parameter should be one of the @ref CLC_Logic_Mode */
    uint32_t DataSource1;                 /*!< CLC input data source 1
                                               The parameter should be one of the @ref CLC_Data_Source1 */
    uint32_t DataSource2;                 /*!< CLC input data source 2
                                               The parameter should be one of the @ref CLC_Data_Source2 */
    uint32_t DataSource3;                 /*!< CLC input data source 3
                                               The parameter should be one of the @ref CLC_Data_Source3 */
    uint32_t DataSource4;                 /*!< CLC input data source 4
                                               The parameter should be one of the @ref CLC_Data_Source4 */
    uint32_t OutputPolarity;              /*!< CLC output polarity
                                               The value should be SET or RESET */
} CLC_InitTypeDef;
/** 
  * end of CLC_InitTypeDef @}  
  */  
  
/** @defgroup CLC_GateConfTypeDef CLC GateConfTypeDef
  * @ingroup  CLC_Structure_Definitions 
  * @brief    CLC gate configure structure definition. The value can be configured with CLC_ConfigGate()
  * @{
  */
typedef struct 
{
    uint32_t Data1T;                       /*!< CLC data1 positive enable
                                                The value should be SET or RESET */
    uint32_t Data1N;                       /*!< CLC data1 negative enable
                                                The value should be SET or RESET */
    uint32_t Data2T;                       /*!< CLC data2 positive enable
                                                The value should be SET or RESET */
    uint32_t Data2N;                       /*!< CLC data2 negative enable
                                                The value should be SET or RESET */
    uint32_t Data3T;                       /*!< CLC data3 positive enable
                                                The value should be SET or RESET */
    uint32_t Data3N;                       /*!< CLC data3 negative enable
                                                The value should be SET or RESET */
    uint32_t Data4T;                       /*!< CLC data4 positive enable
                                                The value should be SET or RESET */
    uint32_t Data4N;                       /*!< CLC data4 negative enable
                                                The value should be SET or RESET */
    uint32_t GatePolarity;                 /*!< CLC gate output polarity
                                                The value should be SET or RESET */
} CLC_GateConfTypeDef; 
/** 
  * end of CLC_GateConfTypeDef @}  
  */

/** @defgroup CLC_HandleTypeDef CLC HandleTypeDef
  * @ingroup  CLC_Structure_Definitions 
  * @brief    CLC handler type definition.
  * @{
  */
typedef struct __CLC_HandleTypeDef
{
  CLC_TypeDef                   *Instance;              /*!< Register base address */

  CLC_InitTypeDef               Init;                   /*!< CLC required parameters @ref CLC_InitTypeDef */

  HAL_LockTypeDef               Lock;                   /*!< CLC locking object @ref HAL_LockTypeDef */
  
} CLC_HandleTypeDef;
/** 
  * end of CLC_HandleTypeDef @}  
  */   
/** 
  * end of CLC_Structure_Definitions @}  
  */

/******************************************************************************/
/*                                CLC Parameters                              */
/******************************************************************************/
  
/** @defgroup CLC_Parameter_Definitions CLC Parameter Definitions
  * @{
  */
  
/** @defgroup CLC_Logic_Mode CLC Logic Mode
  * @ingroup  CLC_Parameter_Definitions
  * @{
  */
/**
  * @brief  CLC logic function definition
  */
#define CLC_MODE_FOUR_INPUT_AND_OR                                    0x00000000
#define CLC_MODE_FOUR_INPUT_OR_XOR                                    (                                        CLC_CON_MODE_BIT0)
#define CLC_MODE_FOUR_INPUT_AND                                       (                    CLC_CON_MODE_BIT1                    )
#define CLC_MODE_SR_LATCH                                             (                    CLC_CON_MODE_BIT1 | CLC_CON_MODE_BIT0)
#define CLC_MODE_SINGLE_INPUT_WITH_S_AND_R                            (CLC_CON_MODE_BIT2                                        )
#define CLC_MODE_TWO_INPUT_WITH_R                                     (CLC_CON_MODE_BIT2                     | CLC_CON_MODE_BIT0)
#define CLC_MODE_JK_WITH_R                                            (CLC_CON_MODE_BIT2 | CLC_CON_MODE_BIT1                    )
#define CLC_MODE_SINGLE_INPUT_TRANSPARENT_LATCH_WITH_S_AND_R          (CLC_CON_MODE_BIT2 | CLC_CON_MODE_BIT1 | CLC_CON_MODE_BIT0)  
/** 
  * end of CLC_Logic_Mode @}  
  */
  
/** @defgroup CLC_Data_Source1 CLC Data Source1
  * @ingroup  CLC_Parameter_Definitions
  * @{
  */
/**
  * @brief CLC input data selection 1
  */
#define CLC_DS1_CLC_INA                                            0x00000000
#define CLC_DS1_SYSCLK                                                  (                                      CLC_SEL_DS1_BIT0)
#define CLC_DS1_CLC3_OUT                                              (                   CLC_SEL_DS1_BIT1                   )
#define CLC_DS1_LSI                                               (                   CLC_SEL_DS1_BIT1 | CLC_SEL_DS1_BIT0)
#define CLC_DS1_MCO                                                   (CLC_SEL_DS1_BIT2                                      )
#define CLC_DS1_TIM1_OC5                                              (CLC_SEL_DS1_BIT2                    | CLC_SEL_DS1_BIT0)
#define CLC_DS1_HRTIM1_CHA1                                            (CLC_SEL_DS1_BIT2 | CLC_SEL_DS1_BIT1                   )
#define CLC_DS1_TIM2_OC1                                              (CLC_SEL_DS1_BIT2 | CLC_SEL_DS1_BIT1 | CLC_SEL_DS1_BIT0)
/** 
  * end of CLC_Data_Source1 @}  
  */
  
/** @defgroup CLC_Data_Source2 CLC Data Source2
  * @ingroup  CLC_Parameter_Definitions
  * @{
  */
/**
  * @brief CLC input data selection 2
  */
#define CLC_DS2_CLC_INB                                            0x00000000
#define CLC_DS2_HRTIM1_CHB1                                            (                                      CLC_SEL_DS2_BIT0)
#define CLC_DS2_COMP1_OUT                                              (                   CLC_SEL_DS2_BIT1                   )
#define CLC_DS2_UART1_TX                                             (                   CLC_SEL_DS2_BIT1 | CLC_SEL_DS2_BIT0)
#define CLC_DS2_COMP4_OUT                                              (CLC_SEL_DS2_BIT2                                      )
#define CLC_DS2_TIM8_OC5                                              (CLC_SEL_DS2_BIT2                    | CLC_SEL_DS2_BIT0)
#define CLC_DS2_HRTIM1_CHC1                                            (CLC_SEL_DS2_BIT2 | CLC_SEL_DS2_BIT1                   )
#define CLC_DS2_TIM3_OC1                                              (CLC_SEL_DS2_BIT2 | CLC_SEL_DS2_BIT1 | CLC_SEL_DS2_BIT0)
/** 
  * end of CLC_Data_Source2 @}  
  */
  
/** @defgroup CLC_Data_Source3 CLC Data Source3
  * @ingroup  CLC_Parameter_Definitions
  * @{
  */
/**
  * @brief CLC input data selection 3
  */
#define CLC_DS3_CLC_INC                                            0x00000000
#define CLC_DS3_CLC1_OUT                                              (                                      CLC_SEL_DS3_BIT0)
#define CLC_DS3_COMP2_OUT                                              (                   CLC_SEL_DS3_BIT1                   )
#define CLC_DS3_SPI1_TX                                               (                   CLC_SEL_DS3_BIT1 | CLC_SEL_DS3_BIT0)
#define CLC_DS3_UART1_RX                                             (CLC_SEL_DS3_BIT2                                      )
#define CLC_DS3_CLC4_OUT                                              (CLC_SEL_DS3_BIT2                    | CLC_SEL_DS3_BIT0)
#define CLC_DS3_HRTIM1_CHD1                                            (CLC_SEL_DS3_BIT2 | CLC_SEL_DS3_BIT1                   )
#define CLC_DS3_TIM4_OC1                                              (CLC_SEL_DS3_BIT2 | CLC_SEL_DS3_BIT1 | CLC_SEL_DS3_BIT0)
/** 
  * end of CLC_Data_Source3 @}  
  */
  
/** @defgroup CLC_Data_Source4 CLC Data Source4
  * @ingroup  CLC_Parameter_Definitions
  * @{
  */
/**
  * @brief CLC input data selection 4
  */
#define CLC_DS4_TIM15_TRGO                                            0x00000000
#define CLC_DS4_CLC2_OUT                                              (                                      CLC_SEL_DS4_BIT0)
#define CLC_DS4_COMP3_OUT                                              (                   CLC_SEL_DS4_BIT1                   )
#define CLC_DS4_SPI1_RX                                               (                   CLC_SEL_DS4_BIT1 | CLC_SEL_DS4_BIT0)
#define CLC_DS4_HRTIM1_CHE1                                            (CLC_SEL_DS4_BIT2                                      )
#define CLC_DS4_CLC_IND                                            (CLC_SEL_DS4_BIT2                    | CLC_SEL_DS4_BIT0)
#define CLC_DS4_HRTIM1_CHF1                                            (CLC_SEL_DS4_BIT2 | CLC_SEL_DS4_BIT1                   )
#define CLC_DS4_TIM5_OC1                                              (CLC_SEL_DS4_BIT2 | CLC_SEL_DS4_BIT1 | CLC_SEL_DS4_BIT0)
/** 
  * end of CLC_Data_Source4 @}  
  */
  
/** @defgroup CLC_Gate CLC Gate
  * @ingroup  CLC_Parameter_Definitions
  * @{
  */
/**
  * @brief CLC calculate gate
  */  
#define CLC_Gate_1                                                    0x00000001
#define CLC_Gate_2                                                    0x00000002
#define CLC_Gate_3                                                    0x00000003
#define CLC_Gate_4                                                    0x00000004
/** 
  * end of CLC_Gate @}  
  */
   
/** @defgroup CLC_Flag CLC Flag
  * @ingroup  CLC_Parameter_Definitions
  * @{
  */
/**
  * @brief CLC flag
  */  
#define CLC_FLAG_POS                                                  CLC_CON_INTPIF
#define CLC_FLAG_NEG                                                  CLC_CON_INTNIF
/** 
  * end of CLC_Flag @}  
  */
   
/** @defgroup CLC_Interrupt CLC Interrupt
  * @ingroup  CLC_Parameter_Definitions
  * @{
  */
/**
  * @brief CLC interrupt
  */  
#define CLC_IT_POS                                                    CLC_CON_INTP
#define CLC_IT_NEG                                                    CLC_CON_INTN
/** 
  * end of CLC_Interrupt @}  
  */
/** 
  * end of CLC_Parameter_Definitions @}  
  */


/******************************************************************************/
/*                                  CLC Macro                                 */
/******************************************************************************/
/** @defgroup CLC_Macro_Definitions CLC Macro Definitions
  * @{
  */  
/**
  * @brief  Enable the CLC peripheral
  * @param  __HANDLE__: CLC handle
  * @retval None
  */
#define __HAL_CLC_ENABLE(__HANDLE__)           (SET_BIT((__HANDLE__)->Instance->CON, CLC_CON_LCEN))
  
/**
  * @brief  Disable the CLC peripheral
  * @param  __HANDLE__: CLC handle
  * @retval None
  */
#define __HAL_CLC_DISABLE(__HANDLE__)           (CLEAR_BIT((__HANDLE__)->Instance->CON, CLC_CON_LCEN))

/**
  * @brief  Get the status of CLC peripheral
  * @param  __HANDLE__: CLC handle
  * @retval The status of CLCx
  */
  #define __HAL_CLC_GET_ENABLED(__HANDLE__)       ((((__HANDLE__)->Instance->CON & CLC_CON_LCEN) == CLC_CON_LCEN) ? 1U : 0U)
  
/**
  * @brief  Enable the CLC output port
  * @param  __HANDLE__: CLC handle
  * @retval None
  */
#define __HAL_CLC_ENABLE_OUTPUT_PORT(__HANDLE__)           (SET_BIT((__HANDLE__)->Instance->CON, CLC_CON_LCOE))
  
/**
  * @brief  Disable the CLC output port
  * @param  __HANDLE__: CLC handle
  * @retval None
  */
#define __HAL_CLC_DISABLE_OUTPUT_PORT(__HANDLE__)           (CLEAR_BIT((__HANDLE__)->Instance->CON, CLC_CON_LCOE))

/**
  * @brief  Enable the CLC interrupt
  * @param  __HANDLE__: CLC handle
  * @param  __INTERRUPT__: CLC interrupt
  *         This parameter can be any combination of the following values:
  *         @arg CLC_IT_POS: CLC positive edge interrupt
  *         @arg CLC_IT_NEG: CLC negative edge interrupt
  * @retval None
  */
#define __HAL_CLC_ENABLE_IT(__HANDLE__, __INTERRUPT__)           (SET_BIT((__HANDLE__)->Instance->CON, __INTERRUPT__))

/**
  * @brief  Disable the CLC interrupt
  * @param  __HANDLE__: CLC handle
  * @param  __INTERRUPT__: CLC interrupt
  *         This parameter can be any combination of the following values:
  *         @arg CLC_IT_POS: CLC positive edge interrupt
  *         @arg CLC_IT_NEG: CLC negative edge interrupt
  * @retval None
  */
#define __HAL_CLC_DISABLE_IT(__HANDLE__, __INTERRUPT__)           (CLEAR_BIT((__HANDLE__)->Instance->CON, __INTERRUPT__))

/**
  * @brief  Get the CLC flag state
  * @param  __HANDLE__: CLC handle
  * @param  __FLAG__:   CLC flag
  *         @arg CLC_FLAG_POS: CLC positive edge interrupt flag
  *         @arg CLC_FlAG_NEG: CLC negative edge interrupt flag
  * @retval Status
  *         @arg SET (flag set)
  *         @arg RESET (flag reset)
  */
#define __HAL_CLC_GET_FLAG(__HANDLE__, __FLAG__)           ((((__HANDLE__)->Instance->CON) & (__FLAG__)) == (__FLAG__))

/**
  * @brief  Clear the CLC flag state
  * @param  __HANDLE__: CLC handle
  * @param  __FLAG__:   CLC flag
  *         @arg CLC_FLAG_POS: CLC positive edge interrupt flag
  *         @arg CLC_FlAG_NEG: CLC negative edge interrupt flag
  * @retval None
  */
#define __HAL_CLC_CLEAR_FLAG(__HANDLE__, __FLAG__)         (CLEAR_BIT((__HANDLE__)->Instance->CON, (__FLAG__)))

/**
  * @brief  Get the CLC output state
  * @param  __HANDLE__: CLC handle
  * @retval Status
  *         @arg SET (output set)
  *         @arg RESET (output reset)
  */
#define __HAL_CLC_GET_OUTPUT(__HANDLE__)                   (((((__HANDLE__)->Instance->CON) & CLC_CON_LCOUT) == CLC_CON_LCOUT) ? 1U : 0U)


/**
  * @brief  Calculate the gate position of __GATE__
  * @param  __GATE__: CLC gate number
  * @retval VAL: value of the gate position
  */
#define __HAL_CLC_GATE_POS(__GATE__)                        ((__GATE__-0x1U) << 0x3)

/**
  * @brief  Check if the parameter INSTANCE is valid
  * @param  INSTANCE
  * @retval Status
  *         @arg SET (valid)
  *         @arg RESET (invalid)
  */
#define IS_CLC_INSTANCE(INSTANCE)                  (((INSTANCE) == CLC1) || \
                                                    ((INSTANCE) == CLC2) || \
                                                    ((INSTANCE) == CLC3) || \
                                                    ((INSTANCE) == CLC4))

/**
  * @brief  Check if the parameter CHANNEL is valid
  * @param  CHANNEL
  * @retval Status
  *         @arg SET (valid)
  *         @arg RESET (invalid)
  */
#define IS_CLC_GATE(CHANNEL)                       (((CHANNEL) == CLC_Gate_1) || \
                                                    ((CHANNEL) == CLC_Gate_2) || \
                                                    ((CHANNEL) == CLC_Gate_3) || \
                                                    ((CHANNEL) == CLC_Gate_4))
                                                    
/**
  * @brief  Check if the parameter MODE is valid
  * @param  MODE
  * @retval Status
  *         @arg SET (valid)
  *         @arg RESET (invalid)
  */
#define IS_CLC_MODE(MODE)                          (((MODE) == CLC_MODE_FOUR_INPUT_AND_OR                             ) || \
                                                    ((MODE) == CLC_MODE_FOUR_INPUT_OR_XOR                             ) || \
                                                    ((MODE) == CLC_MODE_FOUR_INPUT_AND                                ) || \
                                                    ((MODE) == CLC_MODE_SR_LATCH                                      ) || \
                                                    ((MODE) == CLC_MODE_SINGLE_INPUT_WITH_S_AND_R                     ) || \
                                                    ((MODE) == CLC_MODE_TWO_INPUT_WITH_R                              ) || \
                                                    ((MODE) == CLC_MODE_JK_WITH_R                                     ) || \
                                                    ((MODE) == CLC_MODE_SINGLE_INPUT_TRANSPARENT_LATCH_WITH_S_AND_R   ))
                                                    
/**
  * @brief  Check if the parameter DATA_SOURCE1 is valid
  * @param  DATA_SOURCE1
  * @retval Status
  *         @arg SET (valid)
  *         @arg RESET (invalid)
  */
#define IS_CLC_DATA_SOURCE1(DATA_SOURCE1)          (((DATA_SOURCE1) == CLC_DS1_CLC_INA     ) || \
                                                    ((DATA_SOURCE1) == CLC_DS1_SYSCLK           ) || \
                                                    ((DATA_SOURCE1) == CLC_DS1_CLC3_OUT       ) || \
                                                    ((DATA_SOURCE1) == CLC_DS1_LSI        ) || \
                                                    ((DATA_SOURCE1) == CLC_DS1_MCO            ) || \
                                                    ((DATA_SOURCE1) == CLC_DS1_TIM1_OC5       ) || \
                                                    ((DATA_SOURCE1) == CLC_DS1_HRTIM1_CHA1     ) || \
                                                    ((DATA_SOURCE1) == CLC_DS1_TIM2_OC1       ))
                                                    
/**
  * @brief  Check if the parameter DATA_SOURCE2 is valid
  * @param  DATA_SOURCE2
  * @retval Status
  *         @arg SET (valid)
  *         @arg RESET (invalid)
  */
#define IS_CLC_DATA_SOURCE2(DATA_SOURCE2)          (((DATA_SOURCE2) == CLC_DS2_CLC_INB      ) || \
                                                    ((DATA_SOURCE2) == CLC_DS2_HRTIM1_CHB1      ) || \
                                                    ((DATA_SOURCE2) == CLC_DS2_COMP1_OUT        ) || \
                                                    ((DATA_SOURCE2) == CLC_DS2_UART1_TX       ) || \
                                                    ((DATA_SOURCE2) == CLC_DS2_COMP4_OUT        ) || \
                                                    ((DATA_SOURCE2) == CLC_DS2_TIM8_OC5        ) || \
                                                    ((DATA_SOURCE2) == CLC_DS2_HRTIM1_CHC1      ) || \
                                                    ((DATA_SOURCE2) == CLC_DS2_TIM3_OC1        ))
                                                    
/**
  * @brief  Check if the parameter DATA_SOURCE3 is valid
  * @param  DATA_SOURCE3
  * @retval Status
  *         @arg SET (valid)
  *         @arg RESET (invalid)
  */
#define IS_CLC_DATA_SOURCE3(DATA_SOURCE3)          (((DATA_SOURCE3) == CLC_DS3_CLC_INC    ) || \
                                                    ((DATA_SOURCE3) == CLC_DS3_CLC1_OUT      ) || \
                                                    ((DATA_SOURCE3) == CLC_DS3_COMP2_OUT      ) || \
                                                    ((DATA_SOURCE3) == CLC_DS3_SPI1_TX       ) || \
                                                    ((DATA_SOURCE3) == CLC_DS3_UART1_RX     ) || \
                                                    ((DATA_SOURCE3) == CLC_DS3_CLC4_OUT      ) || \
                                                    ((DATA_SOURCE3) == CLC_DS3_HRTIM1_CHD1    ) || \
                                                    ((DATA_SOURCE3) == CLC_DS3_TIM4_OC1      ))
                                                    
/**
  * @brief  Check if the parameter DATA_SOURCE4 is valid
  * @param  DATA_SOURCE4
  * @retval Status
  *         @arg SET (valid)
  *         @arg RESET (invalid)
  */
#define IS_CLC_DATA_SOURCE4(DATA_SOURCE4)          (((DATA_SOURCE4) == CLC_DS4_TIM15_TRGO    ) || \
                                                    ((DATA_SOURCE4) == CLC_DS4_CLC2_OUT      ) || \
                                                    ((DATA_SOURCE4) == CLC_DS4_COMP3_OUT      ) || \
                                                    ((DATA_SOURCE4) == CLC_DS4_SPI1_RX       ) || \
                                                    ((DATA_SOURCE4) == CLC_DS4_HRTIM1_CHE1    ) || \
                                                    ((DATA_SOURCE4) == CLC_DS4_CLC_IND    ) || \
                                                    ((DATA_SOURCE4) == CLC_DS4_HRTIM1_CHF1    ) || \
                                                    ((DATA_SOURCE4) == CLC_DS4_TIM5_OC1      ))
/**                                                                        
  * end of CLC_Macro_Definitions @}                                        
  */
  
/******************************************************************************/
/*                                CLC Functions                               */
/******************************************************************************/

/** @defgroup CLC_Function_Definitions CLC Function Definitions
  * @{
  */
HAL_StatusTypeDef            HAL_CLC_Init(CLC_HandleTypeDef* hclc);
HAL_StatusTypeDef            HAL_CLC_DeInit(CLC_HandleTypeDef* hclc);
void                         HAL_CLC_MspInit(CLC_HandleTypeDef* hclc);
void                         HAL_CLC_MspDeInit(CLC_HandleTypeDef* hclc);

HAL_StatusTypeDef            HAL_CLC_Start(CLC_HandleTypeDef* hclc);
HAL_StatusTypeDef            HAL_CLC_Stop(CLC_HandleTypeDef* hclc);
HAL_StatusTypeDef            HAL_CLC_Start_IT(CLC_HandleTypeDef* hclc, uint32_t IT);
HAL_StatusTypeDef            HAL_CLC_Stop_IT(CLC_HandleTypeDef* hclc);
                            
uint32_t                     HAL_CLC_GetValue(CLC_HandleTypeDef* hclc);
                            
void                         HAL_CLC_IRQHandler(CLC_HandleTypeDef* hclc);
void                         HAL_CLC_CalCpltCallback(CLC_HandleTypeDef* hclc);
void                         HAL_CLC_ErrorCallback(CLC_HandleTypeDef *hclc); 
   
HAL_StatusTypeDef            HAL_CLC_ConfigGate(CLC_HandleTypeDef* hclc, CLC_GateConfTypeDef* gConfig, uint32_t gate);
/** 
  * end of CLC_Function_Definitions @}  
  */
/** 
  * end of CLC @}  
  */

#ifdef __cplusplus
}
#endif

#endif /* _RX32G4XX_HAL_CLC_H_ */

