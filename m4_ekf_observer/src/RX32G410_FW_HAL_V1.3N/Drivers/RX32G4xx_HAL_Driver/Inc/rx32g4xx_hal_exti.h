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
*   File    : rx32g4xx_hal_exti.h
*   By      : RX_DV_Team
**************************************************************************************************
*/


#ifndef _RX32G4XX_HAL_EXTI_H_
#define _RX32G4XX_HAL_EXTI_H_

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal_def.h"


/* Exported types ------------------------------------------------------------*/

/** @addtogroup EXTI
  * @{
  */

/******************************************************************************/
/*                            EXTI Structures                                 */
/******************************************************************************/

/** @defgroup EXTI_Structure_Definitions EXTI Structure Definitions
  * @{
  */

typedef enum
{
  HAL_EXTI_COMMON_CB_ID         = 0x00UL
} EXTI_CallbackIDTypeDef;

/** @defgroup EXTI_HandleTypeDef EXTI HandleTypeDef
  * @ingroup  EXTI_Structure_Definitions
  * @{
  */
/**
  * @brief  EXTI Handle structure definition
  */
typedef struct
{
  uint32_t Line;                    /*!<  Exti line number */
  void (* PendingCallback)(void);   /*!<  Exti pending callback */
} EXTI_HandleTypeDef;
/**
  * end of EXTI_HandleTypeDef @}
  */

/** @defgroup EXTI_ConfigTypeDef EXTI ConfigTypeDef
  * @ingroup  EXTI_Structure_Definitions
  * @{
  */
/**
  * @brief  EXTI Configuration structure definition
  */
typedef struct
{
  uint32_t Line;      /*!< The Exti line to be configured. This parameter
                           can be a value of @ref EXTI_Line */
  uint32_t Mode;      /*!< The Exit Mode to be configured for a core.
                           This parameter can be a combination of @ref EXTI_Mode */
  uint32_t Trigger;   /*!< The Exti Trigger to be configured. This parameter
                           can be a value of @ref EXTI_Trigger */
  uint32_t GPIOSel;   /*!< The Exti GPIO multiplexer selection to be configured.
                           This parameter is only possible for line 0 to 15. It
                           can be a value of @ref EXTI_GPIOSel */
} EXTI_ConfigTypeDef;
/**
  * end of EXTI_ConfigTypeDef @}
  */

/**
  * end of EXTI_Structure_Definitions @}
  */


/******************************************************************************/
/*                            EXTI Parameters                                 */
/******************************************************************************/

/** @defgroup EXTI_Parameter_Definitions EXTI Parameter Definitions
  * @{
  */

/** @defgroup EXTI_Line  EXTI Line
  * @{
  */
#define EXTI_LINE_0                         (EXTI_GPIO     | EXTI_REG1 | 0x00u)
#define EXTI_LINE_1                         (EXTI_GPIO     | EXTI_REG1 | 0x01u)
#define EXTI_LINE_2                         (EXTI_GPIO     | EXTI_REG1 | 0x02u)
#define EXTI_LINE_3                         (EXTI_GPIO     | EXTI_REG1 | 0x03u)
#define EXTI_LINE_4                         (EXTI_GPIO     | EXTI_REG1 | 0x04u)
#define EXTI_LINE_5                         (EXTI_GPIO     | EXTI_REG1 | 0x05u)
#define EXTI_LINE_6                         (EXTI_GPIO     | EXTI_REG1 | 0x06u)
#define EXTI_LINE_7                         (EXTI_GPIO     | EXTI_REG1 | 0x07u)
#define EXTI_LINE_8                         (EXTI_GPIO     | EXTI_REG1 | 0x08u)
#define EXTI_LINE_9                         (EXTI_GPIO     | EXTI_REG1 | 0x09u)
#define EXTI_LINE_10                        (EXTI_GPIO     | EXTI_REG1 | 0x0Au)
#define EXTI_LINE_11                        (EXTI_GPIO     | EXTI_REG1 | 0x0Bu)
#define EXTI_LINE_12                        (EXTI_GPIO     | EXTI_REG1 | 0x0Cu)
#define EXTI_LINE_13                        (EXTI_GPIO     | EXTI_REG1 | 0x0Du)
#define EXTI_LINE_14                        (EXTI_GPIO     | EXTI_REG1 | 0x0Eu)
#define EXTI_LINE_15                        (EXTI_GPIO     | EXTI_REG1 | 0x0Fu)
#define EXTI_LINE_16                        (EXTI_CONFIG   | EXTI_REG1 | 0x10u)
#define EXTI_LINE_17                        (EXTI_CONFIG   | EXTI_REG1 | 0x11u)
#define EXTI_LINE_18                        (EXTI_DIRECT   | EXTI_REG1 | 0x12u)
#define EXTI_LINE_19                        (EXTI_CONFIG   | EXTI_REG1 | 0x13u)
#define EXTI_LINE_20                        (EXTI_CONFIG   | EXTI_REG1 | 0x14u)
#define EXTI_LINE_21                        (EXTI_CONFIG   | EXTI_REG1 | 0x15u)
#define EXTI_LINE_22                        (EXTI_CONFIG   | EXTI_REG1 | 0x16u)
#define EXTI_LINE_23                        (EXTI_DIRECT   | EXTI_REG1 | 0x17u)
#define EXTI_LINE_24                        (EXTI_DIRECT   | EXTI_REG1 | 0x18u)
#define EXTI_LINE_25                        (EXTI_DIRECT   | EXTI_REG1 | 0x19u)
#define EXTI_LINE_26                        (EXTI_DIRECT   | EXTI_REG1 | 0x1Au)
#define EXTI_LINE_27                        (EXTI_DIRECT   | EXTI_REG1 | 0x1Bu)
#define EXTI_LINE_28                        (EXTI_DIRECT   | EXTI_REG1 | 0x1Cu)
#define EXTI_LINE_29                        (EXTI_CONFIG   | EXTI_REG1 | 0x1Du)
#define EXTI_LINE_30                        (EXTI_CONFIG   | EXTI_REG1 | 0x1Eu)
/**
  * end of EXTI_Line @}
  */

