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
*   File    : rx32g4xx_hal_clc.c
*   By      : RX_DV_Team
**************************************************************************************************
*/



/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_common.h"
#include "rx32g4xx_hal_clc.h"

/** @defgroup CLC CLC
  * @{
  */

#ifdef HAL_CLC_MODULE_ENABLED

/* Exported types ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/** @defgroup CLC_Function_Definitions CLC Function Definitions
  * @{
  */
/**
  * @brief  Config the basic setting of CLC.
  * @param  hclc: CLC handler type
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_CLC_Init(CLC_HandleTypeDef* hclc)
{
    HAL_StatusTypeDef tmp_hal_status = HAL_OK;

    assert_param(IS_CLC_INSTANCE(hclc->Instance));
    assert_param(IS_CLC_MODE(hclc->Init.Mode));
    assert_param(IS_CLC_DATA_SOURCE1(hclc->Init.DataSource1));
    assert_param(IS_CLC_DATA_SOURCE2(hclc->Init.DataSource2));
    assert_param(IS_CLC_DATA_SOURCE3(hclc->Init.DataSource3));
    assert_param(IS_CLC_DATA_SOURCE4(hclc->Init.DataSource4));

    if(hclc == NULL)
    {
        return HAL_ERROR;
    }

    // Init the low level hardware
    HAL_CLC_MspInit(hclc);

    // Reset the CLC output (force the output to 0)
    // Set the logic function to 4-input AND gate
    MODIFY_REG(hclc->Instance->CON, CLC_CON_MODE, CLC_MODE_FOUR_INPUT_AND);
    // Set the input source to default
    WRITE_REG(hclc->Instance->SEL, 0x0);
    // Set the gate polarity to non-inverted
    CLEAR_BIT(hclc->Instance->CON, (CLC_CON_G1POL | CLC_CON_G2POL | CLC_CON_G3POL | CLC_CON_G4POL));
    // Set the output polarity to non-inverted
    CLEAR_BIT(hclc->Instance->CON, CLC_CON_LCPOL);
    // Enable the CLC and CLC output port
    __HAL_CLC_ENABLE_OUTPUT_PORT(hclc);
    __HAL_CLC_ENABLE(hclc);

    // Check if the CLC output is reset
    if( __HAL_CLC_GET_OUTPUT(hclc) == SET )
    {
        return HAL_ERROR;
    }

    // Disable CLC module and output port
    __HAL_CLC_DISABLE(hclc);
    __HAL_CLC_DISABLE_OUTPUT_PORT(hclc);

    // Config the CLC calculation mode
    MODIFY_REG(hclc->Instance->CON, CLC_CON_MODE, hclc->Init.Mode);

    // Config the CLC input data source
    MODIFY_REG(hclc->Instance->SEL, CLC_SEL_DS1, hclc->Init.DataSource1);
    MODIFY_REG(hclc->Instance->SEL, CLC_SEL_DS2, hclc->Init.DataSource2);
    MODIFY_REG(hclc->Instance->SEL, CLC_SEL_DS3, hclc->Init.DataSource3);
    MODIFY_REG(hclc->Instance->SEL, CLC_SEL_DS4, hclc->Init.DataSource4);

    // Config the CLC output polarity
    if(hclc->Init.OutputPolarity == ENABLE)
    {
        SET_BIT(hclc->Instance->CON, CLC_CON_LCPOL);
    }
    else
    {
        CLEAR_BIT(hclc->Instance->CON, CLC_CON_LCPOL);
    }

    // Return function status
    return tmp_hal_status;
}

/**
  * @brief  Deinit CLC.
  * @param  hclc: CLC handler type
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_CLC_DeInit(CLC_HandleTypeDef* hclc)
{
    HAL_StatusTypeDef tmp_hal_status = HAL_OK;

    // Check CLC handle
    if(hclc == NULL)
    {
       return HAL_ERROR;
    }

    // Check the parameters
    assert_param(IS_CLC_INSTANCE(hclc->Instance));

    // Clear CLC register CON
    CLEAR_BIT(hclc->Instance->CON, ( CLC_CON_MODE | CLC_CON_LCPOL | CLC_CON_LCOE | CLC_CON_INTNIF | CLC_CON_INTPIF
                                   | CLC_CON_INTN | CLC_CON_INTP  | CLC_CON_LCEN | CLC_CON_G1POL  | CLC_CON_G2POL
                                   | CLC_CON_G3POL| CLC_CON_G4POL));

    // Clear CLC register SEL
    CLEAR_BIT(hclc->Instance->SEL, (CLC_SEL_DS1 | CLC_SEL_DS2 | CLC_SEL_DS3 | CLC_SEL_DS4));

    // Clear CLC register GLS
    WRITE_REG(hclc->Instance->GLS, 0x0);

    // DeInit the low level hardware
    HAL_CLC_MspDeInit(hclc);

    // Return function status
    return tmp_hal_status;
}

/**
  * @brief  Initializes the CLC MSP.
  * @param  hclc: CLC handler type
  * @retval None
  */
