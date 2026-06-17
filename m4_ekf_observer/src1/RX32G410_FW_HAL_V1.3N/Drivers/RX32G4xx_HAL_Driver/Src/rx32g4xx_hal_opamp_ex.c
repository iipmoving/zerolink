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
*   File    : rx32g4xx_hal_opamp_ex.c
*   By      : RX_DV_Team
**************************************************************************************************
*/



/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal.h"

/** @addtogroup OPAMP
  * @{
  */ 
  
#ifdef HAL_OPAMP_MODULE_ENABLED
  
/* Exported types ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/** @addtogroup OPAMP_Function_Definitions OPAMP Function Definitions
  * @{
  */

/**
  * @brief  Run the self calibration of up to 6 OPAMPs in parallel.
  * @note   Calibration is performed in the mode specified in OPAMP init
  *         structure (mode normal or high-speed).
  * @param  hopamp1 handle
  * @param  hopamp2 handle
  * @param  hopamp3 handle
  * @param  hopamp4 handle
  * @retval HAL status
  * @note   Updated offset trimming values (PMOS & NMOS), user trimming is enabled
  */

HAL_StatusTypeDef HAL_OPAMPEx_SelfCalibrateAll(OPAMP_HandleTypeDef *hopamp1, OPAMP_HandleTypeDef *hopamp2,
                                               OPAMP_HandleTypeDef *hopamp3, OPAMP_HandleTypeDef *hopamp4)
{
    uint32_t trimmingvaluen1 = 0;
    uint32_t trimmingvaluep1 = 0;
    uint32_t trimmingvaluen2 = 0;
    uint32_t trimmingvaluep2 = 0;
    uint32_t trimmingvaluen3 = 0;
    uint32_t trimmingvaluep3 = 0;
    uint32_t trimmingvaluen4 = 0;
    uint32_t trimmingvaluep4 = 0;
        
    uint32_t cal_flag;
    
    if ((hopamp1 == NULL) || (hopamp2 == NULL) || (hopamp3 == NULL)|| (hopamp4 == NULL) )
    {
        return HAL_ERROR;
    }
    else if (hopamp1->State != HAL_OPAMP_STATE_READY)
    {
        return HAL_ERROR;
    }
    else if (hopamp2->State != HAL_OPAMP_STATE_READY)
    {
        return HAL_ERROR;
    }
    else if (hopamp3->State != HAL_OPAMP_STATE_READY)
    {
        return HAL_ERROR;
    }
    else if (hopamp4->State != HAL_OPAMP_STATE_READY)
    {
        return HAL_ERROR;
    }
    else
    {
    
        /* Check the parameter */
        assert_param(IS_OPAMP_ALL_INSTANCE(hopamp1->Instance));
        assert_param(IS_OPAMP_ALL_INSTANCE(hopamp2->Instance));
        assert_param(IS_OPAMP_ALL_INSTANCE(hopamp3->Instance));
        assert_param(IS_OPAMP_ALL_INSTANCE(hopamp4->Instance));
    
        SET_BIT(hopamp1->Instance->CSR, OPAMP_CSR_OPAEN);
        SET_BIT(hopamp2->Instance->CSR, OPAMP_CSR_OPAEN);
        SET_BIT(hopamp3->Instance->CSR, OPAMP_CSR_OPAEN);
        SET_BIT(hopamp4->Instance->CSR, OPAMP_CSR_OPAEN);
        
        MODIFY_REG(hopamp1->Instance->CSR, OPAMP_CSR_CALSEL, OPAMP_VREF_3VDDA);
        MODIFY_REG(hopamp2->Instance->CSR, OPAMP_CSR_CALSEL, OPAMP_VREF_3VDDA);
        MODIFY_REG(hopamp3->Instance->CSR, OPAMP_CSR_CALSEL, OPAMP_VREF_3VDDA);
        MODIFY_REG(hopamp4->Instance->CSR, OPAMP_CSR_CALSEL, OPAMP_VREF_3VDDA);
        
        /*  user trimming values are used for offset calibration */
        SET_BIT(hopamp1->Instance->CSR, OPAMP_CSR_USERTRIM);
        SET_BIT(hopamp2->Instance->CSR, OPAMP_CSR_USERTRIM);
        SET_BIT(hopamp3->Instance->CSR, OPAMP_CSR_USERTRIM);
        SET_BIT(hopamp4->Instance->CSR, OPAMP_CSR_USERTRIM);
    
        MODIFY_REG(hopamp1->Instance->CSR, OPAMP_CSR_CALSEL, OPAMP_VREF_10VDDA);
        MODIFY_REG(hopamp2->Instance->CSR, OPAMP_CSR_CALSEL, OPAMP_VREF_10VDDA);
        MODIFY_REG(hopamp3->Instance->CSR, OPAMP_CSR_CALSEL, OPAMP_VREF_10VDDA);
        MODIFY_REG(hopamp4->Instance->CSR, OPAMP_CSR_CALSEL, OPAMP_VREF_10VDDA);
        
        MODIFY_REG(hopamp1->Instance->CSR, OPAMP_TRIMMING_PMOS, trimmingvaluep1); 
        MODIFY_REG(hopamp2->Instance->CSR, OPAMP_TRIMMING_PMOS, trimmingvaluep2); 
        MODIFY_REG(hopamp3->Instance->CSR, OPAMP_TRIMMING_PMOS, trimmingvaluep3); 
        MODIFY_REG(hopamp4->Instance->CSR, OPAMP_TRIMMING_PMOS, trimmingvaluep4); 
        
        HAL_Delay(1);
        
        /* Enable calibration */
        SET_BIT(hopamp1->Instance->CSR, OPAMP_CSR_CALON);
        SET_BIT(hopamp2->Instance->CSR, OPAMP_CSR_CALON);
        SET_BIT(hopamp3->Instance->CSR, OPAMP_CSR_CALON);
        SET_BIT(hopamp4->Instance->CSR, OPAMP_CSR_CALON);
            
        while( __HAL_OPAMP_GET_CALIBRATION_FLAG(hopamp1) == 1 )
        {
            MODIFY_REG(hopamp1->Instance->CSR, OPAMP_CSR_CALSEL, OPAMP_VREF_10VDDA);
        }
        
        while( __HAL_OPAMP_GET_CALIBRATION_FLAG(hopamp2) == 1 )
        {
            MODIFY_REG(hopamp2->Instance->CSR, OPAMP_CSR_CALSEL, OPAMP_VREF_10VDDA);
        }
        
        while( __HAL_OPAMP_GET_CALIBRATION_FLAG(hopamp3) == 1 )
        {
            MODIFY_REG(hopamp3->Instance->CSR, OPAMP_CSR_CALSEL, OPAMP_VREF_10VDDA);
        }
        
        while( __HAL_OPAMP_GET_CALIBRATION_FLAG(hopamp4) == 1 )
        {
            MODIFY_REG(hopamp4->Instance->CSR, OPAMP_CSR_CALSEL, OPAMP_VREF_10VDDA);
        }
    
        while (trimmingvaluep1 <= 0x1F)
        {
            for(int timeout=0; timeout<1000; timeout++)
            {
                cal_flag = __HAL_OPAMP_GET_CALIBRATION_FLAG(hopamp1);
                UNUSED(cal_flag);
            }
            
            if (__HAL_OPAMP_GET_CALIBRATION_FLAG(hopamp1) == 1)
            {
                break;
            }
            
            /* Set candidate trimming */
            MODIFY_REG(hopamp1->Instance->CSR, OPAMP_CSR_TRIMOFFSETP, trimmingvaluep1);  // write 3 times for workaround
            MODIFY_REG(hopamp1->Instance->CSR, OPAMP_CSR_TRIMOFFSETP, trimmingvaluep1);  // write 3 times for workaround
            MODIFY_REG(hopamp1->Instance->CSR, OPAMP_CSR_TRIMOFFSETP, trimmingvaluep1);  // write 3 times for workaround
            HAL_Delay(1);
            trimmingvaluep1++;        
        }
        
        while (trimmingvaluep2 <= 0x1F)
        {
            for(int timeout=0; timeout<1000; timeout++)
            {
                cal_flag = __HAL_OPAMP_GET_CALIBRATION_FLAG(hopamp2);
                UNUSED(cal_flag);
            }
            
            if (__HAL_OPAMP_GET_CALIBRATION_FLAG(hopamp2) == 1)
            {
                break;
            }
            
            /* Set candidate trimming */
            MODIFY_REG(hopamp2->Instance->CSR, OPAMP_CSR_TRIMOFFSETP, trimmingvaluep2);  // write 3 times for workaround
            MODIFY_REG(hopamp2->Instance->CSR, OPAMP_CSR_TRIMOFFSETP, trimmingvaluep2);  // write 3 times for workaround
            MODIFY_REG(hopamp2->Instance->CSR, OPAMP_CSR_TRIMOFFSETP, trimmingvaluep2);  // write 3 times for workaround
            HAL_Delay(1);
            trimmingvaluep2++;        
        }
        
        while (trimmingvaluep3 <= 0x1F)
        {
            for(int timeout=0; timeout<1000; timeout++)
            {
                cal_flag = __HAL_OPAMP_GET_CALIBRATION_FLAG(hopamp3);
                UNUSED(cal_flag);
            }
            
            if (__HAL_OPAMP_GET_CALIBRATION_FLAG(hopamp3) == 1)
            {
                break;
            }
            
            /* Set candidate trimming */
            MODIFY_REG(hopamp3->Instance->CSR, OPAMP_CSR_TRIMOFFSETP, trimmingvaluep3);  // write 3 times for workaround
            MODIFY_REG(hopamp3->Instance->CSR, OPAMP_CSR_TRIMOFFSETP, trimmingvaluep3);  // write 3 times for workaround
            MODIFY_REG(hopamp3->Instance->CSR, OPAMP_CSR_TRIMOFFSETP, trimmingvaluep3);  // write 3 times for workaround
            HAL_Delay(1);
            trimmingvaluep3++;        
        }
        
        while (trimmingvaluep4 <= 0x1F)
        {
            for(int timeout=0; timeout<1000; timeout++)
            {
                cal_flag = __HAL_OPAMP_GET_CALIBRATION_FLAG(hopamp4);
                UNUSED(cal_flag);
            }
            
            if (__HAL_OPAMP_GET_CALIBRATION_FLAG(hopamp4) == 1)
            {
                break;
            }
            
            /* Set candidate trimming */
            MODIFY_REG(hopamp4->Instance->CSR, OPAMP_CSR_TRIMOFFSETP, trimmingvaluep4);  // write 3 times for workaround
            MODIFY_REG(hopamp4->Instance->CSR, OPAMP_CSR_TRIMOFFSETP, trimmingvaluep4);  // write 3 times for workaround
            MODIFY_REG(hopamp4->Instance->CSR, OPAMP_CSR_TRIMOFFSETP, trimmingvaluep4);  // write 3 times for workaround
            HAL_Delay(1);
            trimmingvaluep4++;        
        }
        
        
        CLEAR_BIT(hopamp1->Instance->CSR, OPAMP_CSR_CALON);
        CLEAR_BIT(hopamp2->Instance->CSR, OPAMP_CSR_CALON);
        CLEAR_BIT(hopamp3->Instance->CSR, OPAMP_CSR_CALON);
        CLEAR_BIT(hopamp4->Instance->CSR, OPAMP_CSR_CALON);
        
        MODIFY_REG(hopamp1->Instance->CSR, OPAMP_CSR_CALSEL, OPAMP_VREF_3VDDA);
        MODIFY_REG(hopamp2->Instance->CSR, OPAMP_CSR_CALSEL, OPAMP_VREF_3VDDA);
        MODIFY_REG(hopamp3->Instance->CSR, OPAMP_CSR_CALSEL, OPAMP_VREF_3VDDA);
        MODIFY_REG(hopamp4->Instance->CSR, OPAMP_CSR_CALSEL, OPAMP_VREF_3VDDA);
        
        MODIFY_REG(hopamp1->Instance->CSR, OPAMP_CSR_CALSEL, OPAMP_VREF_90VDDA);
        MODIFY_REG(hopamp2->Instance->CSR, OPAMP_CSR_CALSEL, OPAMP_VREF_90VDDA);
        MODIFY_REG(hopamp3->Instance->CSR, OPAMP_CSR_CALSEL, OPAMP_VREF_90VDDA);
        MODIFY_REG(hopamp4->Instance->CSR, OPAMP_CSR_CALSEL, OPAMP_VREF_90VDDA);
        
        MODIFY_REG(hopamp1->Instance->CSR, OPAMP_CSR_TRIMOFFSETN, trimmingvaluen1);  //set to 0 (write 3 times for workaround)
        MODIFY_REG(hopamp2->Instance->CSR, OPAMP_CSR_TRIMOFFSETN, trimmingvaluen2);  //set to 0 (write 3 times for workaround)
        MODIFY_REG(hopamp3->Instance->CSR, OPAMP_CSR_TRIMOFFSETN, trimmingvaluen3);  //set to 0 (write 3 times for workaround)
        MODIFY_REG(hopamp4->Instance->CSR, OPAMP_CSR_TRIMOFFSETN, trimmingvaluen4);  //set to 0 (write 3 times for workaround)

        
        HAL_Delay(1);
        
        SET_BIT(hopamp1->Instance->CSR, OPAMP_CSR_CALON);
        SET_BIT(hopamp2->Instance->CSR, OPAMP_CSR_CALON);
        SET_BIT(hopamp3->Instance->CSR, OPAMP_CSR_CALON);
        SET_BIT(hopamp4->Instance->CSR, OPAMP_CSR_CALON);
    
        while( __HAL_OPAMP_GET_CALIBRATION_FLAG(hopamp1) == 1 )
        {
            MODIFY_REG(hopamp1->Instance->CSR, OPAMP_CSR_CALSEL, OPAMP_VREF_90VDDA);
        }
        
        while( __HAL_OPAMP_GET_CALIBRATION_FLAG(hopamp2) == 1 )
        {
            MODIFY_REG(hopamp2->Instance->CSR, OPAMP_CSR_CALSEL, OPAMP_VREF_90VDDA);
        }
        
        while( __HAL_OPAMP_GET_CALIBRATION_FLAG(hopamp3) == 1 )
        {
            MODIFY_REG(hopamp3->Instance->CSR, OPAMP_CSR_CALSEL, OPAMP_VREF_90VDDA);
        }
        
        while( __HAL_OPAMP_GET_CALIBRATION_FLAG(hopamp4) == 1 )
        {
            MODIFY_REG(hopamp4->Instance->CSR, OPAMP_CSR_CALSEL, OPAMP_VREF_90VDDA);
        }
    
        while (trimmingvaluen1 <= 0x1F)
        {
            for(int timeout=0; timeout<1000; timeout++)
            {
                cal_flag = __HAL_OPAMP_GET_CALIBRATION_FLAG(hopamp1);
                UNUSED(cal_flag);
            }
            
            if (__HAL_OPAMP_GET_CALIBRATION_FLAG(hopamp1) == 1)
            {
                break;
            }
            
            /* Set candidate trimming */
            MODIFY_REG(hopamp1->Instance->CSR, OPAMP_CSR_TRIMOFFSETN, trimmingvaluen1);  // write 3 times for workaround
            MODIFY_REG(hopamp1->Instance->CSR, OPAMP_CSR_TRIMOFFSETN, trimmingvaluen1);  // write 3 times for workaround
            MODIFY_REG(hopamp1->Instance->CSR, OPAMP_CSR_TRIMOFFSETN, trimmingvaluen1);  // write 3 times for workaround
            HAL_Delay(1);
            trimmingvaluen1++;        
        }
        
        while (trimmingvaluen2 <= 0x1F)
        {
            for(int timeout=0; timeout<1000; timeout++)
            {
                cal_flag = __HAL_OPAMP_GET_CALIBRATION_FLAG(hopamp2);
                UNUSED(cal_flag);
            }
            
            if (__HAL_OPAMP_GET_CALIBRATION_FLAG(hopamp2) == 1)
            {
                break;
            }
            
            /* Set candidate trimming */
            MODIFY_REG(hopamp2->Instance->CSR, OPAMP_CSR_TRIMOFFSETN, trimmingvaluen2);  // write 3 times for workaround
            MODIFY_REG(hopamp2->Instance->CSR, OPAMP_CSR_TRIMOFFSETN, trimmingvaluen2);  // write 3 times for workaround
            MODIFY_REG(hopamp2->Instance->CSR, OPAMP_CSR_TRIMOFFSETN, trimmingvaluen2);  // write 3 times for workaround
            HAL_Delay(1);
            trimmingvaluen2++;        
        }
            
        while (trimmingvaluen3 <= 0x1F)
        {
            for(int timeout=0; timeout<1000; timeout++)
            {
                cal_flag = __HAL_OPAMP_GET_CALIBRATION_FLAG(hopamp3);
                UNUSED(cal_flag);
            }
            
            if (__HAL_OPAMP_GET_CALIBRATION_FLAG(hopamp3) == 1)
            {
                break;
            }
            
            /* Set candidate trimming */
            MODIFY_REG(hopamp3->Instance->CSR, OPAMP_CSR_TRIMOFFSETN, trimmingvaluen3);  // write 3 times for workaround
            MODIFY_REG(hopamp3->Instance->CSR, OPAMP_CSR_TRIMOFFSETN, trimmingvaluen3);  // write 3 times for workaround
            MODIFY_REG(hopamp3->Instance->CSR, OPAMP_CSR_TRIMOFFSETN, trimmingvaluen3);  // write 3 times for workaround
            HAL_Delay(1);
            trimmingvaluen3++;        
        }
            
        while (trimmingvaluen4 <= 0x1F)
        {
            for(int timeout=0; timeout<1000; timeout++)
            {
                cal_flag = __HAL_OPAMP_GET_CALIBRATION_FLAG(hopamp4);
                UNUSED(cal_flag);
            }
            
            if (__HAL_OPAMP_GET_CALIBRATION_FLAG(hopamp4) == 1)
            {
                break;
            }
            
            /* Set candidate trimming */
            MODIFY_REG(hopamp4->Instance->CSR, OPAMP_CSR_TRIMOFFSETN, trimmingvaluen4);  // write 3 times for workaround
            MODIFY_REG(hopamp4->Instance->CSR, OPAMP_CSR_TRIMOFFSETN, trimmingvaluen4);  // write 3 times for workaround
            MODIFY_REG(hopamp4->Instance->CSR, OPAMP_CSR_TRIMOFFSETN, trimmingvaluen4);  // write 3 times for workaround
            HAL_Delay(1);
            trimmingvaluen4++;        
        }
    
        /* Disable calibration */
        CLEAR_BIT(hopamp1->Instance->CSR, OPAMP_CSR_CALON);
        CLEAR_BIT(hopamp2->Instance->CSR, OPAMP_CSR_CALON);
        CLEAR_BIT(hopamp3->Instance->CSR, OPAMP_CSR_CALON);
        CLEAR_BIT(hopamp4->Instance->CSR, OPAMP_CSR_CALON);
    
        /* Write calibration result N */
        hopamp1->Init.TrimmingValueN = trimmingvaluen1;
        hopamp2->Init.TrimmingValueN = trimmingvaluen2;
        hopamp3->Init.TrimmingValueN = trimmingvaluen3;
        hopamp4->Init.TrimmingValueN = trimmingvaluen4;
    
        /* Write calibration result P */
        hopamp1->Init.TrimmingValueP = trimmingvaluep1;
        hopamp2->Init.TrimmingValueP = trimmingvaluep2;
        hopamp3->Init.TrimmingValueP = trimmingvaluep3;
        hopamp4->Init.TrimmingValueP = trimmingvaluep4;
    
    }
    
    return HAL_OK;
}
  
/** 
  * end of OPAMP_Function_Definitions @}  
  */
  
#endif /* HAL_OPAMP_MODULE_ENABLED */

/** 
  * end of OPAMP @}  
  */