/** @defgroup EXTI_Mode  EXTI Mode
  * @{
  */
#define EXTI_MODE_NONE                      0x00000000U
#define EXTI_MODE_INTERRUPT                 0x00000001U
#define EXTI_MODE_EVENT                     0x00000002U
/**
  * end of EXTI_Mode @}
  */

/** @defgroup EXTI_Trigger  EXTI Trigger
  * @{
  */
#define EXTI_TRIGGER_NONE                   0x00000000U
#define EXTI_TRIGGER_RISING                 0x00000001U
#define EXTI_TRIGGER_FALLING                0x00000002U
#define EXTI_TRIGGER_RISING_FALLING         (EXTI_TRIGGER_RISING | EXTI_TRIGGER_FALLING)
/**
  * end of EXTI_Trigger @}
  */

/** @defgroup EXTI_GPIOSel  EXTI GPIOSel
  * @brief
  * @{
  */
#define EXTI_GPIOA                          0x00000000U
#define EXTI_GPIOB                          0x00000001U
#define EXTI_GPIOC                          0x00000002U
#define EXTI_GPIOD                          0x00000003U
#define EXTI_GPIOE                          0x00000004U
#define EXTI_GPIOF                          0x00000005U
/**
  * end of EXTI_GPIOSel @}
  */

/**
  * @brief  EXTI Line property definition
  */
#define EXTI_PROPERTY_SHIFT                 24U
#define EXTI_DIRECT                         (0x01uL << EXTI_PROPERTY_SHIFT)
#define EXTI_CONFIG                         (0x02uL << EXTI_PROPERTY_SHIFT)
#define EXTI_GPIO                           ((0x04uL << EXTI_PROPERTY_SHIFT) | EXTI_CONFIG)
#define EXTI_RESERVED                       (0x08uL << EXTI_PROPERTY_SHIFT)
#define EXTI_PROPERTY_MASK                  (EXTI_DIRECT | EXTI_CONFIG | EXTI_GPIO)

/**
  * @brief  EXTI Register and bit usage
  */
#define EXTI_REG_SHIFT                      16U
#define EXTI_REG1                           (0x00uL << EXTI_REG_SHIFT)
#define EXTI_REG2                           (0x01uL << EXTI_REG_SHIFT)
#define EXTI_REG_MASK                       (EXTI_REG1 | EXTI_REG2)
#define EXTI_PIN_MASK                       0x0000001FU

/**
  * @brief  EXTI Mask for interrupt & event mode
  */
#define EXTI_MODE_MASK                      (EXTI_MODE_EVENT | EXTI_MODE_INTERRUPT)

/**
  * @brief  EXTI Mask for trigger possibilities
  */
#define EXTI_TRIGGER_MASK                   (EXTI_TRIGGER_RISING | EXTI_TRIGGER_FALLING)

/**
  * @brief  EXTI Line number
  */
#define EXTI_LINE_NB                        31UL

