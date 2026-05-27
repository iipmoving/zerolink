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
*   File    : rx32g4xx_hal_flash_ex.c
*   By      : RX_DV_Team
**************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal.h"

#ifdef HAL_FLASH_MODULE_ENABLED

/** @defgroup FLASH FLASH
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/** @defgroup FLASH_Private_Functions FLASH Private Functions
  * @{
  */
static void              FLASH_MassErase(uint32_t Banks);
static HAL_StatusTypeDef FLASH_OB_WRPConfig(uint32_t WRPArea, uint32_t WRPStartOffset, uint32_t WRDPEndOffset);
static HAL_StatusTypeDef FLASH_OB_RDPConfig(uint32_t RDPLevel);
static HAL_StatusTypeDef FLASH_OB_UserConfig(uint32_t UserType, uint32_t UserConfig);
static void              FLASH_OB_GetWRP(uint32_t WRPArea, uint32_t *WRPStartOffset, uint32_t *WRDPEndOffset);
static uint32_t          FLASH_OB_GetUser(void);
static uint32_t          FLASH_OB_GetRDP(void);

/**
  * end of FLASH_Private_Functions @}
  */

/* Exported functions -------------------------------------------------------*/

/** @defgroup FLASH_Function_Definitions FLASH Function Definitions
  * @{
  */

/**
  * @brief  Perform a mass erase or erase the specified FLASH memory pages.
  * @param[in]  pEraseInit pointer to an FLASH_EraseInitTypeDef structure that
  *         contains the configuration information for the erasing.
  * @param[out]  PageError pointer to variable that contains the configuration
  *         information on faulty page in case of error (0xFFFFFFFF means that all
  *         the pages have been correctly erased).
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_FLASHEx_Erase(FLASH_EraseInitTypeDef *pEraseInit, uint32_t *PageError)
{
  HAL_StatusTypeDef status;
  uint32_t page_index;

  /* Check the parameters */
  assert_param(IS_FLASH_TYPEERASE(pEraseInit->TypeErase));

  /* Clear all error flags */
  SET_BIT(FLASH->SR, FLASH_FLAG_SR_ERRORS);

  /* Clear Program bit */
  CLEAR_BIT(FLASH->CR, FLASH_CR_PG);

  /* Clear all Erase bit */
  CLEAR_BIT(FLASH->CR, FLASH_CR_PER | FLASH_CR_MER1 | FLASH_CR_MER2);

  /* Process Locked */
  __HAL_LOCK(&HAL_pFlash);

  /* Wait for last operation to be completed */
  status = HAL_FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);

  if (status == HAL_OK)
  {
    HAL_pFlash.ErrorCode = HAL_FLASH_ERROR_NONE;

    /* Deactivate the cache if they are activated to avoid data misbehavior */
    if (READ_BIT(FLASH->ACR, FLASH_ACR_ICEN) != 0U)
    {
      if (READ_BIT(FLASH->ACR, FLASH_ACR_DCEN) != 0U)
      {
        /* Disable data cache  */
        __HAL_FLASH_DATA_CACHE_DISABLE();
        HAL_pFlash.CacheToReactivate = FLASH_CACHE_ICACHE_DCACHE_ENABLED;
      }
      else
      {
        HAL_pFlash.CacheToReactivate = FLASH_CACHE_ICACHE_ENABLED;
      }
    }
    else if (READ_BIT(FLASH->ACR, FLASH_ACR_DCEN) != 0U)
    {
      /* Disable data cache  */
      __HAL_FLASH_DATA_CACHE_DISABLE();
      HAL_pFlash.CacheToReactivate = FLASH_CACHE_DCACHE_ENABLED;
    }
    else
    {
      HAL_pFlash.CacheToReactivate = FLASH_CACHE_DISABLED;
    }

    if (pEraseInit->TypeErase == FLASH_TYPEERASE_MASSERASE)
    {
      /* Mass erase to be done */
      FLASH_MassErase(pEraseInit->Banks);

      /* Wait for last operation to be completed */
      status = HAL_FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);

      /* If the erase operation is completed, disable the MER1 and MER2 Bits */
      CLEAR_BIT(FLASH->CR, (FLASH_CR_MER1 | FLASH_CR_MER2));
    }
    else
    {
      /*Initialization of PageError variable*/
      *PageError = 0xFFFFFFFFU;

      for (page_index = pEraseInit->Page; page_index < (pEraseInit->Page + pEraseInit->NbPages); page_index++)
      {
        FLASH_PageErase(page_index, pEraseInit->Banks);

        /* Wait for last operation to be completed */
        status = HAL_FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);

        /* If the erase operation is completed, disable the PER Bit */
        CLEAR_BIT(FLASH->CR, (FLASH_CR_PER | FLASH_CR_PNB));

        if (status != HAL_OK)
        {
          /* In case of error, stop erase procedure and return the faulty page */
          *PageError = page_index;
          break;
        }
      }
    }

    /* Flush the caches to be sure of the data consistency */
    HAL_FLASH_FlushCaches();
  }

  /* Process Unlocked */
  __HAL_UNLOCK(&HAL_pFlash);

  return status;
}

