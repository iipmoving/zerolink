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
*   File    : rx32g4xx_hal_opamp.c
*   By      : RX_DV_Team
**************************************************************************************************
*/



/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal.h"

/** @defgroup OPAMP OPAMP
  * @{
  */ 
  
#ifdef HAL_OPAMP_MODULE_ENABLED
  
extern uint32_t Trimoffset_N;
extern uint32_t Trimoffset_P;
/* Exported types ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/** @defgroup OPAMP_Private_Define OPAMP Private Define
  * @{
  */
#define OPAMP_CSR_UPDATE_PARAMETERS_INIT_MASK ( OPAMP_CSR_OPAEN | OPAMP_CSR_FORCEVP | OPAMP_CSR_VMSEL | OPAMP_CSR_OPAHSM | OPAMP_CSR_OPATOEXTEN \
                                               | OPAMP_CSR_VPSEL | OPAMP_CSR_USERTRIM | OPAMP_CSR_CALON | OPAMP_CSR_CALSEL \
                                               | OPAMP_CSR_GAIN | OPAMP_CSR_OUTCONNECT | OPAMP_CSR_TRIMOFFSETN | OPAMP_CSR_TRIMOFFSETP | OPAMP_CSR_TOADCEN)
/* CSR register reset value */
#define OPAMP_CSR_RESET_VALUE             (0x00000000UL)
/* CSR register TRIM value upon reset are factory ones, filter them out from CSR register check */
#define OPAMP_CSR_RESET_CHECK_MASK        (~(OPAMP_CSR_TRIMOFFSETN | OPAMP_CSR_TRIMOFFSETP))
                                    
/**
  * end of OPAMP_Private_Define @}
  */
  
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/** @addtogroup OPAMP_Function_Definitions OPAMP Function Definitions
  * @{
  */