/**
  * end of EXTI_Parameter_Definitions @}
  */


/******************************************************************************/
/*                              EXTI Macro                                    */
/******************************************************************************/

/** @defgroup EXTI_Macro_Definitions EXTI Macro Definitions
  * @{
  */

#define IS_EXTI_LINE(__EXTI_LINE__)          ((((__EXTI_LINE__) & ~(EXTI_PROPERTY_MASK | EXTI_REG_MASK | EXTI_PIN_MASK)) == 0x00U) && \
                                              ((((__EXTI_LINE__) & EXTI_PROPERTY_MASK) == EXTI_DIRECT)   || \
                                               (((__EXTI_LINE__) & EXTI_PROPERTY_MASK) == EXTI_CONFIG)   || \
                                               (((__EXTI_LINE__) & EXTI_PROPERTY_MASK) == EXTI_GPIO))    && \
                                              (((__EXTI_LINE__) & (EXTI_REG_MASK | EXTI_PIN_MASK))      < \
                                               (((EXTI_LINE_NB / 32u) << EXTI_REG_SHIFT) | (EXTI_LINE_NB % 32u))))

#define IS_EXTI_MODE(__EXTI_LINE__)          ((((__EXTI_LINE__) & EXTI_MODE_MASK) != 0x00U) && \
                                              (((__EXTI_LINE__) & ~EXTI_MODE_MASK) == 0x00U))

#define IS_EXTI_TRIGGER(__EXTI_LINE__)       (((__EXTI_LINE__) & ~EXTI_TRIGGER_MASK) == 0x00U)

#define IS_EXTI_CONFIG_LINE(__EXTI_LINE__)   (((__EXTI_LINE__) & EXTI_CONFIG) != 0x00U)

#define IS_EXTI_GPIO_PORT(__PORT__)     (((__PORT__) == EXTI_GPIOA) || \
                                         ((__PORT__) == EXTI_GPIOB) || \
                                         ((__PORT__) == EXTI_GPIOC) || \
                                         ((__PORT__) == EXTI_GPIOD) || \
                                         ((__PORT__) == EXTI_GPIOE) || \
                                         ((__PORT__) == EXTI_GPIOF))

#define IS_EXTI_GPIO_PIN(__PIN__)        ((__PIN__) < 16u)

#define IS_EXTI_PENDING_EDGE(__EDGE__)   (((__EDGE__) == EXTI_TRIGGER_RISING)   || \
                                          ((__EDGE__) == EXTI_TRIGGER_FALLING)|| \
                                          ((__EDGE__) == EXTI_TRIGGER_RISING_FALLING))

#define IS_EXTI_CB(__CB__)               ((__CB__) == HAL_EXTI_COMMON_CB_ID)

/**
  * end of EXTI_Macro_Definitions @}
  */


/******************************************************************************/
/*                             EXTI Functions                                 */
/******************************************************************************/

/** @defgroup EXTI_Function_Definitions EXTI Function Definitions
  * @{
  */

/* Configuration functions ****************************************************/
HAL_StatusTypeDef HAL_EXTI_SetConfigLine(EXTI_HandleTypeDef *hexti, EXTI_ConfigTypeDef *pExtiConfig);
HAL_StatusTypeDef HAL_EXTI_GetConfigLine(EXTI_HandleTypeDef *hexti, EXTI_ConfigTypeDef *pExtiConfig);
HAL_StatusTypeDef HAL_EXTI_ClearConfigLine(EXTI_HandleTypeDef *hexti);
HAL_StatusTypeDef HAL_EXTI_RegisterCallback(EXTI_HandleTypeDef *hexti, EXTI_CallbackIDTypeDef CallbackID, void (*pPendingCbfn)(void));
HAL_StatusTypeDef HAL_EXTI_GetHandle(EXTI_HandleTypeDef *hexti, uint32_t ExtiLine);

/* IO operation functions *****************************************************/
void              HAL_EXTI_IRQHandler(EXTI_HandleTypeDef *hexti);
uint32_t          HAL_EXTI_GetPending(EXTI_HandleTypeDef *hexti, uint32_t Edge);
void              HAL_EXTI_ClearPending(EXTI_HandleTypeDef *hexti, uint32_t Edge);
void              HAL_EXTI_GenerateSWI(EXTI_HandleTypeDef *hexti);

/**
  * end of EXTI_Function_Definitions @}
  */


/**
  * end of EXTI @}
  */


#ifdef __cplusplus
}
#endif

#endif /* _RX32G4XX_HAL_EXTI_H_ */
