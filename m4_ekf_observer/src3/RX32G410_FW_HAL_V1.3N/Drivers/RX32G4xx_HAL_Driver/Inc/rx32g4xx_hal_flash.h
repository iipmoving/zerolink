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
*   File    : rx32g4xx_hal_flash.h
*   By      : RX_DV_Team
**************************************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef RX32G4xx_HAL_FLASH_H
#define RX32G4xx_HAL_FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal_def.h"

/** @addtogroup FLASH
  * @{
  */

/* Exported types ------------------------------------------------------------*/

/** @defgroup FLASH_Structure_Definitions FLASH Structure Definitions
  * @{
  */

/** @defgroup FLASH_EraseInitTypeDef FLASH EraseInitTypeDef
  * @ingroup  FLASH_Structure_Definitions
  * @{
  */

/**
  * @brief  FLASH Erase structure definition
  */
typedef struct
{
  uint32_t TypeErase;   /*!< Mass erase or page erase.
                             This parameter can be a value of @ref FLASH_Type_Erase */
  uint32_t Banks;       /*!< Select bank to erase.
                             This parameter must be a value of @ref FLASH_Banks
                             (FLASH_BANK_BOTH should be used only for mass erase) */
  uint32_t Page;        /*!< Initial Flash page to erase when page erase is disabled.
                             This parameter must be a value between 0 and (max number of pages in the bank - 1)
                             (eg : 127 for 512KB dual bank) */
  uint32_t NbPages;     /*!< Number of pages to be erased.
                             This parameter must be a value between 1 and (max number of pages in the bank - value of initial page)*/
} FLASH_EraseInitTypeDef;

/**
  * end of FLASH_EraseInitTypeDef @}
  */

/** @defgroup FLASH_OBProgramInitTypeDef FLASH OBProgramInitTypeDef
  * @ingroup  FLASH_Structure_Definitions
  * @{
  */

/**
  * @brief  FLASH Option Bytes Program structure definition
  */
typedef struct
{
  uint32_t OptionType;     /*!< Option byte to be configured.
                                This parameter can be a combination of the values of @ref FLASH_OB_Type */
  uint32_t WRPArea;        /*!< Write protection area to be programmed (used for OPTIONBYTE_WRP).
                                Only one WRP area could be programmed at the same time.
                                This parameter can be value of @ref FLASH_OB_WRP_Area */
  uint32_t WRPStartOffset; /*!< Write protection start offset (used for OPTIONBYTE_WRP).
                                This parameter must be a value between 0 and (max number of pages in the bank - 1) */
  uint32_t WRPEndOffset;   /*!< Write protection end offset (used for OPTIONBYTE_WRP).
                                This parameter must be a value between WRPStartOffset and (max number of pages in the bank - 1) */
  uint32_t RDPLevel;       /*!< Set the read protection level.. (used for OPTIONBYTE_RDP).
                                This parameter can be a value of @ref FLASH_OB_Read_Protection */
  uint32_t USERType;       /*!< User option byte(s) to be configured (used for OPTIONBYTE_USER).
                                This parameter can be a combination of @ref FLASH_OB_USER_Type */
  uint32_t USERConfig;     /*!< Value of the user option byte (used for OPTIONBYTE_USER).
                                This parameter can be a combination of @ref FLASH_OB_USER_BOR_LEVEL,
                                @ref FLASH_OB_USER_nRST_STOP, @ref FLASH_OB_USER_nRST_STANDBY,
                                @ref FLASH_OB_USER_nRST_SHUTDOWN, @ref FLASH_OB_USER_IWDG_SW,
                                @ref FLASH_OB_USER_IWDG_STOP, @ref FLASH_OB_USER_IWDG_STANDBY,
                                @ref FLASH_OB_USER_WWDG_SW, @ref FLASH_OB_USER_BFB2 (*),
                                @ref FLASH_OB_USER_nBOOT1, @ref FLASH_OB_USER_SRAM_PE,
                                @ref FLASH_OB_USER_CCMSRAM_RST
                                @note (*) availability depends on devices */
  uint32_t BootEntryPoint; /*!< Set the Boot Lock (used for OPTIONBYTE_BOOT_LOCK).
                                This parameter can be a value of @ref FLASH_OB_Boot_Lock */
  uint32_t SecBank;        /*!< Bank of securable memory area to be programmed (used for OPTIONBYTE_SEC).
                                Only one securable memory area could be programmed at the same time.
                                This parameter can be one of the following values:
                                FLASH_BANK_1: Securable memory area to be programmed in bank 1
                                FLASH_BANK_2: Securable memory area to be programmed in bank 2 (*)
                                @note (*) availability depends on devices */
  uint32_t SecSize;        /*!< Size of securable memory area to be programmed (used for OPTIONBYTE_SEC),
                                in number of pages. Securable memory area is starting from first page of the bank.
                                Only one securable memory could be programmed at the same time.
                                This parameter must be a value between 0 and (max number of pages in the bank - 1) */
} FLASH_OBProgramInitTypeDef;

/**
  * end of FLASH_OBProgramInitTypeDef @}
  */

/** @defgroup FLASH_ProcedureTypeDef FLASH ProcedureTypeDef
  * @ingroup  FLASH_Structure_Definitions
  * @{
  */

/**
  * @brief  FLASH Procedure structure definition
  */
typedef enum
{
  FLASH_PROC_NONE = 0,
  FLASH_PROC_PAGE_ERASE,
  FLASH_PROC_MASS_ERASE,
  FLASH_PROC_PROGRAM,
  FLASH_PROC_PROGRAM_LAST
} FLASH_ProcedureTypeDef;

/**
  * end of FLASH_ProcedureTypeDef @}
  */

/** @defgroup FLASH_CacheTypeDef FLASH CacheTypeDef
  * @ingroup  FLASH_Structure_Definitions
  * @{
  */

/**
  * @brief  FLASH Cache structure definition
  */

typedef enum
{
  FLASH_CACHE_DISABLED = 0,
  FLASH_CACHE_ICACHE_ENABLED,
  FLASH_CACHE_DCACHE_ENABLED,
  FLASH_CACHE_ICACHE_DCACHE_ENABLED
} FLASH_CacheTypeDef;
/**
  * end of FLASH_CacheTypeDef @}
  */