/**
  * @brief  Initializes the OPAMP according to the specified
  *         parameters in the OPAMP_InitTypeDef and initialize the associated handle.
  * @note   If the selected opamp is locked, initialization can't be performed.
  *         To unlock the configuration, perform a system reset.
  * @param  hopamp OPAMP handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OPAMP_Init(OPAMP_HandleTypeDef *hopamp)
{
    HAL_StatusTypeDef status = HAL_OK;
    
    /* Check the OPAMP handle allocation and lock status */
    /* Init not allowed if calibration is ongoing */
    if (hopamp == NULL)
    {
        return HAL_ERROR;
    }
    else if (hopamp->State == HAL_OPAMP_STATE_BUSYLOCKED)
    {
        return HAL_ERROR;
    }
    else if (hopamp->State == HAL_OPAMP_STATE_CALIBBUSY)
    {
        return HAL_ERROR;
    }
    else
    {
        /* Check the parameter */
        assert_param(IS_OPAMP_ALL_INSTANCE(hopamp->Instance));
    
        /* Set OPAMP parameters */
        assert_param(IS_OPAMP_POWERMODE(hopamp->Init.PowerMode));
        assert_param(IS_OPAMP_FUNCTIONAL_NORMALMODE(hopamp->Init.Mode));
    
#if (USE_HAL_OPAMP_REGISTER_CALLBACKS == 1)
        if (hopamp->State == HAL_OPAMP_STATE_RESET)
        {
            if (hopamp->MspInitCallback == NULL)
            {
                hopamp->MspInitCallback               = HAL_OPAMP_MspInit;
            }
        }
#endif /* USE_HAL_OPAMP_REGISTER_CALLBACKS */

        assert_param(IS_OPAMP_TRIMMING(hopamp->Init.UserTrimming));
        
        if ((hopamp->Init.UserTrimming) == OPAMP_TRIMMING_USER)
        {
            assert_param(IS_OPAMP_TRIMMINGVALUE(hopamp->Init.TrimmingValueP));
            assert_param(IS_OPAMP_TRIMMINGVALUE(hopamp->Init.TrimmingValueN));
        }
    
        /* Init SYSCFG and the low level hardware to access opamp */
        __HAL_RCC_SYSCFG_CLK_ENABLE();
    
        if (hopamp->State == HAL_OPAMP_STATE_RESET)
        {
            /* Allocate lock resource and initialize it */
            hopamp->Lock = HAL_UNLOCKED;
        }

#if (USE_HAL_OPAMP_REGISTER_CALLBACKS == 1)
    hopamp->MspInitCallback(hopamp);
#else
    /* Call MSP init function */
    HAL_OPAMP_MspInit(hopamp);
#endif /* USE_HAL_OPAMP_REGISTER_CALLBACKS */

        if ((hopamp->Init.Mode == OPAMP_PGA_MODE) || (hopamp->Init.Mode == OPAMP_FOLLOWER_MODE)|| (hopamp->Init.Mode == OPAMP_OPA_MODE))
        {
            /* Update User Trim config first to be able to modify trimming value afterwards */
            MODIFY_REG(hopamp->Instance->CSR, OPAMP_CSR_USERTRIM, hopamp->Init.UserTrimming);
            MODIFY_REG(hopamp->Instance->CSR, (OPAMP_CSR_VPSEL | OPAMP_CSR_OPAHSM | OPAMP_CSR_TRIMOFFSETP | OPAMP_CSR_TRIMOFFSETN),
                                              (hopamp->Init.VP_SEL | hopamp->Init.PowerMode | (hopamp->Init.TrimmingValueP << 24) | (hopamp->Init.TrimmingValueN << 19) ));
					
            if(hopamp->Init.Mode == OPAMP_PGA_MODE)
            {
                MODIFY_REG(hopamp->Instance->CSR, (OPAMP_CSR_VMSEL | OPAMP_CSR_GAIN), (hopamp->Init.Mode | hopamp->Init.PgaGain));
                MODIFY_REG(hopamp->Instance->CR2, OPAMP_CR2_VCMEN, hopamp->Init.BandgapBuffer);
                MODIFY_REG(hopamp->Instance->CSR,OPAMP_CSR_OUTCONNECT,hopamp->Init.OUTCONNECT);
							
                if(hopamp->Instance == OPAMP1 || hopamp->Instance == OPAMP2)
                {
                    MODIFY_REG(OPAMP1->CR2, OPAMP_CR2_VCMSEL, hopamp->Init.BiasVolRef);
                }
                else
                {
                    MODIFY_REG(OPAMP3->CR2, OPAMP_CR2_VCMSEL, hopamp->Init.BiasVolRef);
                }
                
                if(hopamp->Init.BandgapBuffer == ENABLE)
                {
                    SET_BIT(hopamp->Instance->CR2, OPAMP_CR2_VBGINT);
                }
            }
            else if(hopamp->Init.Mode == OPAMP_FOLLOWER_MODE)
            {
                MODIFY_REG(hopamp->Instance->CSR, OPAMP_CSR_VMSEL, hopamp->Init.Mode);
                CLEAR_BIT(hopamp->Instance->CR2, OPAMP_CR2_VCMEN);
            }
            else  /* OPA mode */
            {
                MODIFY_REG(hopamp->Instance->CSR, OPAMP_CSR_OUTCONNECT, hopamp->Init.OUTCONNECT);
                MODIFY_REG(hopamp->Instance->CSR, OPAMP_CSR_VMSEL, hopamp->Init.VM_SEL);
                CLEAR_BIT(hopamp->Instance->CR2, OPAMP_CR2_VCMEN);
            }
        }
        
        if(hopamp->Init.InternalOutput == ENABLE)
        {
            SET_BIT(hopamp->Instance->CSR, OPAMP_CSR_TOADCEN);
            CLEAR_BIT(hopamp->Instance->CSR, OPAMP_CSR_OPATOEXTEN);
        }
        else
        {
            CLEAR_BIT(hopamp->Instance->CSR, OPAMP_CSR_TOADCEN);
            SET_BIT(hopamp->Instance->CSR, OPAMP_CSR_OPATOEXTEN);
        }
    
        /* Update the OPAMP state*/
        if (hopamp->State == HAL_OPAMP_STATE_RESET)
        {
            /* From RESET state to READY State */
            hopamp->State = HAL_OPAMP_STATE_READY;
        }
        /* else: remain in READY or BUSY state (no update) */
    
        return status;
    }
}