/**
  * @brief  Perform a mass erase or erase the specified FLASH memory pages with interrupt enabled.
  * @param  pEraseInit pointer to an FLASH_EraseInitTypeDef structure that
  *         contains the configuration information for the erasing.
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_FLASHEx_Erase_IT(FLASH_EraseInitTypeDef *pEraseInit)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Process Locked */
  __HAL_LOCK(&HAL_pFlash);

  /* Check the parameters */
  assert_param(IS_FLASH_TYPEERASE(pEraseInit->TypeErase));

  HAL_pFlash.ErrorCode = HAL_FLASH_ERROR_NONE;

  /* Deactivate the cache if they are activated to avoid data misbehavior */
  if (READ_BIT(FLASH->ACR, FLASH_ACR_ICEN) != 0U)
  {
    if (READ_BIT(FLASH->ACR, FLASH_ACR_DCEN) != 0U)
    {
      /* Disable data cache  */
      __HAL_FLASH_DATA_CACHE_DISABLE();
      HAL_pFlash.CacheToReactivate = FLASH_CACHE_ICACHE_DCACHE_ENABLED;
    }
    else
    {
      HAL_pFlash.CacheToReactivate = FLASH_CACHE_ICACHE_ENABLED;
    }
  }
  else if (READ_BIT(FLASH->ACR, FLASH_ACR_DCEN) != 0U)
  {
    /* Disable data cache  */
    __HAL_FLASH_DATA_CACHE_DISABLE();
    HAL_pFlash.CacheToReactivate = FLASH_CACHE_DCACHE_ENABLED;
  }
  else
  {
    HAL_pFlash.CacheToReactivate = FLASH_CACHE_DISABLED;
  }

  /* Enable End of Operation and Error interrupts */
  __HAL_FLASH_ENABLE_IT(FLASH_IT_EOP | FLASH_IT_OPERR);

  HAL_pFlash.Bank = pEraseInit->Banks;

  if (pEraseInit->TypeErase == FLASH_TYPEERASE_MASSERASE)
  {
    /* Mass erase to be done */
    HAL_pFlash.ProcedureOnGoing = FLASH_PROC_MASS_ERASE;
    FLASH_MassErase(pEraseInit->Banks);
  }
  else
  {
    /* Erase by page to be done */
    HAL_pFlash.ProcedureOnGoing = FLASH_PROC_PAGE_ERASE;
    HAL_pFlash.NbPagesToErase = pEraseInit->NbPages;
    HAL_pFlash.Page = pEraseInit->Page;

    /*Erase 1st page and wait for IT */
    FLASH_PageErase(pEraseInit->Page, pEraseInit->Banks);
  }

  return status;
}