/** @defgroup HAL_FLASH_ProcessTypeDef HAL FLASH ProcessTypeDef
  * @ingroup  FLASH_Structure_Definitions
  * @{
  */

/**
  * @brief  FLASH handle Structure definition
  */

typedef struct
{
  HAL_LockTypeDef             Lock;              /* FLASH locking object */
  __IO uint32_t               ErrorCode;         /* FLASH error code */
  __IO FLASH_ProcedureTypeDef ProcedureOnGoing;  /* Internal variable to indicate which procedure is ongoing or not in IT context */
  __IO uint32_t               Address;           /* Internal variable to save address selected for program in IT context */
  __IO uint32_t               Bank;              /* Internal variable to save current bank selected during erase in IT context */
  __IO uint32_t               Page;              /* Internal variable to define the current page which is erasing in IT context */
  __IO uint32_t               NbPagesToErase;    /* Internal variable to save the remaining pages to erase in IT context */
  __IO FLASH_CacheTypeDef     CacheToReactivate; /* Internal variable to indicate which caches should be reactivated */
} HAL_FLASH_ProcessTypeDef;

/**
  * end of HAL_FLASH_ProcessTypeDef @}
  */

/**
  * end of FLASH_Structure_Definitions @}
  */

/******************************************************************************/
/*                               FLASH Parameters                             */
/******************************************************************************/

/* Exported constants --------------------------------------------------------*/
/** @defgroup FLASH_Parameter_Definitions FLASH Parameter Definitions
  * @{
  */

/** @defgroup FLASH_Error FLASH Error
  * @ingroup  FLASH_Parameter_Definitions
  * @{
  */

#define HAL_FLASH_ERROR_NONE      0x00000000U
#define HAL_FLASH_ERROR_OP        FLASH_FLAG_OPERR
#define HAL_FLASH_ERROR_PROG      FLASH_FLAG_PROGERR
#define HAL_FLASH_ERROR_WRP       FLASH_FLAG_WRPERR
#define HAL_FLASH_ERROR_PGA       FLASH_FLAG_PGAERR
#define HAL_FLASH_ERROR_SIZ       FLASH_FLAG_SIZERR
#define HAL_FLASH_ERROR_PGS       FLASH_FLAG_PGSERR
#define HAL_FLASH_ERROR_RD        FLASH_FLAG_RDERR
#define HAL_FLASH_ERROR_OPTV      FLASH_FLAG_OPTVERR
#define HAL_FLASH_ERROR_OPTV_RX   FLASH_FLAG_OPTVERR_RX
#define HAL_FLASH_ERROR_ECCC      FLASH_FLAG_ECCC
#define HAL_FLASH_ERROR_ECCD      FLASH_FLAG_ECCD
#define HAL_FLASH_ERROR_ECCC2     FLASH_FLAG_ECCC2
#define HAL_FLASH_ERROR_ECCD2     FLASH_FLAG_ECCD2
/**
  * end of FLASH_Error @}
  */

/** @defgroup FLASH_Type_Erase FLASH Erase Type
  * @ingroup  FLASH_Parameter_Definitions
  * @{
  */

#define FLASH_TYPEERASE_PAGES     0x00U                    /*!<Pages erase only*/
#define FLASH_TYPEERASE_MASSERASE 0x01U                    /*!<Flash mass erase activation*/
/**
  * end of FLASH_Type_Erase @}
  */

/** @defgroup FLASH_Banks FLASH Banks
  * @ingroup  FLASH_Parameter_Definitions
  * @{
  */

#define FLASH_BANK_1              0x00000001U              /*!< Bank 1   */
#define FLASH_BANK_2              0x00000002U              /*!< Bank 2   */
#define FLASH_BANK_BOTH           (FLASH_BANK_1 | FLASH_BANK_2) /*!< Bank1 and Bank2  */
/**
  * end of FLASH_Banks @}
  */

/** @defgroup FLASH_Type_Program FLASH Program Type
  * @ingroup  FLASH_Parameter_Definitions
  * @{
  */

#define FLASH_TYPEPROGRAM_DOUBLEWORD          0x00U              /*!< Program a double-word (64-bit) at a specified address.*/
#define FLASH_TYPEPROGRAM_DOUBLEDOUBLEWORD    0x01U              /*!< Program a double-double-word (128-bit) at a specified address.*/
#define FLASH_TYPEPROGRAM_FAST                0x02U              /*!< Fast program a 32 row double-word (64-bit) at a specified address.
                                                                      And another 32 row double-word (64-bit) will be programmed */
#define FLASH_TYPEPROGRAM_FAST_AND_LAST       0x03U              /*!< Fast program a 32 row double-word (64-bit) at a specified address
                                                                      And this is the last 32 row double-word (64-bit) programmed */
/**
  * end of FLASH_Type_Program @}
  */

/** @defgroup FLASH_OB_Type FLASH Option Bytes Type
  * @ingroup  FLASH_Parameter_Definitions
  * @{
  */
#define OPTIONBYTE_WRP            0x01U                    /*!< WRP option byte configuration */
#define OPTIONBYTE_RDP            0x02U                    /*!< RDP option byte configuration */
#define OPTIONBYTE_USER           0x04U                    /*!< USER option byte configuration */
#define OPTIONBYTE_PCROP          0x08U                    /*!< PCROP option byte configuration */
#define OPTIONBYTE_BOOT_LOCK      0x10U                    /*!< Boot lock option byte configuration */
#define OPTIONBYTE_SEC            0x20U                    /*!< Securable memory option byte configuration */
/**
  * end of FLASH_OB_Type @}
  */

/** @defgroup FLASH_OB_WRP_Area FLASH WRP Area
  * @ingroup  FLASH_Parameter_Definitions
  * @{
  */