/**
  * @brief  DeInitializes the OPAMP peripheral
  * @note   Deinitialization can't be performed if the OPAMP configuration is locked.
  *         To unlock the configuration, perform a system reset.
  * @param  hopamp OPAMP handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OPAMP_DeInit(OPAMP_HandleTypeDef *hopamp)
{
    HAL_StatusTypeDef status = HAL_OK;
    
    /* Check the OPAMP handle allocation */
    /* DeInit not allowed if calibration is ongoing */
    if (hopamp == NULL)
    {
        status = HAL_ERROR;
    }
    else if (hopamp->State == HAL_OPAMP_STATE_CALIBBUSY)
    {
        status = HAL_ERROR;
    }
    else
    {
        /* Check the parameter */
        assert_param(IS_OPAMP_ALL_INSTANCE(hopamp->Instance));
    
        /* Set OPAMP_CSR register to reset value */
        WRITE_REG(hopamp->Instance->CSR, OPAMP_CSR_RESET_VALUE);
    
        /* DeInit the low level hardware: GPIO, CLOCK and NVIC */
        /* When OPAMP is locked, unlocking can be achieved thanks to */
        /* __HAL_RCC_SYSCFG_CLK_DISABLE() call within HAL_OPAMP_MspDeInit */
        /* Note that __HAL_RCC_SYSCFG_CLK_DISABLE() also disables comparator */
    
#if (USE_HAL_OPAMP_REGISTER_CALLBACKS == 1)
        if (hopamp->MspDeInitCallback == NULL)
        {
            hopamp->MspDeInitCallback = HAL_OPAMP_MspDeInit;
        }
        /* DeInit the low level hardware */
        hopamp->MspDeInitCallback(hopamp);
#else
        HAL_OPAMP_MspDeInit(hopamp);
#endif /* USE_HAL_OPAMP_REGISTER_CALLBACKS */

        if (OPAMP_CSR_RESET_VALUE == (hopamp->Instance->CSR & OPAMP_CSR_RESET_CHECK_MASK))
        {
        /* Update the OPAMP state */
        hopamp->State = HAL_OPAMP_STATE_RESET;
        }
        else /* RESET STATE */
        {
        /* DeInit not complete */
        /* It can be the case if OPAMP was formerly locked */
        status = HAL_ERROR;
    
        /* The OPAMP state is NOT updated */
        }
    
        /* Process unlocked */
        __HAL_UNLOCK(hopamp);
    }

    return status;
}

/**
  * @brief  Initialize the OPAMP MSP.
  * @param  hopamp OPAMP handle
  * @retval None
  */
__weak void HAL_OPAMP_MspInit(OPAMP_HandleTypeDef *hopamp)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(hopamp);
    
    /* NOTE : This function should not be modified, when the callback is needed,
                the HAL_OPAMP_MspInit could be implemented in the user file
    */
}

/**
  * @brief  DeInitialize OPAMP MSP.
  * @param  hopamp OPAMP handle
  * @retval None
  */
__weak void HAL_OPAMP_MspDeInit(OPAMP_HandleTypeDef *hopamp)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(hopamp);
    
    /* NOTE : This function should not be modified, when the callback is needed,
                the HAL_OPAMP_MspDeInit could be implemented in the user file
    */

}


/**
  * @brief  Start the opamp
  * @param  hopamp OPAMP handle
  * @retval HAL status
  */

HAL_StatusTypeDef HAL_OPAMP_Start(OPAMP_HandleTypeDef *hopamp)
{
    HAL_StatusTypeDef status = HAL_OK;
    
    /* Check the OPAMP handle allocation */
    /* Check if OPAMP locked */
    if (hopamp == NULL)
    {
        status = HAL_ERROR;
    }
    else if (hopamp->State == HAL_OPAMP_STATE_BUSYLOCKED)
    {
        status = HAL_ERROR;
    }
    else
    {
        /* Check the parameter */
        assert_param(IS_OPAMP_ALL_INSTANCE(hopamp->Instance));
    
        if (hopamp->State == HAL_OPAMP_STATE_READY)
        {
            if ( hopamp->Instance == OPAMP2 || hopamp->Instance == OPAMP3 )
            {
                SET_BIT(OPAMP2->CSR, OPAMP_CSR_OPAEN);
                SET_BIT(OPAMP3->CSR, OPAMP_CSR_OPAEN);
            }
            else
            {
                SET_BIT(hopamp->Instance->CSR, OPAMP_CSR_OPAEN);
            }
        
            /* Update the OPAMP state*/
            /* From HAL_OPAMP_STATE_READY to HAL_OPAMP_STATE_BUSY */
            hopamp->State = HAL_OPAMP_STATE_BUSY;
        }
        else
        {
            status = HAL_ERROR;
        }
    
    
    }
    return status;
}

