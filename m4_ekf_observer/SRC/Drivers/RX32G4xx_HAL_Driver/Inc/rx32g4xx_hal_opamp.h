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
*   File    : rx32g4xx_hal_opamp.h
*   By      : RX_DV_Team
**************************************************************************************************
*/


#ifndef _RX32G4XX_HAL_OPAMP_H_
#define _RX32G4XX_HAL_OPAMP_H_

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal_def.h"

/* Exported types ------------------------------------------------------------*/

/** @addtogroup OPAMP
  * @{
  */

/******************************************************************************/
/*                              OPAMP Structures                              */
/******************************************************************************/
  
/** @defgroup OPAMP_Structure_Definitions Template Structure Definitions
  * @{
  */
  
/** @defgroup OPAMP_InitTypeDef OPAMP InitTypeDef
  * @ingroup  OPAMP_Structure_Definitions 
  * @{
  */
/**
  * @brief  OPAMP Init structure definition
  */

typedef struct
{
    uint32_t PowerMode;                   /*!< Specifies the power mode Normal or High Speed.
                                                This parameter must be a value of @ref OPAMP_PowerMode */
    
    uint32_t Mode;                        /*!< Specifies the OPAMP mode
                                                This parameter must be a value of @ref OPAMP_Mode
                                                mode is either Follower or PGA */
                                                
    uint32_t VP_SEL;                      /*!< Specifified the OPAMP VP selection
                                                This parameter must be a value of @ref OPAMP_NonInvertingInput */
                                                
    uint32_t VM_SEL;                      /*!< Specifified the OPAMP VM selection
                                                This parameter must be a value of @ref OPAMP_InvertingInput */
    
    uint32_t BandgapBuffer;               /*!< Specifified the OPAMP bangap buffer mode
                                                This parameter must be a value of ENABLE or DISABLE */
    
    uint32_t BiasVolRef;                  /*!< Specifified the OPAMP Bias voltage reference
                                                This parameter must be a value of @ref OPAMP_VOL_REF */
    
    FunctionalState InternalOutput;       /*!< Specifies the configuration of the internal output from OPAMP to ADC.
                                                This parameter can be ENABLE or DISABLE
                                                Note: When this output is enabled, regular output to I/O is disabled */
    
    uint32_t PgaGain;                     /*!< Specifies the gain in PGA mode
                                                i.e. when mode is OPAMP_PGA_MODE.
                                                This parameter must be a value of @ref OPAMP_PgaGain */
    
    uint32_t UserTrimming;                /*!< Specifies the trimming mode
                                                This parameter must be a value of @ref OPAMP_UserTrimming
                                                UserTrimming is either factory or user trimming */
    
    uint32_t TrimmingValueP;              /*!< Specifies the offset trimming value (PMOS)
                                                i.e. when UserTrimming is OPAMP_TRIMMING_USER.
                                                This parameter must be a number between Min_Data = 1 and Max_Data = 31 */
    
    uint32_t TrimmingValueN;              /*!< Specifies the offset trimming value (NMOS)
                                                i.e. when UserTrimming is OPAMP_TRIMMING_USER.
                                                This parameter must be a number between Min_Data = 1 and Max_Data = 31 */
    
    uint32_t OUTCONNECT; 				  /*!< Specifies the OUT CONNECT in PGA mode i.e. when mode is OPAMP_PGA_MODE.
                                                This parameter must be a value of @ref OPAMP_OUT_CONNECT*/
} OPAMP_InitTypeDef;
  
/** 
  * end of OPAMP_InitTypeDef @}  
  */
  
/** @defgroup HAL_OPAMP_StateTypeDef HAL OPAMP StateTypeDef
  * @ingroup  OPAMP_Structure_Definitions 
  * @{
  */
/**
  * @brief  HAL State structures definition
  */

typedef enum
{
  HAL_OPAMP_STATE_RESET               = 0x00000000UL, /*!< OPAMP is not yet Initialized          */

  HAL_OPAMP_STATE_READY               = 0x00000001UL, /*!< OPAMP is initialized and ready for use */
  HAL_OPAMP_STATE_CALIBBUSY           = 0x00000002UL, /*!< OPAMP is enabled in auto calibration mode */

  HAL_OPAMP_STATE_BUSY                = 0x00000004UL, /*!< OPAMP is enabled and running in normal mode */
  HAL_OPAMP_STATE_BUSYLOCKED          = 0x00000005UL, /*!< OPAMP control register is locked
                                                         only system reset allows reconfiguring the opamp. */

} HAL_OPAMP_StateTypeDef;
/** 
  * end of HAL_OPAMP_StateTypeDef @}  
  */
  