#define OB_WRPAREA_BANK1_AREAA    0x00U                    /*!< Flash Bank 1 Area A */
#define OB_WRPAREA_BANK1_AREAB    0x01U                    /*!< Flash Bank 1 Area B */
#define OB_WRPAREA_BANK2_AREAA    0x02U                    /*!< Flash Bank 2 Area A */
#define OB_WRPAREA_BANK2_AREAB    0x04U                    /*!< Flash Bank 2 Area B */

/**
  * end of FLASH_OB_WRP_Area @}
  */

/** @defgroup FLASH_OB_Read_Protection FLASH Option Bytes Read Protection
  * @ingroup  FLASH_Parameter_Definitions
  * @{
  */
#define OB_RDP_LEVEL_0            0xAAU
#define OB_RDP_LEVEL_1            0xBBU
#define OB_RDP_LEVEL_2            0xCCU                    /*!< Warning: When enabling read protection level 2
                                                                it's no more possible to go back to level 1 or 0 */
/**
  * end of FLASH_OB_Read_Protection @}
  */

/** @defgroup FLASH_OB_USER_Type FLASH Option Bytes User Type
  * @ingroup  FLASH_Parameter_Definitions
  * @{
  */
#define OB_USER_BOR_LEV           0x00000001U              /*!< BOR reset Level */
#define OB_USER_nRST_STOP         0x00000002U              /*!< Reset generated when entering the stop mode */
#define OB_USER_nRST_STDBY        0x00000004U              /*!< Reset generated when entering the standby mode */
#define OB_USER_IWDG_SW           0x00000008U              /*!< Independent watchdog selection */
#define OB_USER_IWDG_STOP         0x00000010U              /*!< Independent watchdog counter freeze in stop mode */
#define OB_USER_IWDG_STDBY        0x00000020U              /*!< Independent watchdog counter freeze in standby mode */
#define OB_USER_WWDG_SW           0x00000040U              /*!< Window watchdog selection */
#define OB_USER_BFB2              0x00000080U              /*!< Dual-bank boot */
#define OB_USER_DBANK             0x00000100U              /*!< Single bank with 128-bits data or two banks with 64-bits data */
#define OB_USER_nBOOT1            0x00000200U              /*!< Boot configuration */
#define OB_USER_SRAM_PE           0x00000400U              /*!< SRAM parity check enable (first 32kB of SRAM1 + CCM SRAM) */
#define OB_USER_CCMSRAM_RST       0x00000800U              /*!< CCMSRAM Erase when system reset */
#define OB_USER_nRST_SHDW         0x00001000U              /*!< Reset generated when entering the shutdown mode */
#define OB_USER_nSWBOOT0          0x00002000U              /*!< Software BOOT0 */
#define OB_USER_nBOOT0            0x00004000U              /*!< nBOOT0 option bit */
#define OB_USER_IRHEN             0x00010000U              /*!< Internal Reset Holder enable */
/**
  * end of FLASH_OB_USER_Type @}
  */

/** @defgroup FLASH_OB_USER_nRST_STOP FLASH Option Bytes User Reset On Stop
  * @ingroup  FLASH_Parameter_Definitions
  * @{
  */
#define OB_STOP_RST               0x00000000U              /*!< Reset generated when entering the stop mode */
#define OB_STOP_NORST             FLASH_OPTR_nRST_STOP     /*!< No reset generated when entering the stop mode */
/**
  * end of FLASH_OB_USER_nRST_STOP @}
  */

/** @defgroup FLASH_OB_USER_nRST_STANDBY FLASH Option Bytes User Reset On Standby
  * @ingroup  FLASH_Parameter_Definitions
  * @{
  */
#define OB_STANDBY_RST            0x00000000U              /*!< Reset generated when entering the standby mode */
#define OB_STANDBY_NORST          FLASH_OPTR_nRST_STDBY    /*!< No reset generated when entering the standby mode */
/**
  * end of FLASH_OB_USER_nRST_STANDBY @}
  */

/** @defgroup FLASH_OB_USER_IWDG_SW FLASH Option Bytes User IWDG Type
  * @ingroup  FLASH_Parameter_Definitions
  * @{
  */
#define OB_IWDG_HW                0x00000000U              /*!< Hardware independent watchdog */
#define OB_IWDG_SW                FLASH_OPTR_IWDG_SW       /*!< Software independent watchdog */
/**
  * end of FLASH_OB_USER_IWDG_SW @}
  */

/** @defgroup FLASH_OB_USER_IWDG_STOP FLASH Option Bytes User IWDG Mode On Stop
  * @ingroup  FLASH_Parameter_Definitions
  * @{
  */
#define OB_IWDG_STOP_FREEZE       0x00000000U              /*!< Independent watchdog counter is frozen in Stop mode */
#define OB_IWDG_STOP_RUN          FLASH_OPTR_IWDG_STOP     /*!< Independent watchdog counter is running in Stop mode */
/**
  * end of FLASH_OB_USER_IWDG_STOP @}
  */

/** @defgroup FLASH_OB_USER_IWDG_STANDBY FLASH Option Bytes User IWDG Mode On Standby
  * @ingroup  FLASH_Parameter_Definitions
  * @{
  */
#define OB_IWDG_STDBY_FREEZE      0x00000000U              /*!< Independent watchdog counter is frozen in Standby mode */
#define OB_IWDG_STDBY_RUN         FLASH_OPTR_IWDG_STDBY    /*!< Independent watchdog counter is running in Standby mode */
/**
  * end of FLASH_OB_USER_IWDG_STANDBY @}
  */

/** @defgroup FLASH_OB_USER_WWDG_SW FLASH Option Bytes User WWDG Type
  * @ingroup  FLASH_Parameter_Definitions
  * @{
  */
#define OB_WWDG_HW                0x00000000U              /*!< Hardware window watchdog */
#define OB_WWDG_SW                FLASH_OPTR_WWDG_SW       /*!< Software window watchdog */
/**
  * end of FLASH_OB_USER_WWDG_SW @}
  */

/** @defgroup FLASH_OB_USER_BFB2 FLASH Option Bytes User BFB2 Mode
  * @ingroup  FLASH_Parameter_Definitions
  * @{
  */
#define OB_BFB2_DISABLE           0x00000000U              /*!< Dual-bank boot disable */
#define OB_BFB2_ENABLE            FLASH_OPTR_BFB2          /*!< Dual-bank boot enable */
/**
  * end of FLASH_OB_USER_BFB2 @}
  */