__weak void HAL_CLC_MspInit(CLC_HandleTypeDef* hclc)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(hclc);
    /* NOTE : This function should not be modified. When the callback is needed,
                function HAL_CLC_MspInit must be implemented in the user file.
    */
}

/**
  * @brief  Initializes the CLC MSP.
  * @param  hclc: CLC handler type
  * @retval None
  */
__weak void HAL_CLC_MspDeInit(CLC_HandleTypeDef* hclc)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(hclc);
    /* NOTE : This function should not be modified. When the callback is needed,
                function HAL_CLC_MspDeInit must be implemented in the user file.
    */
}

/**
  * @brief  Start the CLC Calculation.
  * @param  hclc: CLC handler type
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_CLC_Start(CLC_HandleTypeDef* hclc)
{
    HAL_StatusTypeDef tmp_hal_status = HAL_OK;

    // Check CLC handle
    if(hclc == NULL)
    {
       return HAL_ERROR;
    }

    // Check the parameters
    assert_param(IS_CLC_INSTANCE(hclc->Instance));

    // Enable the CLC and the output port
    __HAL_CLC_ENABLE(hclc);
    __HAL_CLC_ENABLE_OUTPUT_PORT(hclc);

    // Return function status
    return tmp_hal_status;
}

/**
  * @brief  Stop the CLC Calculation.
  * @param  hclc: CLC handler type
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_CLC_Stop(CLC_HandleTypeDef* hclc)
{
    HAL_StatusTypeDef tmp_hal_status = HAL_OK;

    // Check CLC handle
    if(hclc == NULL)
    {
       return HAL_ERROR;
    }

    // Check the parameters
    assert_param(IS_CLC_INSTANCE(hclc->Instance));

    // Disable the CLC and the output port
    __HAL_CLC_DISABLE(hclc);
    __HAL_CLC_DISABLE_OUTPUT_PORT(hclc);

    // Return function status
    return tmp_hal_status;
}

/**
  * @brief  Start the CLC Calculation with interrupt enable
  * @param  hclc: CLC handler type
  * @param  IT: CLC interrupt type
  *         The parameter can be the combinition of the following values:
  *         @arg CLC_IT_POS
  *         @arg CLC_IT_NEG
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_CLC_Start_IT(CLC_HandleTypeDef* hclc, uint32_t IT)
{
    HAL_StatusTypeDef tmp_hal_status = HAL_OK;

    // Check CLC handle
    if(hclc == NULL)
    {
       return HAL_ERROR;
    }

    // Check the parameters
    assert_param(IS_CLC_INSTANCE(hclc->Instance));

    // Enable CLC interrupt
    __HAL_CLC_ENABLE_IT(hclc, IT);

    // Enable the CLC and the output port
    __HAL_CLC_ENABLE(hclc);
    __HAL_CLC_ENABLE_OUTPUT_PORT(hclc);

    // Return function status
    return tmp_hal_status;
}

/**
  * @brief  Stop the CLC Calculation with interrupt
  * @param  hclc: CLC handler type
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_CLC_Stop_IT(CLC_HandleTypeDef* hclc)
{
    HAL_StatusTypeDef tmp_hal_status = HAL_OK;

    // Check CLC handle
    if(hclc == NULL)
    {
       return HAL_ERROR;
    }

    // Check the parameters
    assert_param(IS_CLC_INSTANCE(hclc->Instance));

    // Disable CLC interrupt
    __HAL_CLC_DISABLE_IT(hclc, (CLC_IT_POS | CLC_IT_NEG));

    // Disable the CLC and the output port
    __HAL_CLC_DISABLE(hclc);
    __HAL_CLC_DISABLE_OUTPUT_PORT(hclc);

    // Return function status
    return tmp_hal_status;
}

/**
  * @brief  Get the CLC output value
  * @param  hclc: CLC handler type
  * @retval CLC output values
  */
uint32_t  HAL_CLC_GetValue(CLC_HandleTypeDef* hclc)
{
    // Check the parameters
    assert_param(IS_CLC_INSTANCE(hclc->Instance));

    // Ensure the CLC is enabled (output value valid)
    assert_param(__HAL_CLC_GET_ENABLED(hclc));

    // Return function status
    return __HAL_CLC_GET_OUTPUT(hclc);
}

/**
  * @brief  Get the CLC output value
  * @param  hclc: CLC handler type
  * @retval None
  */