/** @defgroup OPAMP_HandleTypeDef OPAMP HandleTypeDef
  * @ingroup  OPAMP_Structure_Definitions 
  * @{
  */
/**
  * @brief OPAMP Handle Structure definition
  */
#if (USE_HAL_OPAMP_REGISTER_CALLBACKS == 1)
typedef struct __OPAMP_HandleTypeDef
#else
typedef struct
#endif
{
    OPAMP_TypeDef           *Instance;                      /*!< OPAMP instance's registers base address   */
    OPAMP_InitTypeDef       Init;                           /*!< OPAMP required parameters */
    HAL_StatusTypeDef       Status;                         /*!< OPAMP peripheral status   */
    HAL_LockTypeDef         Lock;                           /*!< Locking object          */
    __IO HAL_OPAMP_StateTypeDef  State;                     /*!< OPAMP communication state */
    
#if (USE_HAL_OPAMP_REGISTER_CALLBACKS == 1)
    void (* MspInitCallback)(struct __OPAMP_HandleTypeDef *hopamp);
    void (* MspDeInitCallback)(struct __OPAMP_HandleTypeDef *hopamp);
#endif /* USE_HAL_OPAMP_REGISTER_CALLBACKS */

} OPAMP_HandleTypeDef;
/** 
  * end of OPAMP_HandleTypeDef @}  
  */
  
/** @defgroup OPAMP_TrimmingValueTypeDef OPAMP TrimmingValueTypeDef
  * @ingroup  OPAMP_Structure_Definitions 
  * @{
  */
/**
  * @brief OPAMP_TrimmingValueTypeDef definition
  */

typedef  uint32_t OPAMP_TrimmingValueTypeDef;
/** 
  * end of OPAMP_TrimmingValueTypeDef @}  
  */ 
/** @defgroup HAL_OPAMP_CallbackIDTypeDef HAL OPAMP CallbackIDTypeDef
  * @ingroup  OPAMP_Structure_Definitions 
  * @{
  */
#if (USE_HAL_OPAMP_REGISTER_CALLBACKS == 1)
/**
  * @brief  HAL OPAMP Callback ID enumeration definition
  */
typedef enum
{
    HAL_OPAMP_MSPINIT_CB_ID                     = 0x01UL,  /*!< OPAMP MspInit Callback ID           */
    HAL_OPAMP_MSPDEINIT_CB_ID                   = 0x02UL,  /*!< OPAMP MspDeInit Callback ID         */
    HAL_OPAMP_ALL_CB_ID                         = 0x03UL   /*!< OPAMP All ID                        */
    
} HAL_OPAMP_CallbackIDTypeDef;

#endif /* USE_HAL_OPAMP_REGISTER_CALLBACKS */

/** 
  * end of HAL_OPAMP_CallbackIDTypeDef @}  
  */ 
/** @defgroup pOPAMP_CallbackTypeDef pOPAMP CallbackTypeDef
  * @ingroup  OPAMP_Structure_Definitions 
  * @{
  */
/**
  * @brief  HAL OPAMP Callback pointer definition
  */
typedef void (*pOPAMP_CallbackTypeDef)(OPAMP_HandleTypeDef *hopamp);
/** 
  * end of pOPAMP_CallbackTypeDef @}  
  */  
/** 
  * end of OPAMP_Structure_Definitions @}  
  */

/******************************************************************************/
/*                              OPAMP Parameters                              */
/******************************************************************************/
  
/** @defgroup OPAMP_Parameter_Definitions OPAMP Parameter Definitions
  * @{
  */
  
/** @defgroup OPAMP_Mode OPAMP Mode
  * @ingroup  OPAMP_Parameter_Definitions
  * @{
  */
#define OPAMP_OPA_MODE                   (0x00000000UL)   		/*!< OPA mode */ 	
#define OPAMP_PGA_MODE                   OPAMP_CSR_VMSEL_BIT3   /*!< PGA mode */
#define OPAMP_FOLLOWER_MODE              OPAMP_CSR_VMSEL_BIT4   /*!< follower mode */ 
/** 
  * end of OPAMP_Mode @}  
  */
  
/** @defgroup OPAMP_NonInvertingInput OPAMP NonInvertingInput
  * @ingroup  OPAMP_Parameter_Definitions
  * @{
  */
