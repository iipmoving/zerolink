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
*   File    : rx32g4xx_hal_comp.c
*   By      : RX_DV_Team
**************************************************************************************************
*/


/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal_conf.h"
#include "rx32g4xx_hal_comp.h"

/** @defgroup COMP COMP
  * @{
  */ 
  
#ifdef HAL_COMP_MODULE_ENABLED
  
/* Exported types ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/** @defgroup COMP_Private_Constants
  * @{
  */

/* Delay for COMP startup time.                                               */
#define COMP_DELAY_STARTUP_US              (5UL)      /*!< Delay for COMP startup time */

/* Delay for COMP voltage scaler stabilization time.                          */
#define COMP_DELAY_VOLTAGE_SCALER_STAB_US  (200UL)  /*!< Delay for COMP voltage scaler stabilization time */

#define COMP_OUTPUT_LEVEL_BITOFFSET_POS    (30UL)

/**
  * end of COMP_Private_Constants @}
  */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/** @addtogroup COMP_Function_Definitions COMP Function Definitions
  * @{
  */
  
/**
  * @brief  Initialize the COMP according to the specified
  *         parameters in the COMP_InitTypeDef and initialize the associated handle.
  * @note   If the selected comparator is locked, initialization can't be performed.
  *         To unlock the configuration, perform a system reset.
  * @param  hcomp  COMP handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_COMP_Init(COMP_HandleTypeDef *hcomp)
{
    uint32_t tmp_csr;
    uint32_t exti_line;
    __IO uint32_t wait_loop_index = 0UL;
    HAL_StatusTypeDef status = HAL_OK;
    
    /* Check the COMP handle allocation and lock status */
    if (hcomp == NULL)
    {
        status = HAL_ERROR;
    }
    else if (__HAL_COMP_IS_LOCKED(hcomp))
    {
        status = HAL_ERROR;
    }
    else
    {
        /* Check the parameters */
        assert_param(IS_COMP_ALL_INSTANCE(hcomp->Instance));
        assert_param(IS_COMP_INPUT_PLUS(hcomp->Init.InputPlus));
        assert_param(IS_COMP_INPUT_MINUS(hcomp->Instance, hcomp->Init.InputMinus));
        assert_param(IS_COMP_OUTPUTPOL(hcomp->Init.OutputPol));
        assert_param(IS_COMP_Rising_HYSTERESIS(hcomp->Init.Rising_Hysteresis));
        assert_param(IS_COMP_Falling_HYSTERESIS(hcomp->Init.Falling_Hysteresis));      
        assert_param(IS_COMP_BLANKINGSRC_INSTANCE( hcomp->Init.BlankingSrc));      
        assert_param(IS_COMP_TRIGGERMODE(hcomp->Init.TriggerMode));
        assert_param(IS_COMP_IT(hcomp->Init.IT));

    if (hcomp->State == HAL_COMP_STATE_RESET)
    {
        /* Allocate lock resource and initialize it */
        hcomp->Lock = HAL_UNLOCKED;
    
        /* Set COMP error code to none */
        COMP_CLEAR_ERRORCODE(hcomp);
    
#if (USE_HAL_COMP_REGISTER_CALLBACKS == 1)
        /* Init the COMP Callback settings */
        hcomp->TriggerCallback = HAL_COMP_TriggerCallback; /* Legacy weak callback */
    
        if (hcomp->MspInitCallback == NULL)
        {
            hcomp->MspInitCallback = HAL_COMP_MspInit; /* Legacy weak MspInit  */
        }
    
        /* Init the low level hardware */
        /* Note: Internal control clock of the comparators must                 */
        /*       be enabled in "HAL_COMP_MspInit()"                             */
        /*       using "__HAL_RCC_SYSCFG_CLK_ENABLE()".                         */
        hcomp->MspInitCallback(hcomp);
#else
        /* Init the low level hardware */
        /* Note: Internal control clock of the comparators must                 */
        /*       be enabled in "HAL_COMP_MspInit()"                             */
        /*       using "__HAL_RCC_SYSCFG_CLK_ENABLE()".                         */
        HAL_COMP_MspInit(hcomp);
#endif /* USE_HAL_COMP_REGISTER_CALLBACKS */
        }
    
        /* Set COMP parameters */
        tmp_csr = (hcomp->Init.InputMinus
                | hcomp->Init.InputPlus
                | hcomp->Init.BlankingSrc
                | hcomp->Init.Rising_Hysteresis
                | hcomp->Init.Falling_Hysteresis
                | hcomp->Init.OutputPol
                | hcomp->Init.OFLT
                );
    
        /* Set parameters in COMP register */
        /* Note: Update all bits except read-only, lock and enable bits */
        MODIFY_REG(hcomp->Instance->CR,
                COMP_CR_INM_SEL       | COMP_CR_INP_SEL |
                COMP_CR_POL           | COMP_CR_RHYST   |
                COMP_CR_FHYST         | COMP_CR_OFLT    |
                COMP_CR_BLANKING_SEL  | COMP_CR_BRGEN,
                tmp_csr
                );
    
        /* Get the EXTI line corresponding to the selected COMP instance */
        exti_line = COMP_GET_EXTI_LINE(hcomp->Instance);
    
        /* Manage EXTI settings */
        if ((hcomp->Init.TriggerMode & (COMP_EXTI_IT | COMP_EXTI_EVENT)) != 0UL)
        {
            /* Configure EXTI rising edge */
            if ((hcomp->Init.TriggerMode & COMP_EXTI_RISING) != 0UL)
            {
                __HAL_COMP_EXTI_ENABLE_RISING_EDGE(hcomp->Instance);
            }
            else
            {
                __HAL_COMP_EXTI_DISABLE_RISING_EDGE(hcomp->Instance);
            }
        
            /* Configure EXTI falling edge */
            if ((hcomp->Init.TriggerMode & COMP_EXTI_FALLING) != 0UL)
            {
                __HAL_COMP_EXTI_ENABLE_FALLING_EDGE(hcomp->Instance);
            }
            else
            {
                __HAL_COMP_EXTI_DISABLE_FALLING_EDGE(hcomp->Instance);
            }
        
            /* Clear COMP EXTI pending bit (if any) */
            CLEAR_BIT(EXTI->PR, exti_line);
        
            /* Configure EXTI event mode */
            if ((hcomp->Init.TriggerMode & COMP_EXTI_EVENT) != 0UL)
            {
                __HAL_COMP_EXTI_ENABLE_EVENT(hcomp->Instance);
            }
            else
            {
                __HAL_COMP_EXTI_DISABLE_EVENT(hcomp->Instance);
            }
        
            /* Configure EXTI interrupt mode */
            if ((hcomp->Init.TriggerMode & COMP_EXTI_IT) != 0UL)
            {
                __HAL_COMP_EXTI_ENABLE_IT(hcomp->Instance);
            }
            else
            {
                __HAL_COMP_EXTI_DISABLE_IT(hcomp->Instance);
            }
        }
        else
        {
            /* Disable EXTI event mode */
            __HAL_COMP_EXTI_DISABLE_EVENT(hcomp->Instance);
        
            /* Disable EXTI interrupt mode */
            __HAL_COMP_EXTI_DISABLE_IT(hcomp->Instance);
        }
        
        if (hcomp->Init.IT  != 0UL)
        {
            /* Configure IT */
            __HAL_COMP_ENABLE_IT(hcomp, hcomp->Init.IT);
        }
    
        /* Set HAL COMP handle state */
        /* Note: Transition from state reset to state ready,                      */
        /*       otherwise (coming from state ready or busy) no state update.     */
        if (hcomp->State == HAL_COMP_STATE_RESET)
        {
            hcomp->State = HAL_COMP_STATE_READY;
        }
    }
    
    return status;
}