void HAL_CLC_IRQHandler(CLC_HandleTypeDef* hclc)
{
    // Check the parameters
    assert_param(IS_CLC_INSTANCE(hclc->Instance));

    // Check if the interrupt type is negative edge interrupt
    if((hclc->Instance->CON & CLC_CON_INTN) == CLC_CON_INTN)
    {
        if((hclc->Instance->CON & CLC_CON_INTNIF) == CLC_CON_INTNIF)
        {
            // Disable the interrupt and the flag
            __HAL_CLC_DISABLE_IT(hclc, CLC_IT_NEG);
            __HAL_CLC_CLEAR_FLAG(hclc, CLC_FLAG_NEG);

            // Enter callback function automatically
            HAL_CLC_CalCpltCallback(hclc);
        }
    }
    // Check if the interrupt type is positive edge interrupt
    else if((hclc->Instance->CON & CLC_CON_INTP) == CLC_CON_INTP)
    {
        if((hclc->Instance->CON & CLC_CON_INTPIF) == CLC_CON_INTPIF)
        {
            // Disable the interrupt and the flag
            __HAL_CLC_DISABLE_IT(hclc, CLC_IT_POS);
            __HAL_CLC_CLEAR_FLAG(hclc, CLC_FLAG_POS);

            // Enter callback function automatically
            HAL_CLC_CalCpltCallback(hclc);
        }
    }
    // unknown state
    else
    {
        HAL_CLC_ErrorCallback(hclc);
    }
}

/**
  * @brief  CLC calculation complete callback function
  * @param  hclc: CLC handler type
  * @retval None
  */
__weak void HAL_CLC_CalCpltCallback(CLC_HandleTypeDef* hclc)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(hclc);
    /* NOTE : This function should not be modified. When the callback is needed,
                function HAL_CLC_CalCpltCallback must be implemented in the user file.
    */
}

/**
  * @brief  CLC calculation error callback function
  * @param  hclc: CLC handler type
  * @retval None
  */
__weak void HAL_CLC_ErrorCallback(CLC_HandleTypeDef *hclc)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(hclc);
    /* NOTE : This function should not be modified. When the callback is needed,
                function HAL_CLC_ErrorCallback must be implemented in the user file.
    */
}

/**
  * @brief  Config the specific CLC gate
  * @param  hclc: CLC handler type
  * @param  gConfig: CLC gate config type
  * @param  CLC_Gate: CLC calculate gate
  *         @arg CLC_Gate_1
  *         @arg CLC_Gate_2
  *         @arg CLC_Gate_3
  *         @arg CLC_Gate_4
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_CLC_ConfigGate(CLC_HandleTypeDef* hclc, CLC_GateConfTypeDef* gConfig, uint32_t gate)
{
    HAL_StatusTypeDef tmp_hal_status = HAL_OK;
    uint32_t gate_pos = 0;

    // Check CLC handle
    if(hclc == NULL)
    {
       return HAL_ERROR;
    }

    // Check the parameters
    assert_param(IS_CLC_INSTANCE(hclc->Instance));
    assert_param(IS_CLC_GATE(gate));

    // Calculate the specific gate position
    gate_pos = __HAL_CLC_GATE_POS(gate);

    // Config the specific gate
    MODIFY_REG(hclc->Instance->GLS, (CLC_GLS_G1D1N << gate_pos), (gConfig->Data1N << CLC_GLS_G1D1N_Pos) << gate_pos);
    MODIFY_REG(hclc->Instance->GLS, (CLC_GLS_G1D1T << gate_pos), (gConfig->Data1T << CLC_GLS_G1D1T_Pos) << gate_pos);
    MODIFY_REG(hclc->Instance->GLS, (CLC_GLS_G1D2N << gate_pos), (gConfig->Data2N << CLC_GLS_G1D2N_Pos) << gate_pos);
    MODIFY_REG(hclc->Instance->GLS, (CLC_GLS_G1D2T << gate_pos), (gConfig->Data2T << CLC_GLS_G1D2T_Pos) << gate_pos);

    MODIFY_REG(hclc->Instance->GLS, (CLC_GLS_G1D3N << gate_pos), (gConfig->Data3N << CLC_GLS_G1D3N_Pos) << gate_pos);
    MODIFY_REG(hclc->Instance->GLS, (CLC_GLS_G1D3T << gate_pos), (gConfig->Data3T << CLC_GLS_G1D3T_Pos) << gate_pos);
    MODIFY_REG(hclc->Instance->GLS, (CLC_GLS_G1D4N << gate_pos), (gConfig->Data4N << CLC_GLS_G1D4N_Pos) << gate_pos);
    MODIFY_REG(hclc->Instance->GLS, (CLC_GLS_G1D4T << gate_pos), (gConfig->Data4T << CLC_GLS_G1D4T_Pos) << gate_pos);

    /*!< Config the gate output polarity */
    MODIFY_REG(hclc->Instance->CON, (CLC_CON_G1POL << (gate-1)), (gConfig->GatePolarity << CLC_CON_G1POL_Pos << (gate-1)));
    
    // Return function status
    return tmp_hal_status;
}
/**
  * end of CLC_Function_Definitions @}
  */

#endif /* HAL_CLC_MODULE_ENABLED */

/**
  * end of CLC @}
  */