#define OPAMP_VPSEL_P1                                 (0x00000000UL)
#define OPAMP_VPSEL_P2                                 OPAMP_CSR_VPSEL
/** 
  * end of OPAMP_NonInvertingInput @}  
  */
  
/** @defgroup OPAMP_VOL_REF OPAMP VOL REF
  * @ingroup  OPAMP_Parameter_Definitions
  * @{
  */
#define OPAMP_REFERENCE_FROM_VBG                       (0x00000000UL)
#define OPAMP_REFERENCE_FROM_VREF                      OPAMP_CR2_VCMSEL
/** 
  * end of OPAMP_VOL_REF @}  
  */
/** @defgroup OPAMP_InvertingInput OPAMP InvertingInput
  * @ingroup  OPAMP_Parameter_Definitions
  * @{
  */
#define OPAMP_VMSEL_N1                                 OPAMP_CSR_VMSEL_BIT0
#define OPAMP_VMSEL_N2                                 OPAMP_CSR_VMSEL_BIT1
#define OPAMP_VMSEL_N3                                 OPAMP_CSR_VMSEL_BIT2
/** 
  * end of OPAMP_InvertingInput @}  
  */
  
/** @defgroup OPAMP_PgaGain OPAMP PgaGain
  * @ingroup  OPAMP_Parameter_Definitions
  * @{
  */
#define OPAMP_PGA_GAIN_4                        (0x00000000UL)                                             
#define OPAMP_PGA_GAIN_8                        (                                             OPAMP_CSR_GAIN_BIT0) 
#define OPAMP_PGA_GAIN_12                       (                       OPAMP_CSR_GAIN_BIT1                      ) 
#define OPAMP_PGA_GAIN_16                       (                       OPAMP_CSR_GAIN_BIT1 | OPAMP_CSR_GAIN_BIT0) 
#define OPAMP_PGA_GAIN_24                       (OPAMP_CSR_GAIN_BIT2                                             ) 
#define OPAMP_PGA_GAIN_32                       (OPAMP_CSR_GAIN_BIT2 |                        OPAMP_CSR_GAIN_BIT0) 
/** 
  * end of OPAMP_PgaGain @}  
  */
  
/** @defgroup OPAMP_UserTrimming OPAMP UserTrimming
  * @ingroup  OPAMP_Parameter_Definitions
  * @{
  */
#define OPAMP_TRIMMING_FACTORY                  (0x00000000UL)                       /*!< Factory trimming */
#define OPAMP_TRIMMING_USER                     OPAMP_CSR_USERTRIM                   /*!< User trimming */
/** 
  * end of OPAMP_UserTrimming @}  
  */
  
/** @defgroup OPAMP_CAL_VREF OPAMP CAL VREF
  * @ingroup  OPAMP_Parameter_Definitions
  * @{
  */
#define OPAMP_VREF_3VDDA                     (0x00000000UL)            /*!< OPAMP Vref = 3.3% VDDA */
#define OPAMP_VREF_10VDDA                    OPAMP_CSR_CALSEL_BIT0     /*!< OPAMP Vref = 10% VDDA  */
#define OPAMP_VREF_50VDDA                    OPAMP_CSR_CALSEL_BIT1     /*!< OPAMP Vref = 50% VDDA  */
#define OPAMP_VREF_90VDDA                    OPAMP_CSR_CALSEL          /*!< OPAMP Vref = 90% VDDA  */
/** 
  * end of OPAMP_CAL_VREF @}  
  */
  
/** @defgroup OPAMP_OUT_CONNECT OPAMP OUT CONNECT
  * @ingroup  OPAMP_Parameter_Definitions
  * @{
  */
#define OPAMP_NO_CONNECT                               	(0x00000000UL)
#define OPAMP_CONNECT_TO_GND                           	OPAMP_CSR_OUTCONNECT_BIT0
#define OPAMP_CONNECT_TO_VINN1                       		OPAMP_CSR_OUTCONNECT_BIT1
#define OPAMP_CONNECT_TO_VINN3                       		(OPAMP_CSR_OUTCONNECT_BIT1 | OPAMP_CSR_OUTCONNECT_BIT0)
/** 
  * end of OPAMP_OUT_CONNECT @}  
  */
  
/** @defgroup OPAMP_PowerMode OPAMP POWER MODE
  * @ingroup  OPAMP_Parameter_Definitions
  * @{
  */