/**
  * @brief  Program Option bytes.
  * @param  pOBInit pointer to an FLASH_OBInitStruct structure that
  *         contains the configuration information for the programming.
  * @note   To configure any option bytes, the option lock bit OPTLOCK must be
  *         cleared with the call of HAL_FLASH_OB_Unlock() function.
  * @note   New option bytes configuration will be taken into account in two cases:
  *         - after an option bytes launch through the call of HAL_FLASH_OB_Launch()
  *         - after a power reset (BOR reset or exit from Standby/Shutdown modes)
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_FLASHEx_OBProgram(FLASH_OBProgramInitTypeDef *pOBInit)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Check the parameters */
  assert_param(IS_OPTIONBYTE(pOBInit->OptionType));

  /* Process Locked */
  __HAL_LOCK(&HAL_pFlash);

  HAL_pFlash.ErrorCode = HAL_FLASH_ERROR_NONE;

  /* Write protection configuration */
  if ((pOBInit->OptionType & OPTIONBYTE_WRP) != 0U)
  {
    /* Configure of Write protection on the selected area */
    if (FLASH_OB_WRPConfig(pOBInit->WRPArea, pOBInit->WRPStartOffset, pOBInit->WRPEndOffset) != HAL_OK)
    {
      status = HAL_ERROR;
    }
  }

  /* Read protection configuration */
  if ((pOBInit->OptionType & OPTIONBYTE_RDP) != 0U)
  {
    /* Configure the Read protection level */
    if (FLASH_OB_RDPConfig(pOBInit->RDPLevel) != HAL_OK)
    {
      status = HAL_ERROR;
    }
  }

  /* User Configuration */
  if ((pOBInit->OptionType & OPTIONBYTE_USER) != 0U)
  {
    /* Configure the user option bytes */
    if (FLASH_OB_UserConfig(pOBInit->USERType, pOBInit->USERConfig) != HAL_OK)
    {
      status = HAL_ERROR;
    }
  }
  /* Process Unlocked */
  __HAL_UNLOCK(&HAL_pFlash);

  return status;
}

/**
  * @brief  Get the Option bytes configuration.
  * @param  pOBInit pointer to an FLASH_OBInitStruct structure that contains the
  *         configuration information.
  * @note   The fields pOBInit->WRPArea and pOBInit->PCROPConfig should indicate
  *         which area is requested for the WRP and PCROP, else no information will be returned.
  * @retval None
  */
void HAL_FLASHEx_OBGetConfig(FLASH_OBProgramInitTypeDef *pOBInit)
{
  pOBInit->OptionType = (OPTIONBYTE_RDP | OPTIONBYTE_USER);

  if ((pOBInit->WRPArea == OB_WRPAREA_BANK1_AREAA) || (pOBInit->WRPArea == OB_WRPAREA_BANK1_AREAB) ||
      (pOBInit->WRPArea == OB_WRPAREA_BANK2_AREAA) || (pOBInit->WRPArea == OB_WRPAREA_BANK2_AREAB))

  {
    pOBInit->OptionType |= OPTIONBYTE_WRP;
    /* Get write protection on the selected area */
    FLASH_OB_GetWRP(pOBInit->WRPArea, &(pOBInit->WRPStartOffset), &(pOBInit->WRPEndOffset));
  }

  /* Get Read protection level */
  pOBInit->RDPLevel = FLASH_OB_GetRDP();

  /* Get the user option bytes */
  pOBInit->USERConfig = FLASH_OB_GetUser();
}

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Mass erase of FLASH memory.
  * @param  Banks Banks to be erased.
  *         This parameter can be one of the following values:
  *            @arg FLASH_BANK_1: Bank1 to be erased
  *            @arg FLASH_BANK_2: Bank2 to be erased
  *            @arg FLASH_BANK_BOTH: Bank1 and Bank2 to be erased
  * @retval None
  */
static void FLASH_MassErase(uint32_t Banks)
{
  // Clear all error flag
  SET_BIT(FLASH->SR, FLASH_FLAG_ALL_ERRORS);

  if (READ_BIT(FLASH->OPTR, FLASH_OPTR_DBANK) != 0U)
  {
    /* Check the parameters */
    assert_param(IS_FLASH_BANK(Banks));

    /* Set the Mass Erase Bit for the bank 1 if requested */
    if ((Banks & FLASH_BANK_1) != 0U)
    {
      SET_BIT(FLASH->CR, FLASH_CR_MER1);
    }

    /* Set the Mass Erase Bit for the bank 2 if requested */
    if ((Banks & FLASH_BANK_2) != 0U)
    {
      SET_BIT(FLASH->CR, FLASH_CR_MER2);
    }
  }
  else
  {
    SET_BIT(FLASH->CR, (FLASH_CR_MER1 | FLASH_CR_MER2));
  }

  /* Proceed to erase all sectors */
  SET_BIT(FLASH->CR, FLASH_CR_STRT);
}