/** @defgroup FLASH_OB_USER_DBANK FLASH Option Bytes User DBANK Type
  * @ingroup  FLASH_Parameter_Definitions
  * @{
  */
#define OB_DBANK_128_BITS         0x00000000U              /*!< Single-bank with 128-bits data */
#define OB_DBANK_64_BITS          FLASH_OPTR_DBANK         /*!< Dual-bank with 64-bits data */
/**
  * end of FLASH_OB_USER_DBANK @}
  */

/** @defgroup FLASH_OB_USER_nBOOT1 FLASH Option Bytes User BOOT1 Type
  * @ingroup  FLASH_Parameter_Definitions
  * @{
  */
#define OB_BOOT1_SRAM             0x00000000U              /*!< Embedded SRAM1 is selected as boot space (if BOOT0=1) */
#define OB_BOOT1_SYSTEM           FLASH_OPTR_nBOOT1        /*!< System memory is selected as boot space (if BOOT0=1) */
/**
  * end of FLASH_OB_USER_nBOOT1 @}
  */

/** @defgroup FLASH_OB_USER_SRAM_PE FLASH Option Bytes User SRAM Parity Check Type
  * @ingroup  FLASH_Parameter_Definitions
  * @{
  */
#define OB_SRAM_PARITY_ENABLE     0x00000000U              /*!< SRAM parity check enable (first 32kB of SRAM1 + CCM SRAM) */
#define OB_SRAM_PARITY_DISABLE    FLASH_OPTR_SRAM_PE       /*!< SRAM parity check disable (first 32kB of SRAM1 + CCM SRAM) */
/**
  * end of FLASH_OB_USER_SRAM_PE @}
  */

/** @defgroup FLASH_OB_USER_CCMSRAM_RST FLASH Option Bytes User CCMSRAM Erase On Reset Type
  * @ingroup  FLASH_Parameter_Definitions
  * @{
  */
#define OB_CCMSRAM_RST_ERASE      0x00000000U              /*!< CCMSRAM erased when a system reset occurs */
#define OB_CCMSRAM_RST_NOT_ERASE  FLASH_OPTR_CCMSRAM_RST   /*!< CCMSRAM is not erased when a system reset occurs */
/**
  * end of FLASH_OB_USER_CCMSRAM_RST @}
  */

/** @defgroup FLASH_OB_USER_nSWBOOT0 FLASH Option Bytes User Software BOOT0
  * @ingroup  FLASH_Parameter_Definitions
  * @{
  */
#define OB_BOOT0_FROM_OB          0x00000000U              /*!< BOOT0 taken from the option bit nBOOT0 */
#define OB_BOOT0_FROM_PIN         FLASH_OPTR_nSWBOOT0      /*!< BOOT0 taken from PB8/BOOT0 pin */
/**
  * end of FLASH_OB_USER_nSWBOOT0 @}
  */

/** @defgroup FLASH_OB_USER_nBOOT0 FLASH Option Bytes User nBOOT0 option bit
  * @ingroup  FLASH_Parameter_Definitions
  * @{
  */
#define OB_nBOOT0_RESET           0x00000000U              /*!< nBOOT0 = 0 */
#define OB_nBOOT0_SET             FLASH_OPTR_nBOOT0        /*!< nBOOT0 = 1 */
/**
  * end of FLASH_OB_USER_nBOOT0 @}
  */

/** @defgroup FLASH_Latency FLASH Latency
  * @ingroup  FLASH_Parameter_Definitions
  * @{
  */
#define FLASH_LATENCY_0           (0x00000000)                                                                  /*!< FLASH Zero  Latency cycle  */
#define FLASH_LATENCY_1           (                                                  FLASH_ACR_LATENCT_BIT0)    /*!< FLASH One   Latency cycle  */
#define FLASH_LATENCY_2           (                         FLASH_ACR_LATENCT_BIT1                         )    /*!< FLASH Two   Latency cycles */
#define FLASH_LATENCY_3           (                         FLASH_ACR_LATENCT_BIT1 | FLASH_ACR_LATENCT_BIT0)    /*!< FLASH Three Latency cycles */
#define FLASH_LATENCY_4           (FLASH_ACR_LATENCT_BIT2                                                  )    /*!< FLASH Four  Latency cycles */
/**
  * end of FLASH_Latency @}
  */

/** @defgroup FLASH_Keys FLASH Keys
  * @ingroup  FLASH_Parameter_Definitions
  * @{
  */
#define FLASH_KEY1                0x45670123U               /*!< Flash key1 */
#define FLASH_KEY2                0xCDEF89ABU               /*!< Flash key2: used with FLASH_KEY1
                                                                 to unlock the FLASH registers access */

#define FLASH_OPTKEY1             0x08192A3BU               /*!< Flash option byte key1 */
#define FLASH_OPTKEY2             0x4C5D6E7FU               /*!< Flash option byte key2: used with FLASH_OPTKEY1
                                                                 to allow option bytes operations */
/**
  * end of FLASH_Keys @}
  */

/** @defgroup FLASH_Flags FLASH Flags Definition
  * @ingroup  FLASH_Parameter_Definitions
  * @{
  */