/**
  * @brief  Stop the opamp
  * @param  hopamp OPAMP handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OPAMP_Stop(OPAMP_HandleTypeDef *hopamp)
{
    HAL_StatusTypeDef status = HAL_OK;
    
    /* Check the OPAMP handle allocation */
    /* Check if OPAMP locked */
    /* Check if OPAMP calibration ongoing */
    if (hopamp == NULL)
    {
        status = HAL_ERROR;
    }
    else if (hopamp->State == HAL_OPAMP_STATE_BUSYLOCKED)
    {
        status = HAL_ERROR;
    }
    else if (hopamp->State == HAL_OPAMP_STATE_CALIBBUSY)
    {
        status = HAL_ERROR;
    }
    else
    {
        /* Check the parameter */
        assert_param(IS_OPAMP_ALL_INSTANCE(hopamp->Instance));
    
        if (hopamp->State == HAL_OPAMP_STATE_BUSY)
        {
            /* Disable the selected opamp */
            CLEAR_BIT(hopamp->Instance->CSR, OPAMP_CSR_OPAEN);
        
            /* Update the OPAMP state*/
            /* From  HAL_OPAMP_STATE_BUSY to HAL_OPAMP_STATE_READY*/
            hopamp->State = HAL_OPAMP_STATE_READY;
        }
        else
        {
            status = HAL_ERROR;
        }
    }
    return status;
}

/**
  * @brief  Run the self calibration of one OPAMP
  * @note   Calibration is performed in the mode specified in OPAMP init
  *         structure (high-speed mode).
  * @param  hopamp handle
  * @retval Updated offset trimming values (PMOS & NMOS), user trimming is enabled
  * @note   Calibration runs about 25 ms.
  */
void HAL_OPAMP_selfCalibrate(OPAMP_TypeDef *OPAMPx)
{
		__HAL_RCC_SYSCFG_CLK_ENABLE();
		
		uint32_t TRIMOFFSETN = 0;
		uint32_t TRIMOFFSETP = 0;
		uint32_t TRIMOFFSETN_Low 	= 0;
		uint32_t TRIMOFFSETN_High = 31;
		uint32_t TRIMOFFSETP_Low 	= 0;
		uint32_t TRIMOFFSETP_High = 31;
	
		if ( OPAMPx == OPAMP2 || OPAMPx == OPAMP3 )
		{
				SET_BIT(OPAMP2->CSR, OPAMP_CSR_OPAEN);
				SET_BIT(OPAMP3->CSR, OPAMP_CSR_OPAEN);
		}
		else
		{
				SET_BIT(OPAMPx->CSR, OPAMP_CSR_OPAEN);
		}

		MODIFY_REG(OPAMPx->CSR, OPAMP_CSR_OPAHSM, OPAMP_CSR_OPAHSM);
		MODIFY_REG(OPAMPx->CSR, OPAMP_CSR_USERTRIM, OPAMP_CSR_USERTRIM);
		MODIFY_REG(OPAMPx->CSR, OPAMP_CSR_TRIMOFFSETN, 0);										//set N trim code to 0
		MODIFY_REG(OPAMPx->CSR, OPAMP_CSR_TRIMOFFSETP, 0);										//set P trim code to 0
		
		MODIFY_REG(OPAMPx->CSR, OPAMP_CSR_CALSEL, OPAMP_CSR_CALSEL);
		MODIFY_REG(OPAMPx->CSR, OPAMP_CSR_CALON, OPAMP_CSR_CALON);
	
		while((TRIMOFFSETN_High-TRIMOFFSETN_Low) != 1)
		{
				TRIMOFFSETN = TRIMOFFSETN_Low + (TRIMOFFSETN_High - TRIMOFFSETN_Low)/2;
			
				MODIFY_REG(OPAMPx->CSR,OPAMP_CSR_TRIMOFFSETN,TRIMOFFSETN<<19);			//write 3 times for workaround
				HAL_Delay(2);
				MODIFY_REG(OPAMPx->CSR,OPAMP_CSR_TRIMOFFSETN,TRIMOFFSETN<<19);			//write 3 times for workaround
				HAL_Delay(2);
				MODIFY_REG(OPAMPx->CSR,OPAMP_CSR_TRIMOFFSETN,TRIMOFFSETN<<19);			//write 3 times for workaround
				HAL_Delay(2);
			
				if((READ_BIT(OPAMPx->CSR,BIT30) == 0))
				{
					TRIMOFFSETN_Low = TRIMOFFSETN;
				}
				else
				{
					TRIMOFFSETN_High = TRIMOFFSETN;
				}
				HAL_Delay(2);
		}

		MODIFY_REG(OPAMPx->CSR, OPAMP_CSR_CALON, 0);
		MODIFY_REG(OPAMPx->CSR, OPAMP_CSR_CALSEL, OPAMP_VREF_10VDDA);
		MODIFY_REG(OPAMPx->CSR, OPAMP_CSR_CALON, OPAMP_CSR_CALON);
		
		while((TRIMOFFSETP_High-TRIMOFFSETP_Low) != 1)
		{
				TRIMOFFSETP = TRIMOFFSETP_Low + (TRIMOFFSETP_High - TRIMOFFSETP_Low)/2;
			
				MODIFY_REG(OPAMPx->CSR,OPAMP_CSR_TRIMOFFSETP,TRIMOFFSETP<<24);			//write 3 times for workaround
				HAL_Delay(2);
				MODIFY_REG(OPAMPx->CSR,OPAMP_CSR_TRIMOFFSETP,TRIMOFFSETP<<24);			//write 3 times for workaround
				HAL_Delay(2);
				MODIFY_REG(OPAMPx->CSR,OPAMP_CSR_TRIMOFFSETP,TRIMOFFSETP<<24);			//write 3 times for workaround
				HAL_Delay(2);
			
				if((READ_BIT(OPAMPx->CSR,BIT30) == 0))
				{
					TRIMOFFSETP_Low = TRIMOFFSETP;
				}
				else
				{
					TRIMOFFSETP_High = TRIMOFFSETP;
				}
				HAL_Delay(2);
		}
		
		MODIFY_REG(OPAMPx->CSR, OPAMP_CSR_CALON, 0);
		MODIFY_REG(OPAMPx->CSR,OPAMP_CSR_TRIMOFFSETN,TRIMOFFSETN<<19);			/* Write calibration result N */
		MODIFY_REG(OPAMPx->CSR,OPAMP_CSR_TRIMOFFSETP,TRIMOFFSETP<<24);			/* Write calibration result P */
		
		Trimoffset_N = TRIMOFFSETN;
		Trimoffset_P = TRIMOFFSETP;
}