/**
  * @brief  Erase the specified FLASH memory page.
  * @param  Page FLASH page to erase.
  *         This parameter must be a value between 0 and (max number of pages in the bank - 1).
  * @param  Banks Bank where the page will be erased.
  *         This parameter can be one of the following values:
  *            @arg FLASH_BANK_1: Page in bank 1 to be erased
  *            @arg FLASH_BANK_2: Page in bank 2 to be erased
  * @retval None
  */
void FLASH_PageErase(uint32_t Page, uint32_t Banks)
{
  /* Check the parameters */
  assert_param(IS_FLASH_PAGE(Page));

  /* Clear all error flags */
  SET_BIT(FLASH->SR, FLASH_FLAG_SR_ERRORS);

  if (READ_BIT(FLASH->OPTR, FLASH_OPTR_DBANK) == 0U)
  {
    CLEAR_BIT(FLASH->CR, FLASH_CR_BKER);
  }
  else
  {
    assert_param(IS_FLASH_BANK_EXCLUSIVE(Banks));

    if ((Banks & FLASH_BANK_1) != 0U)
    {
      CLEAR_BIT(FLASH->CR, FLASH_CR_BKER);
    }
    else
    {
      SET_BIT(FLASH->CR, FLASH_CR_BKER);
    }
  }

  /* Proceed to erase the page */
  SET_BIT(FLASH->CR, FLASH_CR_PER);
  MODIFY_REG(FLASH->CR, FLASH_CR_PNB, ((Page & 0xFFU) << FLASH_CR_PNB_Pos));
  SET_BIT(FLASH->CR, FLASH_CR_STRT);

}

/**
  * @brief  Flush the instruction and data caches.
  * @retval None
  */
void HAL_FLASH_FlushCaches(void)
{
  FLASH_CacheTypeDef cache = HAL_pFlash.CacheToReactivate;

  /* Flush instruction cache  */
  if ((cache == FLASH_CACHE_ICACHE_ENABLED) ||
      (cache == FLASH_CACHE_ICACHE_DCACHE_ENABLED))
  {
    /* Disable instruction cache */
    __HAL_FLASH_INSTRUCTION_CACHE_DISABLE();
    /* Reset instruction cache */
    __HAL_FLASH_INSTRUCTION_CACHE_RESET();
    /* Enable instruction cache */
    __HAL_FLASH_INSTRUCTION_CACHE_ENABLE();
  }

  /* Flush data cache */
  if ((cache == FLASH_CACHE_DCACHE_ENABLED) ||
      (cache == FLASH_CACHE_ICACHE_DCACHE_ENABLED))
  {
    /* Reset data cache */
    __HAL_FLASH_DATA_CACHE_RESET();
    /* Enable data cache */
    __HAL_FLASH_DATA_CACHE_ENABLE();
  }

  /* Reset internal variable */
  HAL_pFlash.CacheToReactivate = FLASH_CACHE_DISABLED;
}