/**
  * @brief  DeInitialize the COMP peripheral.
  * @note   Deinitialization cannot be performed if the COMP configuration is locked.
  *         To unlock the configuration, perform a system reset.
  * @param  hcomp  COMP handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_COMP_DeInit(COMP_HandleTypeDef *hcomp)
{
    HAL_StatusTypeDef status = HAL_OK;
    
    /* Check the COMP handle allocation and lock status */
    if (hcomp == NULL)
    {
        status = HAL_ERROR;
    }
    else if (__HAL_COMP_IS_LOCKED(hcomp))
    {
        status = HAL_ERROR;
    }
    else
    {
        /* Check the parameter */
        assert_param(IS_COMP_ALL_INSTANCE(hcomp->Instance));
    
        /* Set COMP_CSR register to reset value */
        CLEAR_BIT(hcomp->Instance->CR, ( COMP_CR_EN | COMP_CR_RIE | COMP_CR_FIE | COMP_CR_INM_SEL
                                       | COMP_CR_INP_SEL | COMP_CR_BASEN | COMP_CR_CHDIS | COMP_CR_CHDIS_BLANK
                                       | COMP_CR_OFLT | COMP_CR_POL | COMP_CR_RHYST | COMP_CR_HIE | COMP_CR_BLANKING_SEL
                                       | COMP_CR_BRGEN | COMP_CR_LIE | COMP_CR_FHYST | COMP_CR_HIF | COMP_CR_RIF
                                       | COMP_CR_FIF | COMP_CR_LIF | COMP_CR_LOCK));
                                       
        SET_BIT(hcomp->Instance->CR, COMP_CR_BLANK_EN);
    
#if (USE_HAL_COMP_REGISTER_CALLBACKS == 1)
        if (hcomp->MspDeInitCallback == NULL)
        {
            hcomp->MspDeInitCallback = HAL_COMP_MspDeInit; /* Legacy weak MspDeInit  */
        }
    
        /* DeInit the low level hardware: GPIO, RCC clock, NVIC */
        hcomp->MspDeInitCallback(hcomp);
#else
        /* DeInit the low level hardware: GPIO, RCC clock, NVIC */
        HAL_COMP_MspDeInit(hcomp);
#endif /* USE_HAL_COMP_REGISTER_CALLBACKS */
    
        /* Set HAL COMP handle state */
        hcomp->State = HAL_COMP_STATE_RESET;
    
        /* Release Lock */
        __HAL_UNLOCK(hcomp);
    }
    
    return status;
}