/**
  * @brief  Lock the selected opamp configuration.
  * @param  hopamp OPAMP handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OPAMP_Lock(OPAMP_HandleTypeDef *hopamp)
{
    HAL_StatusTypeDef status = HAL_OK;
    
    /* Check the OPAMP handle allocation */
    /* Check if OPAMP locked */
    /* OPAMP can be locked when enabled and running in normal mode */
    /*   It is meaningless otherwise */
    if (hopamp == NULL)
    {
        status = HAL_ERROR;
    }
    else if (hopamp->State != HAL_OPAMP_STATE_BUSY)
    {
        status = HAL_ERROR;
    }
    else
    {
        /* Check the parameter */
        assert_param(IS_OPAMP_ALL_INSTANCE(hopamp->Instance));
    
        /* Lock OPAMP */
        SET_BIT(hopamp->Instance->CSR, OPAMP_CSR_LOCK);
    
        /* OPAMP state changed to locked */
        hopamp->State = HAL_OPAMP_STATE_BUSYLOCKED;
    }
    return status;
}

/**
  * @brief  Return the OPAMP state
  * @param  hopamp OPAMP handle
  * @retval HAL state
  */
HAL_OPAMP_StateTypeDef HAL_OPAMP_GetState(OPAMP_HandleTypeDef *hopamp)
{
    /* Check the OPAMP handle allocation */
    if (hopamp == NULL)
    {
        return HAL_OPAMP_STATE_RESET;
    }
    
    /* Check the parameter */
    assert_param(IS_OPAMP_ALL_INSTANCE(hopamp->Instance));
    
    return hopamp->State;
}


#if (USE_HAL_OPAMP_REGISTER_CALLBACKS == 1)
/**
  * @brief  Register a User OPAMP Callback
  *         To be used instead of the weak (surcharged) predefined callback
  * @param hopamp : OPAMP handle
  * @param CallbackID : ID of the callback to be registered
  *        This parameter can be one of the following values:
  *          @arg @ref HAL_OPAMP_MSPINIT_CB_ID       OPAMP MspInit callback ID
  *          @arg @ref HAL_OPAMP_MSPDEINIT_CB_ID     OPAMP MspDeInit callback ID
  * @param pCallback : pointer to the Callback function
  * @retval status
  */