#define OPAMP_POWERMODE_NORMALSPEED                    (0x00000000UL)         /*!< OPAMP output in normal mode */
#define OPAMP_POWERMODE_HIGHSPEED                      OPAMP_CSR_OPAHSM       /*!< OPAMP output in high speed mode */
/** 
  * end of OPAMP_PowerMode @}  
  */

/** @defgroup OPAMP_TRIMMING_TRANSISTORS_DIFF_PAIR OPAMP TRIMMING TRANSISTORS DIFF PAIR
  * @ingroup  OPAMP_Parameter_Definitions
  * @{
  */
#define OPAMP_TRIMMING_NMOS_VREF_90PC_VDDA             (OPAMP_CSR_TRIMOFFSETN | OPAMP_CSR_CALSEL_BIT1 | OPAMP_CSR_CALSEL_BIT0) /*!< OPAMP trimming of transistors differential pair NMOS (internal reference voltage set to 0.9*Vdda). Default parameters to be used for calibration using two trimming steps (one with each transistors differential pair NMOS and PMOS). */
#define OPAMP_TRIMMING_NMOS_VREF_50PC_VDDA             (OPAMP_CSR_TRIMOFFSETN | OPAMP_CSR_CALSEL_BIT1                        ) /*!< OPAMP trimming of transistors differential pair NMOS (internal reference voltage set to 0.5*Vdda). */
#define OPAMP_TRIMMING_PMOS_VREF_10PC_VDDA             (OPAMP_CSR_TRIMOFFSETP                         | OPAMP_CSR_CALSEL_BIT0) /*!< OPAMP trimming of transistors differential pair PMOS (internal reference voltage set to 0.1*Vdda). Default parameters to be used for calibration using two trimming steps (one with each transistors differential pair NMOS and PMOS). */
#define OPAMP_TRIMMING_PMOS_VREF_3p3PC_VDDA            (OPAMP_CSR_TRIMOFFSETP                                                ) /*!< OPAMP trimming of transistors differential pair PMOS (internal reference voltage set to 0.33*Vdda). */
#define OPAMP_TRIMMING_NMOS                            (OPAMP_TRIMMING_NMOS_VREF_90PC_VDDA) /*!< OPAMP trimming of transistors differential pair NMOS (internal reference voltage set to 0.9*Vdda). Default parameters to be used for calibration using two trimming steps (one with each transistors differential pair NMOS and PMOS). */
#define OPAMP_TRIMMING_PMOS                            (OPAMP_TRIMMING_PMOS_VREF_10PC_VDDA) /*!< OPAMP trimming of transistors differential pair PMOS (internal reference voltage set to 0.1*Vdda). Default parameters to be used for calibration using two trimming steps (one with each transistors differential pair NMOS and PMOS). */
/** 
  * end of OPAMP_TRIMMING_TRANSISTORS_DIFF_PAIR @}  
  */
  
  
/** @defgroup OPAMP_CALIBRATION_MODE OPAMP CALIBRATION MODE
  * @ingroup  OPAMP_Parameter_Definitions
  * @{
  */
#define OPAMP_MODE_FUNCTIONAL                          (0x00000000UL)        /*!< OPAMP functional mode */
#define OPAMP_MODE_CALIBRATION                         (OPAMP_CSR_CALON)     /*!< OPAMP calibration mode */
/** 
  * end of OPAMP_CALIBRATION_MODE @}  
  */ 
/** 
  * end of OPAMP_Parameter_Definitions @}  
  */


/******************************************************************************/
/*                                OPAMP Macro                                 */
/******************************************************************************/
/** @defgroup OPAMP_Macro_Definitions OPAMP Macro Definitions
  * @{
  */

/** @brief Reset OPAMP handle state
  * @param  __HANDLE__ OPAMP handle.
  * @retval None
  */
#define __HAL_OPAMP_RESET_HANDLE_STATE(__HANDLE__) ((__HANDLE__)->State = HAL_OPAMP_STATE_RESET)

/** @brief Check if OPAMP calibration flag is set
  * @param  __HANDLE__ OPAMP handle.
  * @retval Status of the flag
  */
  #define __HAL_OPAMP_GET_CALIBRATION_FLAG(__HANDLE__) ((((__HANDLE__)->Instance->CSR & OPAMP_CSR_CALOUT) == OPAMP_CSR_CALOUT) ? 1U : 0U)

/** @brief Check if the parameter INPUT is valid
  * @param  INPUT
  * @retval Result
  *         @arg SET(valid)
  *         @arg RESET (invalid)
  */