#define FLASH_FLAG_EOP            FLASH_SR_EOP             /*!< FLASH End of operation flag */
#define FLASH_FLAG_OPERR          FLASH_SR_OPERR           /*!< FLASH Operation error flag */
#define FLASH_FLAG_PROGERR        FLASH_SR_PROGERR         /*!< FLASH Programming error flag */
#define FLASH_FLAG_WRPERR         FLASH_SR_WRPERR          /*!< FLASH Write protection error flag */
#define FLASH_FLAG_PGAERR         FLASH_SR_PGAERR          /*!< FLASH Programming alignment error flag */
#define FLASH_FLAG_SIZERR         FLASH_SR_SIZERR          /*!< FLASH Size error flag  */
#define FLASH_FLAG_PGSERR         FLASH_SR_PGSERR          /*!< FLASH Programming sequence error flag */
#define FLASH_FLAG_RDERR          FLASH_SR_RDERR           /*!< FLASH PCROP read error flag */
#define FLASH_FLAG_OPTVERR        FLASH_SR_OPTVERR         /*!< FLASH Option validity error flag */
#define FLASH_FLAG_OPTVERR_RX     FLASH_SR_OPTVERR_RX      /*!< FLASH RX_Option validity error flag */
#define FLASH_FLAG_BSY            FLASH_SR_BSY             /*!< FLASH Busy flag */
#define FLASH_FLAG_ECCC           FLASH_ECCR_ECCC          /*!< FLASH ECC correction in 64 LSB bits */
#define FLASH_FLAG_ECCD           FLASH_ECCR_ECCD          /*!< FLASH ECC detection in 64 LSB bits */
#define FLASH_FLAG_ECCC2          FLASH_ECCR_ECCC2         /*!< FLASH ECC correction in 64 MSB bits (mode 128 bits only) */
#define FLASH_FLAG_ECCD2          FLASH_ECCR_ECCD2         /*!< FLASH ECC detection in 64 MSB bits (mode 128 bits only) */

#define FLASH_FLAG_SR_ERRORS      (FLASH_FLAG_OPERR   | FLASH_FLAG_PROGERR | FLASH_FLAG_WRPERR | \
                                   FLASH_FLAG_PGAERR  | FLASH_FLAG_SIZERR  | FLASH_FLAG_PGSERR | \
                                   FLASH_FLAG_RDERR   | FLASH_FLAG_OPTVERR | FLASH_FLAG_OPTVERR_RX )

#define FLASH_FLAG_ECCR_ERRORS    (FLASH_FLAG_ECCC    | FLASH_FLAG_ECCD    | FLASH_FLAG_ECCC2  | FLASH_FLAG_ECCD2)
#define FLASH_FLAG_ALL_ERRORS     (FLASH_FLAG_SR_ERRORS | FLASH_FLAG_ECCR_ERRORS)
/**
  * end of FLASH_Flags @}
  */

/** @defgroup FLASH_Interrupt_definition FLASH Interrupts Definition
  * @ingroup  FLASH_Parameter_Definitions
  * @{
  */

/**
  * @brief FLASH Interrupt definition
  */

#define FLASH_IT_EOP              FLASH_CR_EOPIE           /*!< End of FLASH Operation Interrupt source */
#define FLASH_IT_OPERR            FLASH_CR_ERRIE           /*!< Error Interrupt source */
#define FLASH_IT_RDERR            FLASH_CR_RDERRIE         /*!< PCROP Read Error Interrupt source*/
#define FLASH_IT_ECCC            (FLASH_ECCR_ECCCIE >> 24U) /*!< ECC Correction Interrupt source */
/**
  * end of FLASH_Interrupt_definition @}
  */

/**
  * end of FLASH_Parameter_Definitions @}
  */

/******************************************************************************/
/*                              FLASH Macro                                   */
/******************************************************************************/

/* Exported macros -----------------------------------------------------------*/

/** @defgroup FLASH_Macro_Definitions FLASH Macro Definitions
  * @{
  */

/**
  * @brief  Set the FLASH Latency.
  * @param  __LATENCY__ FLASH Latency.
  *         This parameter can be one of the following values :
  *           @arg FLASH_LATENCY_0:  FLASH Zero wait state
  *           @arg FLASH_LATENCY_1:  FLASH One wait state
  *           @arg FLASH_LATENCY_2:  FLASH Two wait states
  *           @arg FLASH_LATENCY_3:  FLASH Three wait states
  *           @arg FLASH_LATENCY_4:  FLASH Four wait states
  * @retval None
  */
#define __HAL_FLASH_SET_LATENCY(__LATENCY__)    MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY, (__LATENCY__))

/**
  * @brief  Get the FLASH Latency.
  * @retval FLASH_Latency.
  *         This parameter can be one of the following values :
  *           @arg FLASH_LATENCY_0:  FLASH Zero wait state
  *           @arg FLASH_LATENCY_1:  FLASH One wait state
  *           @arg FLASH_LATENCY_2:  FLASH Two wait states
  *           @arg FLASH_LATENCY_3:  FLASH Three wait states
  *           @arg FLASH_LATENCY_4:  FLASH Four wait states
  */
#define __HAL_FLASH_GET_LATENCY()               READ_BIT(FLASH->ACR, FLASH_ACR_LATENCY)

/**
  * @brief  Enable the FLASH prefetch buffer.
  * @retval None
  */
#define __HAL_FLASH_PREFETCH_BUFFER_ENABLE()    SET_BIT(FLASH->ACR, FLASH_ACR_PRFTEN)

/**
  * @brief  Disable the FLASH prefetch buffer.
  * @retval None
  */
#define __HAL_FLASH_PREFETCH_BUFFER_DISABLE()   CLEAR_BIT(FLASH->ACR, FLASH_ACR_PRFTEN)

/**
  * @brief  Enable the FLASH instruction cache.
  * @retval None
  */
#define __HAL_FLASH_INSTRUCTION_CACHE_ENABLE()  SET_BIT(FLASH->ACR, FLASH_ACR_ICEN)

/**
  * @brief  Disable the FLASH instruction cache.
  * @retval None
  */
#define __HAL_FLASH_INSTRUCTION_CACHE_DISABLE() CLEAR_BIT(FLASH->ACR, FLASH_ACR_ICEN)

/**
  * @brief  Enable the FLASH data cache.
  * @retval None
  */
#define __HAL_FLASH_DATA_CACHE_ENABLE()         SET_BIT(FLASH->ACR, FLASH_ACR_DCEN)

/**
  * @brief  Disable the FLASH data cache.
  * @retval None
  */
#define __HAL_FLASH_DATA_CACHE_DISABLE()        CLEAR_BIT(FLASH->ACR, FLASH_ACR_DCEN)

/**
  * @brief  Reset the FLASH instruction Cache.
  * @note   This function must be used only when the Instruction Cache is disabled.
  * @retval None
  */