/**
  * @brief  Configure the write protection area into Option Bytes.
  * @note   When the memory read protection level is selected (RDP level = 1),
  *         it is not possible to program or erase Flash memory if the CPU debug
  *         features are connected (JTAG or single wire) or boot code is being
  *         executed from RAM or System flash, even if WRP is not activated.
  * @note   To configure any option bytes, the option lock bit OPTLOCK must be
  *         cleared with the call of HAL_FLASH_OB_Unlock() function.
  * @note   New option bytes configuration will be taken into account in two cases:
  *         - after an option bytes launch through the call of HAL_FLASH_OB_Launch()
  *         - after a power reset (BOR reset or exit from Standby/Shutdown modes)
  * @param  WRPArea specifies the area to be configured.
  *         This parameter can be one of the following values:
  *            @arg OB_WRPAREA_BANK1_AREAA: Flash Bank 1 Area A
  *            @arg OB_WRPAREA_BANK1_AREAB: Flash Bank 1 Area B
  *            @arg OB_WRPAREA_BANK2_AREAA: Flash Bank 2 Area A
  *            @arg OB_WRPAREA_BANK2_AREAB: Flash Bank 2 Area B
  * @param  WRPStartOffset specifies the start page of the write protected area.
  *         This parameter can be page number between 0 and (max number of pages in the bank - 1).
  * @param  WRDPEndOffset specifies the end page of the write protected area.
  *         This parameter can be page number between WRPStartOffset and (max number of pages in the bank - 1).
  * @retval HAL Status
  */
static HAL_StatusTypeDef FLASH_OB_WRPConfig(uint32_t WRPArea, uint32_t WRPStartOffset, uint32_t WRDPEndOffset)
{
  HAL_StatusTypeDef status;

  /* Check the parameters */
  assert_param(IS_OB_WRPAREA(WRPArea));
  assert_param(IS_FLASH_PAGE(WRPStartOffset));
  assert_param(IS_FLASH_PAGE(WRDPEndOffset));

  /* Wait for last operation to be completed */
  status = HAL_FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);

  if (status == HAL_OK)
  {
    /* Configure the write protected area */
    if (WRPArea == OB_WRPAREA_BANK1_AREAA)
    {
      FLASH->WRP1AR = ((WRDPEndOffset << FLASH_WRP1AR_WRP1A_END_Pos) | WRPStartOffset);
    }
    else if (WRPArea == OB_WRPAREA_BANK1_AREAB)
    {
      FLASH->WRP1BR = ((WRDPEndOffset << FLASH_WRP1BR_WRP1B_END_Pos) | WRPStartOffset);
    }
    else
    {
      /* Nothing to do */
    }

    /* Set OPTSTRT Bit */
    SET_BIT(FLASH->CR, FLASH_CR_OPTSTRT);

    /* Wait for last operation to be completed */
    status = HAL_FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);
  }

  return status;
}

/**
  * @brief  Set the read protection level into Option Bytes.
  * @note   To configure any option bytes, the option lock bit OPTLOCK must be
  *         cleared with the call of HAL_FLASH_OB_Unlock() function.
  * @note   New option bytes configuration will be taken into account in two cases:
  *         - after an option bytes launch through the call of HAL_FLASH_OB_Launch()
  *         - after a power reset (BOR reset or exit from Standby/Shutdown modes)
  * @note   !!! Warning : When enabling OB_RDP level 2 it's no more possible
  *         to go back to level 1 or 0 !!!
  * @param  RDPLevel specifies the read protection level.
  *         This parameter can be one of the following values:
  *            @arg OB_RDP_LEVEL_0: No protection
  *            @arg OB_RDP_LEVEL_1: Memory Read protection
  *            @arg OB_RDP_LEVEL_2: Full chip protection
  *
  * @retval HAL Status
  */
static HAL_StatusTypeDef FLASH_OB_RDPConfig(uint32_t RDPLevel)
{
  HAL_StatusTypeDef status;

  /* Check the parameters */
  assert_param(IS_OB_RDP_LEVEL(RDPLevel));

  /* Wait for last operation to be completed */
  status = HAL_FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);

  if (status == HAL_OK)
  {
    /* Configure the RDP level in the option bytes register */
    MODIFY_REG(FLASH->OPTR, FLASH_OPTR_RDP, RDPLevel);

    /* Set OPTSTRT Bit */
    SET_BIT(FLASH->CR, FLASH_CR_OPTSTRT);

    /* Wait for last operation to be completed */
    status = HAL_FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);
  }

  return status;
}