#define IS_OPAMP_FUNCTIONAL_NORMALMODE(INPUT) (((INPUT) == OPAMP_PGA_MODE) || \
                                               ((INPUT) == OPAMP_FOLLOWER_MODE) || \
																							 ((INPUT) == OPAMP_OPA_MODE))
/** @brief Check if the parameter POWERMODE is valid
  * @param  POWERMODE
  * @retval Result
  *         @arg SET(valid)
  *         @arg RESET (invalid)
  */
#define IS_OPAMP_POWERMODE(POWERMODE) (((POWERMODE) == OPAMP_POWERMODE_NORMALSPEED) || \
                                       ((POWERMODE) == OPAMP_POWERMODE_HIGHSPEED) )

/** @brief Check if the parameter TRIMMING is valid
  * @param  TRIMMING
  * @retval Result
  *         @arg SET(valid)
  *         @arg RESET (invalid)
  */
#define IS_OPAMP_TRIMMING(TRIMMING) (((TRIMMING) == OPAMP_TRIMMING_FACTORY) || \
                                     ((TRIMMING) == OPAMP_TRIMMING_USER))

/** @brief Check if the parameter TRIMMINGVALUE is valid
  * @param  TRIMMINGVALUE
  * @retval Result
  *         @arg SET(valid)
  *         @arg RESET (invalid)
  */
#define IS_OPAMP_TRIMMINGVALUE(TRIMMINGVALUE) ((TRIMMINGVALUE) <= 0x1FUL)

/** @brief Check if the parameter VREF is valid
  * @param  VREF
  * @retval Result
  *         @arg SET(valid)
  *         @arg RESET (invalid)
  */
#define IS_OPAMP_VREF(VREF) (((VREF) == OPAMP_VREF_3VDDA)  || \
                             ((VREF) == OPAMP_VREF_10VDDA) || \
                             ((VREF) == OPAMP_VREF_50VDDA) || \
                             ((VREF) == OPAMP_VREF_90VDDA))  
  
/** 
  * end of OPAMP_Macro_Definitions @}  
  */
  
/******************************************************************************/
/*                               OPAMP Functions                              */
/******************************************************************************/

/** @defgroup OPAMP_Function_Definitions OPAMP Function Definitions
  * @{
  */
HAL_StatusTypeDef           HAL_OPAMP_Init(OPAMP_HandleTypeDef *hopamp);
HAL_StatusTypeDef           HAL_OPAMP_DeInit(OPAMP_HandleTypeDef *hopamp);
void                        HAL_OPAMP_MspInit(OPAMP_HandleTypeDef *hopamp);
void                        HAL_OPAMP_MspDeInit(OPAMP_HandleTypeDef *hopamp);

HAL_StatusTypeDef           HAL_OPAMP_Start(OPAMP_HandleTypeDef *hopamp);
HAL_StatusTypeDef           HAL_OPAMP_Stop(OPAMP_HandleTypeDef *hopamp);
HAL_StatusTypeDef           HAL_OPAMP_SelfCalibrate(OPAMP_HandleTypeDef *hopamp);

#if (USE_HAL_OPAMP_REGISTER_CALLBACKS == 1)
/* OPAMP callback registering/unregistering */
HAL_StatusTypeDef           HAL_OPAMP_RegisterCallback(OPAMP_HandleTypeDef *hopamp, HAL_OPAMP_CallbackIDTypeDef CallbackId,
                                             pOPAMP_CallbackTypeDef pCallback);
HAL_StatusTypeDef           HAL_OPAMP_UnRegisterCallback(OPAMP_HandleTypeDef *hopamp, HAL_OPAMP_CallbackIDTypeDef CallbackId);
#endif /* USE_HAL_OPAMP_REGISTER_CALLBACKS */

HAL_StatusTypeDef           HAL_OPAMP_Lock(OPAMP_HandleTypeDef *hopamp);
HAL_OPAMP_StateTypeDef      HAL_OPAMP_GetState(OPAMP_HandleTypeDef *hopamp);
OPAMP_TrimmingValueTypeDef  HAL_OPAMP_GetTrimOffset(OPAMP_HandleTypeDef *hopamp, uint32_t trimmingoffset);  
  
void HAL_OPAMP_selfCalibrate(OPAMP_TypeDef *OPAMPx);
	
/** 
  * end of OPAMP_Function_Definitions @}  
  */
/** 
  * end of OPAMP @}  
  */


#ifdef __cplusplus
}
#endif

#endif /* _RX32G4XX_HAL_OPAMP_H_ */