#define __HAL_FLASH_INSTRUCTION_CACHE_RESET()   do { SET_BIT(FLASH->ACR, FLASH_ACR_ICRST);   \
                                                     CLEAR_BIT(FLASH->ACR, FLASH_ACR_ICRST); \
                                                   } while (0)

/**
  * @brief  Reset the FLASH data Cache.
  * @note   This function must be used only when the data Cache is disabled.
  * @retval None
  */
#define __HAL_FLASH_DATA_CACHE_RESET()          do { SET_BIT(FLASH->ACR, FLASH_ACR_DCRST);   \
                                                     CLEAR_BIT(FLASH->ACR, FLASH_ACR_DCRST); \
                                                   } while (0)

/** @defgroup FLASH_Interrupt FLASH Interrupts Macros
  *  @brief macros to handle FLASH interrupts
  * @{
  */

/**
  * @brief  Enable the specified FLASH interrupt.
  * @param  __INTERRUPT__ FLASH interrupt
  *         This parameter can be any combination of the following values:
  *           @arg FLASH_IT_EOP:   End of FLASH Operation Interrupt
  *           @arg FLASH_IT_OPERR: Error Interrupt
  *           @arg FLASH_IT_RDERR: PCROP Read Error Interrupt
  *           @arg FLASH_IT_ECCC:  ECC Correction Interrupt
  * @retval None
  */
#define __HAL_FLASH_ENABLE_IT(__INTERRUPT__)    do { if(((__INTERRUPT__) & FLASH_IT_ECCC) != 0U) { SET_BIT(FLASH->ECCR, FLASH_ECCR_ECCCIE); }\
                                                     if(((__INTERRUPT__) & (~FLASH_IT_ECCC)) != 0U) { SET_BIT(FLASH->CR, ((__INTERRUPT__) & (~FLASH_IT_ECCC))); }\
                                                   } while (0)

/**
  * @brief  Disable the specified FLASH interrupt.
  * @param  __INTERRUPT__ FLASH interrupt
  *         This parameter can be any combination of the following values:
  *           @arg FLASH_IT_EOP:   End of FLASH Operation Interrupt
  *           @arg FLASH_IT_OPERR: Error Interrupt
  *           @arg FLASH_IT_RDERR: PCROP Read Error Interrupt
  *           @arg FLASH_IT_ECCC:  ECC Correction Interrupt
  * @retval None
  */
#define __HAL_FLASH_DISABLE_IT(__INTERRUPT__)   do { if(((__INTERRUPT__) & FLASH_IT_ECCC) != 0U) { CLEAR_BIT(FLASH->ECCR, FLASH_ECCR_ECCCIE); }\
                                                     if(((__INTERRUPT__) & (~FLASH_IT_ECCC)) != 0U) { CLEAR_BIT(FLASH->CR, ((__INTERRUPT__) & (~FLASH_IT_ECCC))); }\
                                                   } while (0)

/**
  * @brief  Check whether the specified FLASH flag is set or not.
  * @param  __FLAG__ specifies the FLASH flag to check.
  *         This parameter can be one of the following values:
  *           @arg FLASH_FLAG_EOP:        FLASH End of Operation flag
  *           @arg FLASH_FLAG_OPERR:      FLASH Operation error flag
  *           @arg FLASH_FLAG_PROGERR:    FLASH Programming error flag
  *           @arg FLASH_FLAG_WRPERR:     FLASH Write protection error flag
  *           @arg FLASH_FLAG_PGAERR:     FLASH Programming alignment error flag
  *           @arg FLASH_FLAG_SIZERR:     FLASH Size error flag
  *           @arg FLASH_FLAG_PGSERR:     FLASH Programming sequence error flag
  *           @arg FLASH_FLAG_MISERR:     FLASH Fast programming data miss error flag
  *           @arg FLASH_FLAG_FASTERR:    FLASH Fast programming error flag
  *           @arg FLASH_FLAG_RDERR:      FLASH PCROP read  error flag
  *           @arg FLASH_FLAG_OPTVERR:    FLASH Option validity error flag
  *           @arg FLASH_FLAG_OPTVERR_RX: FLASH RX_Option validity error flag
  *           @arg FLASH_FLAG_BSY:        FLASH write/erase operations in progress flag
  *           @arg FLASH_FLAG_ECCC:       FLASH one ECC error has been detected and corrected in 64 LSB bits
  *           @arg FLASH_FLAG_ECCD:       FLASH two ECC errors have been detected in 64 LSB bits
  *           @arg FLASH_FLAG_ECCC2(*):   FLASH one ECC error has been detected and corrected in 64 MSB bits (mode 128 bits only)
  *           @arg FLASH_FLAG_ECCD2(*):   FLASH two ECC errors have been detected in 64 MSB bits (mode 128 bits only)
  * @note  (*) availability depends on devices
  * @retval The new state of FLASH_FLAG (SET or RESET).
  */
#define __HAL_FLASH_GET_FLAG(__FLAG__)          ((((__FLAG__) & FLASH_FLAG_ECCR_ERRORS) != 0U) ? \
                                                 (READ_BIT(FLASH->ECCR, (__FLAG__)) == (__FLAG__)) : \
                                                 (READ_BIT(FLASH->SR,   (__FLAG__)) == (__FLAG__)))