/**
  * @brief  Initialize the COMP MSP.
  * @param  hcomp  COMP handle
  * @retval None
  */
__weak void HAL_COMP_MspInit(COMP_HandleTypeDef *hcomp)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(hcomp);
    
    /* NOTE : This function should not be modified, when the callback is needed,
                the HAL_COMP_MspInit could be implemented in the user file
    */
}

/**
  * @brief  DeInitialize the COMP MSP.
  * @param  hcomp  COMP handle
  * @retval None
  */
__weak void HAL_COMP_MspDeInit(COMP_HandleTypeDef *hcomp)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(hcomp);
    
    /* NOTE : This function should not be modified, when the callback is needed,
                the HAL_COMP_MspDeInit could be implemented in the user file
    */
}

#if (USE_HAL_COMP_REGISTER_CALLBACKS == 1)
/**
  * @brief  Register a User COMP Callback
  *         To be used instead of the weak predefined callback
  * @param  hcomp Pointer to a COMP_HandleTypeDef structure that contains
  *                the configuration information for the specified COMP.
  * @param  CallbackID ID of the callback to be registered
  *         This parameter can be one of the following values:
  *          @arg @ref HAL_COMP_TRIGGER_CB_ID Trigger callback ID
  *          @arg @ref HAL_COMP_MSPINIT_CB_ID MspInit callback ID
  *          @arg @ref HAL_COMP_MSPDEINIT_CB_ID MspDeInit callback ID
  * @param  pCallback pointer to the Callback function
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_COMP_RegisterCallback(COMP_HandleTypeDef *hcomp, HAL_COMP_CallbackIDTypeDef CallbackID,
                                            pCOMP_CallbackTypeDef pCallback)
{
    HAL_StatusTypeDef status = HAL_OK;
    
    if (pCallback == NULL)
    {
        /* Update the error code */
        hcomp->ErrorCode |= HAL_COMP_ERROR_INVALID_CALLBACK;
    
        return HAL_ERROR;
    }
    
    if (HAL_COMP_STATE_READY == hcomp->State)
    {
        switch (CallbackID)
        {
            case HAL_COMP_TRIGGER_CB_ID :
                hcomp->TriggerCallback = pCallback;
                break;
        
            case HAL_COMP_MSPINIT_CB_ID :
                hcomp->MspInitCallback = pCallback;
                break;
        
            case HAL_COMP_MSPDEINIT_CB_ID :
                hcomp->MspDeInitCallback = pCallback;
                break;
        
            default :
                /* Update the error code */
                hcomp->ErrorCode |= HAL_COMP_ERROR_INVALID_CALLBACK;
        
                /* Return error status */
                status = HAL_ERROR;
                break;
        }
    }
    else if (HAL_COMP_STATE_RESET == hcomp->State)
    {
        switch (CallbackID)
        {
            case HAL_COMP_MSPINIT_CB_ID :
                hcomp->MspInitCallback = pCallback;
                break;
        
            case HAL_COMP_MSPDEINIT_CB_ID :
                hcomp->MspDeInitCallback = pCallback;
                break;
        
            default :
                /* Update the error code */
                hcomp->ErrorCode |= HAL_COMP_ERROR_INVALID_CALLBACK;
        
                /* Return error status */
                status = HAL_ERROR;
                break;
        }
    }
    else
    {
        /* Update the error code */
        hcomp->ErrorCode |= HAL_COMP_ERROR_INVALID_CALLBACK;
    
        /* Return error status */
        status =  HAL_ERROR;
    }
    
    return status;
}