/**
  * @brief  Program the FLASH User Option Bytes.
  * @note   To configure any option bytes, the option lock bit OPTLOCK must be
  *         cleared with the call of HAL_FLASH_OB_Unlock() function.
  * @note   New option bytes configuration will be taken into account in two cases:
  *         - after an option bytes launch through the call of HAL_FLASH_OB_Launch()
  *         - after a power reset (BOR reset or exit from Standby/Shutdown modes)
  * @param  UserType The FLASH User Option Bytes to be modified.
  *         This parameter can be a combination of @ref FLASH_OB_USER_Type.
  * @param  UserConfig The selected User Option Bytes values:
  *         This parameter can be a combination of
  *         @ref FLASH_OB_USER_nRST_STOP,     @ref FLASH_OB_USER_nRST_STANDBY ,
  *         @ref FLASH_OB_USER_nRST_SHUTDOWN, @ref FLASH_OB_USER_IWDG_SW,
  *         @ref FLASH_OB_USER_IWDG_STOP,     @ref FLASH_OB_USER_IWDG_STANDBY,
  *         @ref FLASH_OB_USER_WWDG_SW,       @ref FLASH_OB_USER_WWDG_SW,
  *         @ref FLASH_OB_USER_BFB2,          @ref FLASH_OB_USER_nBOOT1,
  *         @ref FLASH_OB_USER_SRAM_PE,       @ref FLASH_OB_USER_CCMSRAM_RST,
  *         @ref FLASH_OB_USER_nSWBOOT0,      @ref FLASH_OB_USER_nBOOT0,
  * @note   (*) availability depends on devices
  * @retval HAL Status
  */