/**
  * @brief  Clear the FLASH's pending flags.
  * @param  __FLAG__ specifies the FLASH flags to clear.
  *         This parameter can be any combination of the following values:
  *           @arg FLASH_FLAG_EOP:         FLASH End of Operation flag
  *           @arg FLASH_FLAG_OPERR:       FLASH Operation error flag
  *           @arg FLASH_FLAG_PROGERR:     FLASH Programming error flag
  *           @arg FLASH_FLAG_WRPERR:      FLASH Write protection error flag
  *           @arg FLASH_FLAG_PGAERR:      FLASH Programming alignment error flag
  *           @arg FLASH_FLAG_SIZERR:      FLASH Size error flag
  *           @arg FLASH_FLAG_PGSERR:      FLASH Programming sequence error flag
  *           @arg FLASH_FLAG_MISERR:      FLASH Fast programming data miss error flag
  *           @arg FLASH_FLAG_FASTERR:     FLASH Fast programming error flag
  *           @arg FLASH_FLAG_RDERR:       FLASH PCROP read  error flag
  *           @arg FLASH_FLAG_OPTVERR:     FLASH Option validity error flag
  *           @arg FLASH_FLAG_OPTVERR_RX:  FLASH RX_Option validity error flag
  *           @arg FLASH_FLAG_ECCC:        FLASH one ECC error has been detected and corrected in 64 LSB bits
  *           @arg FLASH_FLAG_ECCD:        FLASH two ECC errors have been detected in 64 LSB bits
  *           @arg FLASH_FLAG_ECCC2(*):    FLASH one ECC error has been detected and corrected in 64 MSB bits (mode 128 bits only)
  *           @arg FLASH_FLAG_ECCD2(*):    FLASH two ECC errors have been detected in 64 MSB bits (mode 128 bits only)
  *           @arg FLASH_FLAG_SR_ERRORS:   FLASH All SR errors flags
  *           @arg FLASH_FLAG_ECCR_ERRORS: FLASH All ECCR errors flags
  * @note  (*) availability depends on devices
  * @retval None
  */
#define __HAL_FLASH_CLEAR_FLAG(__FLAG__)        do { if(((__FLAG__) & FLASH_FLAG_ECCR_ERRORS) != 0U) { SET_BIT(FLASH->ECCR, ((__FLAG__) & FLASH_FLAG_ECCR_ERRORS)); }\
                                                     if(((__FLAG__) & ~(FLASH_FLAG_ECCR_ERRORS)) != 0U) { WRITE_REG(FLASH->SR, ((__FLAG__) & ~(FLASH_FLAG_ECCR_ERRORS))); }\
                                                   } while (0)
/**
  * end of FLASH_Macro_Definitions @}
  */

/* Include FLASH HAL Extended module */
#include "rx32g4xx_hal_flash_ex.h"


/* Exported variables ---------------------------------------------------------*/

extern HAL_FLASH_ProcessTypeDef HAL_pFlash;

/******************************************************************************/
/*                                FLASH Functions                             */
/******************************************************************************/

/** @defgroup FLASH_Function_Definitions FLASH Function Definitions
  * @{
  */

/* Exported functions --------------------------------------------------------*/

HAL_StatusTypeDef  HAL_FLASH_Program(uint32_t TypeProgram, uint32_t Address, uint32_t* Data);
HAL_StatusTypeDef  HAL_FLASH_Program_IT(uint32_t TypeProgram, uint32_t Address, uint32_t* Data);
/* FLASH IRQ handler method */
void               HAL_FLASH_IRQHandler(void);
/* Callbacks in non blocking modes */
void               HAL_FLASH_EndOfOperationCallback(uint32_t ReturnValue);
void               HAL_FLASH_OperationErrorCallback(uint32_t ReturnValue);

/* Peripheral Control functions  **********************************************/

HAL_StatusTypeDef  HAL_FLASH_Unlock(void);
HAL_StatusTypeDef  HAL_FLASH_Lock(void);

/* Option bytes control */
HAL_StatusTypeDef  HAL_FLASH_OB_Unlock(void);
HAL_StatusTypeDef  HAL_FLASH_OB_Lock(void);
HAL_StatusTypeDef  HAL_FLASH_OB_Launch(void);
void  FLASH_Set_UserOptionByte(uint32_t OB_USER);
void  FLASH_Clear_UserOptionByte(uint32_t OB_USER);

/* Peripheral State functions  ************************************************/
uint32_t HAL_FLASH_GetError(void);
HAL_StatusTypeDef  HAL_FLASH_WaitForLastOperation(uint32_t Timeout);
/**
  * end of FLASH_Function_Definitions @}
  */

/******************************************************************************/
/*                            FLASH Parameters                                */
/******************************************************************************/

/* Private constants --------------------------------------------------------*/

/** @defgroup FLASH_Parameter_Definitions FLASH Parameter Definitions
  * @{
  */

/**
  * @brief  FLASH Size
  */

#define FLASH_SIZE_DATA_REGISTER        FLASHSIZE_BASE

#define FLASH_SIZE                      ((((*((uint16_t *)FLASH_SIZE_DATA_REGISTER)) == 0xFFFFU)) ? (0x200UL << 10U) : \
                                        (((*((uint32_t *)FLASH_SIZE_DATA_REGISTER)) & 0xFFFFUL) << 10U))
#define FLASH_BANK_SIZE                 (FLASH_SIZE >> 1)
#define FLASH_PAGE_NB                   ((FLASH_SIZE == 0x00080000U) ? 512U : \
                                        ((FLASH_SIZE == 0x00040000U) ? 256U : 128U))

#define FLASH_PAGE_SIZE_128_BITS        0x400U  /* 1 KB */

#define FLASH_PAGE_SIZE                 0x400U  /* 1 KB */

#define FLASH_TIMEOUT_VALUE             1000U   /* 1 s  */


/**
  * end of FLASH_Parameter_Definitions @}
  */

/******************************************************************************/
/*                              FLASH Macro                                   */
/******************************************************************************/

/* Private macros ------------------------------------------------------------*/

/** @addtogroup FLASH_Macros_Definitions FLASH Macros Definitions
  *  @{
  */