/**
  * @brief  Unregister a COMP Callback
  *         COMP callback is redirected to the weak predefined callback
  * @param  hcomp Pointer to a COMP_HandleTypeDef structure that contains
  *                the configuration information for the specified COMP.
  * @param  CallbackID ID of the callback to be unregistered
  *         This parameter can be one of the following values:
  *          @arg @ref HAL_COMP_TRIGGER_CB_ID Trigger callback ID
  *          @arg @ref HAL_COMP_MSPINIT_CB_ID MspInit callback ID
  *          @arg @ref HAL_COMP_MSPDEINIT_CB_ID MspDeInit callback ID
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_COMP_UnRegisterCallback(COMP_HandleTypeDef *hcomp, HAL_COMP_CallbackIDTypeDef CallbackID)
{
    HAL_StatusTypeDef status = HAL_OK;
    
    if (HAL_COMP_STATE_READY == hcomp->State)
    {
        switch (CallbackID)
        {
            case HAL_COMP_TRIGGER_CB_ID :
                hcomp->TriggerCallback = HAL_COMP_TriggerCallback;         /* Legacy weak callback */
                break;
        
            case HAL_COMP_MSPINIT_CB_ID :
                hcomp->MspInitCallback = HAL_COMP_MspInit;                 /* Legacy weak MspInit */
                break;
        
            case HAL_COMP_MSPDEINIT_CB_ID :
                hcomp->MspDeInitCallback = HAL_COMP_MspDeInit;             /* Legacy weak MspDeInit */
                break;
        
            default :
                /* Update the error code */
                hcomp->ErrorCode |= HAL_COMP_ERROR_INVALID_CALLBACK;
        
                /* Return error status */
                status =  HAL_ERROR;
                break;
        }
    }
    else if (HAL_COMP_STATE_RESET == hcomp->State)
    {
        switch (CallbackID)
        {
            case HAL_COMP_MSPINIT_CB_ID :
                hcomp->MspInitCallback = HAL_COMP_MspInit;                 /* Legacy weak MspInit */
                break;
        
            case HAL_COMP_MSPDEINIT_CB_ID :
                hcomp->MspDeInitCallback = HAL_COMP_MspDeInit;             /* Legacy weak MspDeInit */
                break;
        
            default :
                /* Update the error code */
                hcomp->ErrorCode |= HAL_COMP_ERROR_INVALID_CALLBACK;
        
                /* Return error status */
                status =  HAL_ERROR;
                break;
        }
    }
    else
    {
        /* Update the error code */
        hcomp->ErrorCode |= HAL_COMP_ERROR_INVALID_CALLBACK;
    
        /* Return error status */
        status =  HAL_ERROR;
    }
    
    return status;
}