static HAL_StatusTypeDef FLASH_OB_UserConfig(uint32_t UserType, uint32_t UserConfig)
{
  uint32_t optr_reg_val = 0;
  uint32_t optr_reg_mask = 0;
  HAL_StatusTypeDef status;

  /* Check the parameters */
  assert_param(IS_OB_USER_TYPE(UserType));

  /* Wait for last operation to be completed */
  status = HAL_FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);

  if (status == HAL_OK)
  {
    if ((UserType & OB_USER_nRST_STOP) != 0U)
    {
      /* nRST_STOP option byte should be modified */
      assert_param(IS_OB_USER_STOP(UserConfig & FLASH_OPTR_nRST_STOP));

      /* Set value and mask for nRST_STOP option byte */
      optr_reg_val |= (UserConfig & FLASH_OPTR_nRST_STOP);
      optr_reg_mask |= FLASH_OPTR_nRST_STOP;
    }

    if ((UserType & OB_USER_nRST_STDBY) != 0U)
    {
      /* nRST_STDBY option byte should be modified */
      assert_param(IS_OB_USER_STANDBY(UserConfig & FLASH_OPTR_nRST_STDBY));

      /* Set value and mask for nRST_STDBY option byte */
      optr_reg_val |= (UserConfig & FLASH_OPTR_nRST_STDBY);
      optr_reg_mask |= FLASH_OPTR_nRST_STDBY;
    }
    if ((UserType & OB_USER_IWDG_SW) != 0U)
    {
      /* IWDG_SW option byte should be modified */
      assert_param(IS_OB_USER_IWDG(UserConfig & FLASH_OPTR_IWDG_SW));

      /* Set value and mask for IWDG_SW option byte */
      optr_reg_val |= (UserConfig & FLASH_OPTR_IWDG_SW);
      optr_reg_mask |= FLASH_OPTR_IWDG_SW;
    }

    if ((UserType & OB_USER_IWDG_STOP) != 0U)
    {
      /* IWDG_STOP option byte should be modified */
      assert_param(IS_OB_USER_IWDG_STOP(UserConfig & FLASH_OPTR_IWDG_STOP));

      /* Set value and mask for IWDG_STOP option byte */
      optr_reg_val |= (UserConfig & FLASH_OPTR_IWDG_STOP);
      optr_reg_mask |= FLASH_OPTR_IWDG_STOP;
    }

    if ((UserType & OB_USER_IWDG_STDBY) != 0U)
    {
      /* IWDG_STDBY option byte should be modified */
      assert_param(IS_OB_USER_IWDG_STDBY(UserConfig & FLASH_OPTR_IWDG_STDBY));

      /* Set value and mask for IWDG_STDBY option byte */
      optr_reg_val |= (UserConfig & FLASH_OPTR_IWDG_STDBY);
      optr_reg_mask |= FLASH_OPTR_IWDG_STDBY;
    }

    if ((UserType & OB_USER_WWDG_SW) != 0U)
    {
      /* WWDG_SW option byte should be modified */
      assert_param(IS_OB_USER_WWDG(UserConfig & FLASH_OPTR_WWDG_SW));

      /* Set value and mask for WWDG_SW option byte */
      optr_reg_val |= (UserConfig & FLASH_OPTR_WWDG_SW);
      optr_reg_mask |= FLASH_OPTR_WWDG_SW;
    }

    if ((UserType & OB_USER_BFB2) != 0U)
    {
      /* BFB2 option byte should be modified */
      assert_param(IS_OB_USER_BFB2(UserConfig & FLASH_OPTR_BFB2));

      /* Set value and mask for BFB2 option byte */
      optr_reg_val |= (UserConfig & FLASH_OPTR_BFB2);
      optr_reg_mask |= FLASH_OPTR_BFB2;
    }


    if ((UserType & OB_USER_nBOOT1) != 0U)
    {
      /* nBOOT1 option byte should be modified */
      assert_param(IS_OB_USER_BOOT1(UserConfig & FLASH_OPTR_nBOOT1));

      /* Set value and mask for nBOOT1 option byte */
      optr_reg_val |= (UserConfig & FLASH_OPTR_nBOOT1);
      optr_reg_mask |= FLASH_OPTR_nBOOT1;
    }

    if ((UserType & OB_USER_SRAM_PE) != 0U)
    {
      /* SRAM_PE option byte should be modified */
      assert_param(IS_OB_USER_SRAM_PARITY(UserConfig & FLASH_OPTR_SRAM_PE));

      /* Set value and mask for SRAM_PE option byte */
      optr_reg_val |= (UserConfig & FLASH_OPTR_SRAM_PE);
      optr_reg_mask |= FLASH_OPTR_SRAM_PE;
    }

    if ((UserType & OB_USER_CCMSRAM_RST) != 0U)
    {
      /* CCMSRAM_RST option byte should be modified */
      assert_param(IS_OB_USER_CCMSRAM_RST(UserConfig & FLASH_OPTR_CCMSRAM_RST));

      /* Set value and mask for CCMSRAM_RST option byte */
      optr_reg_val |= (UserConfig & FLASH_OPTR_CCMSRAM_RST);
      optr_reg_mask |= FLASH_OPTR_CCMSRAM_RST;
    }

    if ((UserType & OB_USER_nSWBOOT0) != 0U)
    {
      /* nSWBOOT0 option byte should be modified */
      assert_param(IS_OB_USER_SWBOOT0(UserConfig & FLASH_OPTR_nSWBOOT0));

      /* Set value and mask for nSWBOOT0 option byte */
      optr_reg_val |= (UserConfig & FLASH_OPTR_nSWBOOT0);
      optr_reg_mask |= FLASH_OPTR_nSWBOOT0;
    }

    if ((UserType & OB_USER_nBOOT0) != 0U)
    {
      /* nBOOT0 option byte should be modified */
      assert_param(IS_OB_USER_BOOT0(UserConfig & FLASH_OPTR_nBOOT0));

      /* Set value and mask for nBOOT0 option byte */
      optr_reg_val |= (UserConfig & FLASH_OPTR_nBOOT0);
      optr_reg_mask |= FLASH_OPTR_nBOOT0;
    }

    /* Configure the option bytes register */
    MODIFY_REG(FLASH->OPTR, optr_reg_mask, optr_reg_val);

    /* Set OPTSTRT Bit */
    SET_BIT(FLASH->CR, FLASH_CR_OPTSTRT);

    /* Wait for last operation to be completed */
    status = HAL_FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);
  }

  return status;
}