/**
  * @brief  Check if the parameter __VALUE__ is valid
  * @param  __VALUE__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_FLASH_TYPEERASE(__VALUE__)          (((__VALUE__) == FLASH_TYPEERASE_PAGES) || \
                                                ((__VALUE__) == FLASH_TYPEERASE_MASSERASE))

#define IS_FLASH_TYPEPROGRAM(__VALUE__)        (((__VALUE__) == FLASH_TYPEPROGRAM_DOUBLEWORD) ||       \
                                                ((__VALUE__) == FLASH_TYPEPROGRAM_DOUBLEDOUBLEWORD) || \
                                                ((__VALUE__) == FLASH_TYPEPROGRAM_FAST) ||             \
                                                ((__VALUE__) == FLASH_TYPEPROGRAM_FAST_AND_LAST))

#define IS_OPTIONBYTE(__VALUE__)               (((__VALUE__) <= (OPTIONBYTE_WRP | OPTIONBYTE_RDP | OPTIONBYTE_USER | OPTIONBYTE_PCROP | \
                                                                 OPTIONBYTE_BOOT_LOCK | OPTIONBYTE_SEC)))

#define IS_OB_WRPAREA(VALUE)                   (((VALUE) == OB_WRPAREA_BANK1_AREAA) || ((VALUE) == OB_WRPAREA_BANK1_AREAB) || \
                                                ((VALUE) == OB_WRPAREA_BANK2_AREAA) || ((VALUE) == OB_WRPAREA_BANK2_AREAB))

#define IS_OB_USER_STOP(__VALUE__)             (((__VALUE__) == OB_STOP_RST) || ((__VALUE__) == OB_STOP_NORST))

#define IS_OB_USER_STANDBY(__VALUE__)          (((__VALUE__) == OB_STANDBY_RST) || ((__VALUE__) == OB_STANDBY_NORST))

#define IS_OB_USER_IWDG(__VALUE__)             (((__VALUE__) == OB_IWDG_HW) || ((__VALUE__) == OB_IWDG_SW))

#define IS_OB_USER_IWDG_STOP(__VALUE__)        (((__VALUE__) == OB_IWDG_STOP_FREEZE) || ((__VALUE__) == OB_IWDG_STOP_RUN))

#define IS_OB_USER_IWDG_STDBY(__VALUE__)       (((__VALUE__) == OB_IWDG_STDBY_FREEZE) || ((__VALUE__) == OB_IWDG_STDBY_RUN))

#define IS_OB_USER_WWDG(__VALUE__)             (((__VALUE__) == OB_WWDG_HW) || ((__VALUE__) == OB_WWDG_SW))

#define IS_OB_USER_BFB2(__VALUE__)             (((__VALUE__) == OB_BFB2_DISABLE) || ((__VALUE__) == OB_BFB2_ENABLE))

#define IS_OB_USER_DBANK(__VALUE__)            (((__VALUE__) == OB_DBANK_128_BITS) || ((__VALUE__) == OB_DBANK_64_BITS))

#define IS_OB_USER_BOOT1(__VALUE__)            (((__VALUE__) == OB_BOOT1_SRAM) || ((__VALUE__) == OB_BOOT1_SYSTEM))

#define IS_OB_USER_SRAM_PARITY(__VALUE__)      (((__VALUE__) == OB_SRAM_PARITY_ENABLE) || ((__VALUE__) == OB_SRAM_PARITY_DISABLE))

#define IS_OB_USER_CCMSRAM_RST(__VALUE__)      (((__VALUE__) == OB_CCMSRAM_RST_ERASE) || ((__VALUE__) == OB_CCMSRAM_RST_NOT_ERASE))

#define IS_OB_USER_SWBOOT0(__VALUE__)          (((__VALUE__) == OB_BOOT0_FROM_OB) || ((__VALUE__) == OB_BOOT0_FROM_PIN))

#define IS_OB_USER_BOOT0(__VALUE__)            (((__VALUE__) == OB_nBOOT0_RESET) || ((__VALUE__) == OB_nBOOT0_SET))

#define IS_OB_SECMEM_SIZE(__VALUE__)            ((__VALUE__) <= FLASH_PAGE_NB)

/**
  * @brief  Check if the parameter __BANK__ is valid
  * @param  __BANK__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_FLASH_BANK(__BANK__)                (((__BANK__) == FLASH_BANK_1)  || \
                                                ((__BANK__) == FLASH_BANK_2)  || \
                                                ((__BANK__) == FLASH_BANK_BOTH))

#define IS_FLASH_BANK_EXCLUSIVE(__BANK__)      (((__BANK__) == FLASH_BANK_1)  || \
                                                ((__BANK__) == FLASH_BANK_2))

/**
  * @brief  Check if the parameter __ADDRESS__ is valid
  * @param  __ADDRESS__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_FLASH_MAIN_MEM_ADDRESS(__ADDRESS__) (((__ADDRESS__) >= FLASH_BASE) && ((__ADDRESS__) < (FLASH_BASE+FLASH_SIZE)))

#define IS_FLASH_OTP_ADDRESS(__ADDRESS__)      (((__ADDRESS__) >= 0x1FFF7000U) && ((__ADDRESS__) <= 0x1FFF73FFU))

#define IS_FLASH_PROGRAM_ADDRESS(__ADDRESS__)  (IS_FLASH_MAIN_MEM_ADDRESS(__ADDRESS__) || IS_FLASH_OTP_ADDRESS(__ADDRESS__))

/**
  * @brief  Check if the parameter __PAGE__ is valid
  * @param  __PAGE__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_FLASH_PAGE(__PAGE__)                ((__PAGE__) < FLASH_PAGE_NB)

/**
  * @brief  Check if the parameter __LEVEL__ is valid
  * @param  __LEVEL__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_OB_RDP_LEVEL(__LEVEL__)             (((__LEVEL__) == OB_RDP_LEVEL_0) ||\
                                                ((__LEVEL__) == OB_RDP_LEVEL_1) ||\
                                                ((__LEVEL__) == OB_RDP_LEVEL_2))
/**
  * @brief  Check if the parameter __TYPE__ is valid
  * @param  __TYPE__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_OB_USER_TYPE(__TYPE__)              (((__TYPE__) <= 0x1FFFFU) && ((__TYPE__) != 0U))

/**
  * @brief  Check if the parameter __LATENCY__ is valid
  * @param  __LATENCY__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_FLASH_LATENCY(__LATENCY__)          (((__LATENCY__) == FLASH_LATENCY_0) || ((__LATENCY__) == FLASH_LATENCY_1) || \
                                                ((__LATENCY__) == FLASH_LATENCY_2) || ((__LATENCY__) == FLASH_LATENCY_3) || \
                                                ((__LATENCY__) == FLASH_LATENCY_4))

/**
  * end of FLASH_Macros_Definitions @}
  */

/**
  * end of FLASH @}
  */

#ifdef __cplusplus
}
#endif

#endif /* RX32G4xx_HAL_FLASH_H */