#endif /* USE_HAL_COMP_REGISTER_CALLBACKS */

/**
  * @brief  Start the comparator.
  * @param  hcomp  COMP handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_COMP_Start(COMP_HandleTypeDef *hcomp)
{
    __IO uint32_t wait_loop_index = 0UL;
    HAL_StatusTypeDef status = HAL_OK;
    
    /* Check the COMP handle allocation and lock status */
    if (hcomp == NULL)
    {
        status = HAL_ERROR;
    }
    else if (__HAL_COMP_IS_LOCKED(hcomp))
    {
        status = HAL_ERROR;
    }
    else
    {
        /* Check the parameter */
        assert_param(IS_COMP_ALL_INSTANCE(hcomp->Instance));
    
        if (hcomp->State == HAL_COMP_STATE_READY)
        {
            /* Enable the selected comparator */
            SET_BIT(hcomp->Instance->CR, COMP_CR_EN);
        
            /* Set HAL COMP handle state */
            hcomp->State = HAL_COMP_STATE_BUSY;
        
            /* Delay for COMP startup time */
            /* Wait loop initialization and execution */
            /* Note: Variable divided by 2 to compensate partially                  */
            /*       CPU processing cycles.                                         */
            /* Note: In case of system low frequency (below 1Mhz), short delay      */
            /*       of startup time (few us) is within CPU processing cycles       */
            /*       of following instructions.                                     */
            wait_loop_index = (COMP_DELAY_STARTUP_US * (SystemCoreClock / (1000000UL * 2UL)));
            while (wait_loop_index != 0UL)
            {
                wait_loop_index--;
            }
        }
        else
        {
            status = HAL_ERROR;
        }
    }
    
    return status;
}

/**
  * @brief  Stop the comparator.
  * @param  hcomp  COMP handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_COMP_Stop(COMP_HandleTypeDef *hcomp)
{
    HAL_StatusTypeDef status = HAL_OK;
    
    /* Check the COMP handle allocation and lock status */
    if (hcomp == NULL)
    {
        status = HAL_ERROR;
    }
    else if (__HAL_COMP_IS_LOCKED(hcomp))
    {
        status = HAL_ERROR;
    }
    else
    {
        /* Check the parameter */
        assert_param(IS_COMP_ALL_INSTANCE(hcomp->Instance));
    
        /* Check compliant states: HAL_COMP_STATE_READY or HAL_COMP_STATE_BUSY    */
        /* (all states except HAL_COMP_STATE_RESET and except locked status.      */
        if (hcomp->State != HAL_COMP_STATE_RESET)
        {
            /* Disable the selected comparator */
            CLEAR_BIT(hcomp->Instance->CR, COMP_CR_EN);
    
            /* Set HAL COMP handle state */
            hcomp->State = HAL_COMP_STATE_READY;
        }
        else
        {
            status = HAL_ERROR;
        }
    }
    
    return status;
}

/**
  * @brief  Start the comparator with interrupt enabled.
  * @param  hcomp  COMP handle
  * @param  interrupt The parameter can be the combinition of the following arguments
  *         @arg COMP_IT_HIGH
  *         @arg COMP_IT_LOW
  *         @arg COMP_IT_RISING
  *         @arg COMP_IT_FALLING
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_COMP_Start_IT(COMP_HandleTypeDef *hcomp, uint32_t Interrupt)
{
    __IO uint32_t wait_loop_index = 0UL;
    HAL_StatusTypeDef status = HAL_OK;
    
    /* Check the COMP handle allocation and lock status */
    if (hcomp == NULL)
    {
        status = HAL_ERROR;
    }
    else if (__HAL_COMP_IS_LOCKED(hcomp))
    {
        status = HAL_ERROR;
    }
    else
    {
        /* Check the parameter */
        assert_param(IS_COMP_ALL_INSTANCE(hcomp->Instance));
    
        if (hcomp->State == HAL_COMP_STATE_READY)
        {
            /* Enable Interrupt */
            SET_BIT(hcomp->Instance->CR, Interrupt);
            
            /* Enable the selected comparator */
            SET_BIT(hcomp->Instance->CR, COMP_CR_EN);
        
            /* Set HAL COMP handle state */
            hcomp->State = HAL_COMP_STATE_BUSY;
        
            /* Delay for COMP startup time */
            /* Wait loop initialization and execution */
            /* Note: Variable divided by 2 to compensate partially                  */
            /*       CPU processing cycles.                                         */
            /* Note: In case of system low frequency (below 1Mhz), short delay      */
            /*       of startup time (few us) is within CPU processing cycles       */
            /*       of following instructions.                                     */
            wait_loop_index = (COMP_DELAY_STARTUP_US * (SystemCoreClock / (1000000UL * 2UL)));
            while (wait_loop_index != 0UL)
            {
                wait_loop_index--;
            }
        }
        else
        {
            status = HAL_ERROR;
        }
    }
    
    return status;
}