/**
  * @brief  Return the Write Protection configuration into Option Bytes.
  * @param[in]  WRPArea specifies the area to be returned.
  *          This parameter can be one of the following values:
  *            @arg OB_WRPAREA_BANK1_AREAA: Flash Bank 1 Area A
  *            @arg OB_WRPAREA_BANK1_AREAB: Flash Bank 1 Area B
  *            @arg OB_WRPAREA_BANK2_AREAA: Flash Bank 2 Area A
  *            @arg OB_WRPAREA_BANK2_AREAB: Flash Bank 2 Area B
  * @param[out]  WRPStartOffset specifies the address where to copied the start page
  *              of the write protected area.
  * @param[out]  WRDPEndOffset specifies the address where to copied the end page of
  *              the write protected area.
  * @retval None
  */
static void FLASH_OB_GetWRP(uint32_t WRPArea, uint32_t *WRPStartOffset, uint32_t *WRDPEndOffset)
{
  /* Get the configuration of the write protected area */
  if (WRPArea == OB_WRPAREA_BANK1_AREAA)
  {
    *WRPStartOffset = READ_BIT(FLASH->WRP1AR, FLASH_WRP1AR_WRP1A_STRT);
    *WRDPEndOffset = (READ_BIT(FLASH->WRP1AR, FLASH_WRP1AR_WRP1A_END) >> FLASH_WRP1AR_WRP1A_END_Pos);
  }
  else if (WRPArea == OB_WRPAREA_BANK1_AREAB)
  {
    *WRPStartOffset = READ_BIT(FLASH->WRP1BR, FLASH_WRP1BR_WRP1B_STRT);
    *WRDPEndOffset = (READ_BIT(FLASH->WRP1BR, FLASH_WRP1BR_WRP1B_END) >> FLASH_WRP1BR_WRP1B_END_Pos);
  }
  else
  {
    /* Nothing to do */
  }
}

/**
  * @brief  Return the FLASH Read Protection level into Option Bytes.
  * @retval RDP_Level
  *         This return value can be one of the following values:
  *            @arg OB_RDP_LEVEL_0: No protection
  *            @arg OB_RDP_LEVEL_1: Read protection of the memory
  *            @arg OB_RDP_LEVEL_2: Full chip protection
  */
static uint32_t FLASH_OB_GetRDP(void)
{
  uint32_t rdp_level = READ_BIT(FLASH->OPTR, FLASH_OPTR_RDP);

  if ((rdp_level != OB_RDP_LEVEL_0) && (rdp_level != OB_RDP_LEVEL_2))
  {
    return (OB_RDP_LEVEL_1);
  }
  else
  {
    return rdp_level;
  }
}

/**
  * @brief  Return the FLASH User Option Byte value.
  * @retval OB_user_config
  *         This return value is a combination of
  *         @ref FLASH_OB_USER_nRST_STOP,     @ref FLASH_OB_USER_nRST_STANDBY,
  *         @ref FLASH_OB_USER_nRST_SHUTDOWN, @ref FLASH_OB_USER_IWDG_SW,
  *         @ref FLASH_OB_USER_IWDG_STOP,     @ref FLASH_OB_USER_IWDG_STANDBY,
  *         @ref FLASH_OB_USER_WWDG_SW,       @ref FLASH_OB_USER_WWDG_SW,
  *         @ref FLASH_OB_USER_BFB2,          @ref FLASH_OB_USER_DBANK,
  *         @ref FLASH_OB_USER_nBOOT1,        @ref FLASH_OB_USER_SRAM_PE,
  *         @ref FLASH_OB_USER_CCMSRAM_RST,   @ref OB_USER_nSWBOOT0,
  *         @ref FLASH_OB_USER_NRST_MODE,     @ref FLASH_OB_USER_nBOOT0
  * @note  (*) availability depends on devices
  */
static uint32_t FLASH_OB_GetUser(void)
{
  uint32_t user_config = READ_REG(FLASH->OPTR);
  CLEAR_BIT(user_config, FLASH_OPTR_RDP);

  return user_config;
}

/**
  * end of FLASH_Function_Definitions @}
  */

/**
  * end of FLASH @}
  */

#endif /* HAL_FLASH_MODULE_ENABLED */