HAL_StatusTypeDef HAL_OPAMP_RegisterCallback(OPAMP_HandleTypeDef *hopamp, HAL_OPAMP_CallbackIDTypeDef CallbackId,
                                             pOPAMP_CallbackTypeDef pCallback)
{
    HAL_StatusTypeDef status = HAL_OK;
    
    if (pCallback == NULL)
    {
        return HAL_ERROR;
    }
    
    /* Process locked */
    __HAL_LOCK(hopamp);
    
    if (hopamp->State == HAL_OPAMP_STATE_READY)
    {
        switch (CallbackId)
        {
        case HAL_OPAMP_MSPINIT_CB_ID :
            hopamp->MspInitCallback = pCallback;
            break;
        case HAL_OPAMP_MSPDEINIT_CB_ID :
            hopamp->MspDeInitCallback = pCallback;
            break;
        default :
            /* update return status */
            status =  HAL_ERROR;
            break;
        }
    }
    else if (hopamp->State == HAL_OPAMP_STATE_RESET)
    {
        switch (CallbackId)
        {
        case HAL_OPAMP_MSPINIT_CB_ID :
            hopamp->MspInitCallback = pCallback;
            break;
        case HAL_OPAMP_MSPDEINIT_CB_ID :
            hopamp->MspDeInitCallback = pCallback;
            break;
        default :
            /* update return status */
            status =  HAL_ERROR;
            break;
        }
    }
    else
    {
        /* update return status */
        status =  HAL_ERROR;
    }
    
    /* Release Lock */
    __HAL_UNLOCK(hopamp);
    return status;
}

/**
  * @brief  Unregister a User OPAMP Callback
  *         OPAMP Callback is redirected to the weak (surcharged) predefined callback
  * @param hopamp : OPAMP handle
  * @param CallbackID : ID of the callback to be unregistered
  *        This parameter can be one of the following values:
  *          @arg @ref HAL_OPAMP_MSPINIT_CB_ID              OPAMP MSP Init Callback ID
  *          @arg @ref HAL_OPAMP_MSPDEINIT_CB_ID            OPAMP MSP DeInit Callback ID
  *          @arg @ref HAL_OPAMP_ALL_CB_ID                   OPAMP All Callbacks
  * @retval status
  */

HAL_StatusTypeDef HAL_OPAMP_UnRegisterCallback(OPAMP_HandleTypeDef *hopamp, HAL_OPAMP_CallbackIDTypeDef CallbackId)
{
    HAL_StatusTypeDef status = HAL_OK;
    
    /* Process locked */
    __HAL_LOCK(hopamp);
    
    if (hopamp->State == HAL_OPAMP_STATE_READY)
    {
        switch (CallbackId)
        {
            case HAL_OPAMP_MSPINIT_CB_ID :
                hopamp->MspInitCallback = HAL_OPAMP_MspInit;
                break;
            case HAL_OPAMP_MSPDEINIT_CB_ID :
                hopamp->MspDeInitCallback = HAL_OPAMP_MspDeInit;
                break;
            case HAL_OPAMP_ALL_CB_ID :
                hopamp->MspInitCallback = HAL_OPAMP_MspInit;
                hopamp->MspDeInitCallback = HAL_OPAMP_MspDeInit;
                break;
            default :
                /* update return status */
                status =  HAL_ERROR;
                break;
        }
    }
    else if (hopamp->State == HAL_OPAMP_STATE_RESET)
    {
        switch (CallbackId)
        {
            case HAL_OPAMP_MSPINIT_CB_ID :
                hopamp->MspInitCallback = HAL_OPAMP_MspInit;
                break;
            case HAL_OPAMP_MSPDEINIT_CB_ID :
                hopamp->MspDeInitCallback = HAL_OPAMP_MspDeInit;
                break;
            default :
                /* update return status */
                status =  HAL_ERROR;
                break;
        }
    }
    else
    {
        /* update return status */
        status =  HAL_ERROR;
    }
    
    /* Release Lock */
    __HAL_UNLOCK(hopamp);
    return status;
}

#endif /* USE_HAL_OPAMP_REGISTER_CALLBACKS */
  
/** 
  * end of OPAMP_Function_Definitions @}  
  */
  
#endif /* HAL_OPAMP_MODULE_ENABLED */

/** 
  * end of OPAMP @}  
  */