/**
  * @brief  Stop the comparator.
  * @param  hcomp  COMP handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_COMP_Stop_IT(COMP_HandleTypeDef *hcomp, uint32_t Interrupt)
{
    HAL_StatusTypeDef status = HAL_OK;
    
    /* Check the COMP handle allocation and lock status */
    if (hcomp == NULL)
    {
        status = HAL_ERROR;
    }
    else if (__HAL_COMP_IS_LOCKED(hcomp))
    {
        status = HAL_ERROR;
    }
    else
    {
        /* Check the parameter */
        assert_param(IS_COMP_ALL_INSTANCE(hcomp->Instance));
    
        /* Check compliant states: HAL_COMP_STATE_READY or HAL_COMP_STATE_BUSY    */
        /* (all states except HAL_COMP_STATE_RESET and except locked status.      */
        if (hcomp->State != HAL_COMP_STATE_RESET)
        {
            /* Disable Interrupt */
            CLEAR_BIT(hcomp->Instance->CR, Interrupt);
            
            /* Disable the selected comparator */
            CLEAR_BIT(hcomp->Instance->CR, COMP_CR_EN);
    
            /* Set HAL COMP handle state */
            hcomp->State = HAL_COMP_STATE_READY;
        }
        else
        {
            status = HAL_ERROR;
        }
    }
    
    return status;
}

/**
  * @brief  Comparator IRQ handler.
  * @param  hcomp  COMP handle
  * @retval None
  */
void HAL_COMP_IRQHandler(COMP_HandleTypeDef *hcomp)
{
    /* Get the EXTI line corresponding to the selected COMP instance */
    uint32_t exti_line = COMP_GET_EXTI_LINE(hcomp->Instance);
    uint32_t tmp_comp_exti_flag_set = 0UL;
    
    /* Check COMP EXTI flag */
    if ((EXTI->PR & exti_line) == exti_line)
    {
        tmp_comp_exti_flag_set = 1UL;
    }
    
    if (tmp_comp_exti_flag_set != 0UL)
    {
        /* Clear COMP EXTI line pending bit */
        CLEAR_BIT(EXTI->PR, exti_line);
    
        /* COMP trigger user callback */
#if (USE_HAL_COMP_REGISTER_CALLBACKS == 1)
        hcomp->TriggerCallback(hcomp);
#else
        HAL_COMP_TriggerCallback(hcomp);
#endif /* USE_HAL_COMP_REGISTER_CALLBACKS */
    }
    
    
    if( (hcomp->Instance->CR & COMP_CR_FIF) == COMP_CR_FIF )
    {
        CLEAR_BIT(hcomp->Instance->CR, COMP_CR_FIF);
    }
    
    if( (hcomp->Instance->CR & COMP_CR_RIF) == COMP_CR_RIF )
    {
        CLEAR_BIT(hcomp->Instance->CR, COMP_CR_RIF);
    }
    
    if( (hcomp->Instance->CR & COMP_CR_HIF) == COMP_CR_HIF )
    {
        CLEAR_BIT(hcomp->Instance->CR, COMP_CR_HIF);
    }
    
    if( (hcomp->Instance->CR & COMP_CR_LIF) == COMP_CR_LIF )
    {
        CLEAR_BIT(hcomp->Instance->CR, COMP_CR_LIF);
    }
}

/**
  * @brief  Lock the selected comparator configuration.
  * @note   A system reset is required to unlock the comparator configuration.
  * @note   Locking the comparator from reset state is possible
  *         if __HAL_RCC_SYSCFG_CLK_ENABLE() is being called before.
  * @param  hcomp  COMP handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_COMP_Lock(COMP_HandleTypeDef *hcomp)
{
    HAL_StatusTypeDef status = HAL_OK;
    
    /* Check the COMP handle allocation and lock status */
    if (hcomp == NULL)
    {
        status = HAL_ERROR;
    }
    else if (__HAL_COMP_IS_LOCKED(hcomp))
    {
        status = HAL_ERROR;
    }
    else
    {
        /* Check the parameter */
        assert_param(IS_COMP_ALL_INSTANCE(hcomp->Instance));
    
        /* Set HAL COMP handle state */
        switch (hcomp->State)
        {
            case HAL_COMP_STATE_RESET:
                hcomp->State = HAL_COMP_STATE_RESET_LOCKED;
                break;
            case HAL_COMP_STATE_READY:
                hcomp->State = HAL_COMP_STATE_READY_LOCKED;
                break;
            default: /* HAL_COMP_STATE_BUSY */
                hcomp->State = HAL_COMP_STATE_BUSY_LOCKED;
                break;
        }
    
        /* Set the lock bit corresponding to selected comparator */
        __HAL_COMP_LOCK(hcomp);
    }
    
    return status;
}

/**
  * @brief  Return the output level (high or low) of the selected comparator.
  *         On this STM32 series, comparator 'value' is taken before
  *         polarity and blanking are applied, thus:
  *           - Comparator output is low when the input plus is at a lower
  *             voltage than the input minus
  *           - Comparator output is high when the input plus is at a higher
  *             voltage than the input minus
  * @param  hcomp  COMP handle
  * @retval Returns the selected comparator output level:
  *         @arg COMP_OUTPUT_LEVEL_LOW
  *         @arg COMP_OUTPUT_LEVEL_HIGH
  *
  */
uint32_t HAL_COMP_GetOutputLevel(const COMP_HandleTypeDef *hcomp)
{
    /* Check the parameter */
    assert_param(IS_COMP_ALL_INSTANCE(hcomp->Instance));
    
    return (uint32_t)(READ_BIT(hcomp->Instance->CR, COMP_CR_OUT)
                        >> COMP_OUTPUT_LEVEL_BITOFFSET_POS);
}

/**
  * @brief  Comparator trigger callback.
  * @param  hcomp  COMP handle
  * @retval None
  */
__weak void HAL_COMP_TriggerCallback(COMP_HandleTypeDef *hcomp)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(hcomp);
    
    /* NOTE : This function should not be modified, when the callback is needed,
                the HAL_COMP_TriggerCallback should be implemented in the user file
    */
}

/**
  * @brief  Return the COMP handle state.
  * @param  hcomp  COMP handle
  * @retval HAL state
  */
HAL_COMP_StateTypeDef HAL_COMP_GetState(const COMP_HandleTypeDef *hcomp)
{
    /* Check the COMP handle allocation */
    if (hcomp == NULL)
    {
        return HAL_COMP_STATE_RESET;
    }
    
    /* Check the parameter */
    assert_param(IS_COMP_ALL_INSTANCE(hcomp->Instance));
    
    /* Return HAL COMP handle state */
    return hcomp->State;
}

/**
  * @brief  Return the COMP error code.
  * @param hcomp COMP handle
  * @retval COMP error code
  */
uint32_t HAL_COMP_GetError(const COMP_HandleTypeDef *hcomp)
{
    /* Check the parameters */
    assert_param(IS_COMP_ALL_INSTANCE(hcomp->Instance));
    
    return hcomp->ErrorCode;
}

  
/** 
  * end of COMP_Function_Definitions @}  
  */

#endif /* HAL_OPAMP_MODULE_ENABLED */

/** 
  * end of COMP @}  
  */
