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
*   File    : rx32g4xx_hal_flash.c
*   By      : RX_DV_Team
**************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal.h"

/** @addtogroup FLASH FLASH
  * @{
  */

#ifdef HAL_FLASH_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/** @defgroup FLASH_Private_Constants FLASH Private Constants
  * @{
  */
#define FLASH_NB_DOUBLE_WORDS_IN_ROW  32

/**
  * end of FLASH_Private_Constants @}
  */

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/** @defgroup FLASH_Private_Variables FLASH Private Variables
  * @{
  */

/**
  * @brief  Variable used for Program/Erase sectors under interruption
  */
HAL_FLASH_ProcessTypeDef HAL_pFlash = {.Lock = HAL_UNLOCKED,
                                       .ErrorCode = HAL_FLASH_ERROR_NONE,
                                       .ProcedureOnGoing = FLASH_PROC_NONE,
                                       .Address = 0U,
                                       .Bank = FLASH_BANK_1,
                                       .Page = 0U,
                                       .NbPagesToErase = 0U,
                                       .CacheToReactivate = FLASH_CACHE_DISABLED};
/**
  * end of FLASH_Private_Variables @}
  */

/* Private function prototypes -----------------------------------------------*/

/** @defgroup FLASH_Private_Functions FLASH Private Functions
  * @{
  */

static void FLASH_Program_DoubleWord(uint32_t Address, uint32_t* Buf);
static void FLASH_Program_DoubleDoubleWord(uint32_t Address, uint32_t* Buf);


/**
  * end of FLASH_Private_Functions @}
  */

/* Exported functions --------------------------------------------------------*/
/** @defgroup  FLASH_Function_Definitions FLASH Function Definitions
  * @{
  */

/**
  * @brief  Program double word or fast program of a row at a specified address.
  * @param  TypeProgram Indicate the way to program at a specified address.
  *         This parameter can be a value of @ref FLASH_Type_Program.
  * @param  Address specifies the address to be programmed.
  * @param  Data specifies the data to be programmed.
  *         This parameter is the data for the double word program and the address where
  *         are stored the data for the row fast program.
  *
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_FLASH_Program(uint32_t TypeProgram, uint32_t Address, uint32_t* Data)
{
  HAL_StatusTypeDef status;
  uint32_t prog_bit = 0;

  /* Check the parameters */
  assert_param(IS_FLASH_TYPEPROGRAM(TypeProgram));

  /* Process Locked */
  __HAL_LOCK(&HAL_pFlash);

  /* Wait for last operation to be completed */
  status = HAL_FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);

  if (status == HAL_OK)
  {
    HAL_pFlash.ErrorCode = HAL_FLASH_ERROR_NONE;

    /* Deactivate the data cache if they are activated to avoid data misbehavior */
    if(READ_BIT(FLASH->ACR, FLASH_ACR_DCEN) != 0U)
    {
      /* Disable data cache  */
      __HAL_FLASH_DATA_CACHE_DISABLE();
      HAL_pFlash.CacheToReactivate = FLASH_CACHE_DCACHE_ENABLED;
    }
    else
    {
      HAL_pFlash.CacheToReactivate = FLASH_CACHE_DISABLED;
    }
    if (TypeProgram == FLASH_TYPEPROGRAM_DOUBLEWORD)
    {
      /* Program double-word (64-bit) at a specified address */
      FLASH_Program_DoubleWord(Address, Data);
      prog_bit = FLASH_CR_PG;
    }

    else if (TypeProgram == FLASH_TYPEPROGRAM_DOUBLEDOUBLEWORD)
    {
      /* Program double-double-word (128-bit) at a specified address */
      FLASH_Program_DoubleDoubleWord(Address, Data);
    }

    else
    {
      /* Nothing to do */
    }

    /* Wait for last operation to be completed */
    status = HAL_FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);

    /* If the program operation is completed, disable the PG or FSTPG Bit */
    if (prog_bit != 0U)
    {
      CLEAR_BIT(FLASH->CR, prog_bit);
    }

    /* Flush the caches to be sure of the data consistency */
    HAL_FLASH_FlushCaches();
  }

  /* Process Unlocked */
  __HAL_UNLOCK(&HAL_pFlash);

  /* return status */
  return status;
}

/**
  * @brief  Program double word or fast program of a row at a specified address with interrupt enabled.
  * @param  TypeProgram Indicate the way to program at a specified address.
  *         This parameter can be a value of @ref FLASH_Type_Program.
  * @param  Address specifies the address to be programmed.
  * @param  Data specifies the data to be programmed.
  *         This parameter is the data for the double word program and the address where
  *         are stored the data for the row fast program.
  *
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_FLASH_Program_IT(uint32_t TypeProgram, uint32_t Address, uint32_t* Data)
{
  HAL_StatusTypeDef status;

  /* Check the parameters */
  assert_param(IS_FLASH_TYPEPROGRAM(TypeProgram));

  /* Process Locked */
  __HAL_LOCK(&HAL_pFlash);

  /* Reset error code */
  HAL_pFlash.ErrorCode = HAL_FLASH_ERROR_NONE;

  /* Deactivate the data cache if they are activated to avoid data misbehavior */
  if(READ_BIT(FLASH->ACR, FLASH_ACR_DCEN) != 0U)
  {
    /* Disable data cache  */
    __HAL_FLASH_DATA_CACHE_DISABLE();
    HAL_pFlash.CacheToReactivate = FLASH_CACHE_DCACHE_ENABLED;
  }
  else
  {
    HAL_pFlash.CacheToReactivate = FLASH_CACHE_DISABLED;
  }

  /* Wait for last operation to be completed */
  status = HAL_FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE);

  if (status != HAL_OK)
  {
    /* Process Unlocked */
    __HAL_UNLOCK(&HAL_pFlash);
  }
  else
  {
    /* Set internal variables used by the IRQ handler */
    if (TypeProgram == FLASH_TYPEPROGRAM_FAST_AND_LAST)
    {
      HAL_pFlash.ProcedureOnGoing = FLASH_PROC_PROGRAM_LAST;
    }
    else
    {
      HAL_pFlash.ProcedureOnGoing = FLASH_PROC_PROGRAM;
    }
    HAL_pFlash.Address = Address;

    /* Enable End of Operation and Error interrupts */
    __HAL_FLASH_ENABLE_IT(FLASH_IT_EOP | FLASH_IT_OPERR);

    if (TypeProgram == FLASH_TYPEPROGRAM_DOUBLEWORD)
    {
      /* Program double-word (64-bit) at a specified address */
      FLASH_Program_DoubleWord(Address, Data);
    }

    else if (TypeProgram == FLASH_TYPEPROGRAM_DOUBLEDOUBLEWORD)
    {
      /* Program double-double-word (128-bit) at a specified address */
      FLASH_Program_DoubleDoubleWord(Address, Data);
    }
    else
    {
      /* Nothing to do */
    }
  }

  return status;
}

/**
  * @brief  Handle FLASH interrupt request.
  * @retval None
  */
void HAL_FLASH_IRQHandler(void)
{
  uint32_t tmp_page;
  uint32_t error;
  FLASH_ProcedureTypeDef procedure;

  /* If the operation is completed, disable the PG, PNB, MER1, MER2 and PER Bit */
  CLEAR_BIT(FLASH->CR, (FLASH_CR_PG | FLASH_CR_MER1 | FLASH_CR_PER | FLASH_CR_PNB));
  CLEAR_BIT(FLASH->CR, FLASH_CR_MER2);
    
  /* Check FLASH operation error flags */
  error = (FLASH->SR & FLASH_FLAG_SR_ERRORS);

  if (error != 0U)
  {
    /* Save the error code */
    HAL_pFlash.ErrorCode |= error;

    /* Clear error programming flags */
    __HAL_FLASH_CLEAR_FLAG(error);

    /* Flush the caches to be sure of the data consistency */
    HAL_FLASH_FlushCaches();

    /* FLASH error interrupt user callback */
    procedure = HAL_pFlash.ProcedureOnGoing;
    if (procedure == FLASH_PROC_PAGE_ERASE)
    {
      HAL_FLASH_OperationErrorCallback(HAL_pFlash.Page);
    }
    else if (procedure == FLASH_PROC_MASS_ERASE)
    {
      HAL_FLASH_OperationErrorCallback(HAL_pFlash.Bank);
    }
    else if ((procedure == FLASH_PROC_PROGRAM) ||
             (procedure == FLASH_PROC_PROGRAM_LAST))
    {
      HAL_FLASH_OperationErrorCallback(HAL_pFlash.Address);
    }
    else
    {
      /* Nothing to do */
    }

    /*Stop the procedure ongoing*/
    HAL_pFlash.ProcedureOnGoing = FLASH_PROC_NONE;
  }

  /* Check FLASH End of Operation flag  */
  if (__HAL_FLASH_GET_FLAG(FLASH_FLAG_EOP))
  {
    /* Clear FLASH End of Operation pending bit */
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP);

    if (HAL_pFlash.ProcedureOnGoing == FLASH_PROC_PAGE_ERASE)
    {
      /* Nb of pages to erased can be decreased */
      HAL_pFlash.NbPagesToErase--;

      /* Check if there are still pages to erase*/
      if (HAL_pFlash.NbPagesToErase != 0U)
      {
        /* Indicate user which page has been erased*/
        HAL_FLASH_EndOfOperationCallback(HAL_pFlash.Page);

        /* Increment page number */
        HAL_pFlash.Page++;
        tmp_page = HAL_pFlash.Page;
        FLASH_PageErase(tmp_page, HAL_pFlash.Bank);
      }
      else
      {
        /* No more pages to Erase */
        /* Reset Address and stop Erase pages procedure */
        HAL_pFlash.Page = 0xFFFFFFFFU;
        HAL_pFlash.ProcedureOnGoing = FLASH_PROC_NONE;

        /* Flush the caches to be sure of the data consistency */
        HAL_FLASH_FlushCaches();

        /* FLASH EOP interrupt user callback */
        HAL_FLASH_EndOfOperationCallback(HAL_pFlash.Page);
      }
    }
    else
    {
      /* Flush the caches to be sure of the data consistency */
      HAL_FLASH_FlushCaches();

      procedure = HAL_pFlash.ProcedureOnGoing;
      if (procedure == FLASH_PROC_MASS_ERASE)
      {
        /* MassErase ended. Return the selected bank */
        /* FLASH EOP interrupt user callback */
        HAL_FLASH_EndOfOperationCallback(HAL_pFlash.Bank);
      }
      else if ((procedure == FLASH_PROC_PROGRAM) ||
               (procedure == FLASH_PROC_PROGRAM_LAST))
      {
        /* Program ended. Return the selected address */
        /* FLASH EOP interrupt user callback */
        HAL_FLASH_EndOfOperationCallback(HAL_pFlash.Address);
      }
      else
      {
        /* Nothing to do */
      }

      /*Clear the procedure ongoing*/
      HAL_pFlash.ProcedureOnGoing = FLASH_PROC_NONE;
    }
  }

  if (HAL_pFlash.ProcedureOnGoing == FLASH_PROC_NONE)
  {
    /* Disable End of Operation and Error interrupts */
    __HAL_FLASH_DISABLE_IT(FLASH_IT_EOP | FLASH_IT_OPERR);

    /* Process Unlocked */
    __HAL_UNLOCK(&HAL_pFlash);
  }
}

/**
  * @brief  FLASH end of operation interrupt callback.
  * @param  ReturnValue The value saved in this parameter depends on the ongoing procedure:
  *           @arg Mass Erase: Bank number which has been requested to erase
  *           @arg Page Erase: Page which has been erased
  *                            (if 0xFFFFFFFF, it means that all the selected pages have been erased)
  *           @arg Program: Address which was selected for data program
  * @retval None
  */
__weak void HAL_FLASH_EndOfOperationCallback(uint32_t ReturnValue)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(ReturnValue);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_FLASH_EndOfOperationCallback could be implemented in the user file
   */
}

/**
  * @brief  FLASH operation error interrupt callback.
  * @param  ReturnValue The value saved in this parameter depends on the ongoing procedure:
  *           @arg Mass Erase: Bank number which has been requested to erase
  *           @arg Page Erase: Page number which returned an error
  *           @arg Program: Address which was selected for data program
  * @retval None
  */
__weak void HAL_FLASH_OperationErrorCallback(uint32_t ReturnValue)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(ReturnValue);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_FLASH_OperationErrorCallback could be implemented in the user file
   */
}

/**
  * @brief  Unlock the FLASH control register access.
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_FLASH_Unlock(void)
{
  HAL_StatusTypeDef status = HAL_OK;

  if (READ_BIT(FLASH->CR, FLASH_CR_LOCK) != 0U)
  {
    /* Authorize the FLASH Registers access */
    WRITE_REG(FLASH->KEYR, FLASH_KEY1);
    WRITE_REG(FLASH->KEYR, FLASH_KEY2);

    /* verify Flash is unlocked */
    if (READ_BIT(FLASH->CR, FLASH_CR_LOCK) != 0U)
    {
      status = HAL_ERROR;
    }
  }

  return status;
}

/**
  * @brief  Lock the FLASH control register access.
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_FLASH_Lock(void)
{
  HAL_StatusTypeDef status = HAL_ERROR;

  /* Set the LOCK Bit to lock the FLASH Registers access */
  SET_BIT(FLASH->CR, FLASH_CR_LOCK);

  /* verify Flash is locked */
  if (READ_BIT(FLASH->CR, FLASH_CR_LOCK) != 0U)
  {
    status = HAL_OK;
  }

  return status;
}

/**
  * @brief  Unlock the FLASH Option Bytes Registers access.
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_FLASH_OB_Unlock(void)
{
  HAL_StatusTypeDef status = HAL_OK;

  if (READ_BIT(FLASH->CR, FLASH_CR_OPTLOCK) != 0U)
  {
    /* Authorizes the Option Byte register programming */
    WRITE_REG(FLASH->OPTKEYR, FLASH_OPTKEY1);
    WRITE_REG(FLASH->OPTKEYR, FLASH_OPTKEY2);

    /* verify option bytes are unlocked */
    if (READ_BIT(FLASH->CR, FLASH_CR_OPTLOCK) != 0U)
    {
      status = HAL_ERROR;
    }
  }

  return status;
}

/**
  * @brief  Lock the FLASH Option Bytes Registers access.
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_FLASH_OB_Lock(void)
{
  HAL_StatusTypeDef status = HAL_ERROR;

  /* Set the OPTLOCK Bit to lock the FLASH Option Byte Registers access */
  SET_BIT(FLASH->CR, FLASH_CR_OPTLOCK);

  /* Verify option bytes are locked */
  if (READ_BIT(FLASH->CR, FLASH_CR_OPTLOCK) != 0U)
  {
    status = HAL_OK;
  }

  return status;
}

/**
  * @brief  Launch the option byte loading.
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_FLASH_OB_Launch(void)
{
  /* Set the bit to force the option byte reloading */
  SET_BIT(FLASH->CR, FLASH_CR_OBL_LAUNCH);

  /* Wait for last operation to be completed */
  return (HAL_FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE));
}

/**
  * @brief  Get the specific FLASH error flag.
  * @retval FLASH_ErrorCode. The returned value can be:
  *            @arg HAL_FLASH_ERROR_NONE: No error set
  *            @arg HAL_FLASH_ERROR_OP: FLASH Operation error
  *            @arg HAL_FLASH_ERROR_PROG: FLASH Programming error
  *            @arg HAL_FLASH_ERROR_WRP: FLASH Write protection error
  *            @arg HAL_FLASH_ERROR_PGA: FLASH Programming alignment error
  *            @arg HAL_FLASH_ERROR_SIZ: FLASH Size error
  *            @arg HAL_FLASH_ERROR_PGS: FLASH Programming sequence error
  *            @arg HAL_FLASH_ERROR_RD: FLASH Read Protection error flag (PCROP)
  *            @arg HAL_FLASH_ERROR_OPTV: FLASH Option validity error
  *            @arg HAL_FLASH_ERROR_OPTV_RX: FLASH Option validity error(autoload)
  *            @arg HAL_FLASH_ERROR_ECCC: ECC error
  *            @arg HAL_FLASH_ERROR_ECCD: ECC error
  *            @arg HAL_FLASH_ERROR_ECCC2: ECC error
  *            @arg HAL_FLASH_ERROR_ECCD2: ECC error
  */
uint32_t HAL_FLASH_GetError(void)
{
  return HAL_pFlash.ErrorCode;
}

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Wait for a FLASH operation to complete.
  * @param  Timeout maximum flash operation timeout.
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_FLASH_WaitForLastOperation(uint32_t Timeout)
{
  /* Wait for the FLASH operation to complete by polling on BUSY flag to be reset.
     Even if the FLASH operation fails, the BUSY flag will be reset and an error
     flag will be set */

  uint32_t tickstart = HAL_GetTick();
  uint32_t error;

  while (__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY))
  {
    if ((HAL_GetTick() - tickstart) > Timeout)
    {
      return HAL_TIMEOUT;
    }
  }

  /* Check FLASH operation error flags */
  error = (FLASH->SR & FLASH_FLAG_SR_ERRORS);
  if (error != 0u)
  {
    /* Save the error code */
    HAL_pFlash.ErrorCode |= error;

    /* Clear error programming flags */
    __HAL_FLASH_CLEAR_FLAG(error);

    return HAL_ERROR;
  }

  /* Check FLASH End of Operation flag  */
  if (__HAL_FLASH_GET_FLAG(FLASH_FLAG_EOP))
  {
    /* Clear FLASH End of Operation pending bit */
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP);
  }

  /* If there is an error flag set */
  return HAL_OK;
}

/**
  * @brief  Program double-word (64-bit) at a specified address.
  * @param  Address specifies the address to be programmed.
  * @param  Buf specifies the data buffer to be programmed.
  * @retval None
  */
static void FLASH_Program_DoubleWord(uint32_t Address, uint32_t* Buf)
{
    
  uint32_t D0, D1;
    
  /* Check the parameters */
  assert_param(IS_FLASH_PROGRAM_ADDRESS(Address));

  /* Set PG bit */
  SET_BIT(FLASH->CR, FLASH_CR_PG);

  D0 = *((uint32_t *)(Buf + 0 ));                   /* The first  4-bytes data*/
  D1 = *((uint32_t *)(Buf + 1 ));                   /* The second 4-bytes data*/

  *(uint32_t *)(Address     ) = D0;                 /* Program first  4-bytes word */
  *(uint32_t *)(Address + 4 ) = D1;                 /* Program second 4-bytes word */

}


/**
  * @brief  Program double-double-word (128-bit) at a specified address.
  * @param  Address specifies the address to be programmed.
  * @param  Buf specifies the data buffer to be programmed.
  * @retval None
  */
static void FLASH_Program_DoubleDoubleWord(uint32_t Address, uint32_t* Buf)
{
  uint32_t D0, D1, D2, D3;
    
  /* Check the parameters */
  assert_param(IS_FLASH_PROGRAM_ADDRESS(Address));

  /* Set PG bit */
  SET_BIT(FLASH->CR, FLASH_CR_PG);

  D0 = *((uint32_t *)(Buf + 0 ));                   /* The first  4-bytes data*/
  D1 = *((uint32_t *)(Buf + 1 ));                   /* The second 4-bytes data*/
  D2 = *((uint32_t *)(Buf + 2 ));                   /* The third  4-bytes data*/
  D3 = *((uint32_t *)(Buf + 3 ));                   /* The fourth 4-bytes data*/

  *(uint32_t *)(Address     ) = D0;                 /* Program first  4-bytes word */
  *(uint32_t *)(Address + 4 ) = D1;                 /* Program second 4-bytes word */
  *(uint32_t *)(Address + 8 ) = D2;                 /* Program third  4-bytes word */
  *(uint32_t *)(Address + 12) = D3;                 /* Program fourth 4-bytes word */
}

/**
  * @brief  Programs the FLASH User Option Byte: IWDG_SW / RST_STOP / RST_STDBY / IWDG_STDBY / IWDG_STOP / WWDG_SW / ECC / Nboot1 / nSWBOOT0 / nBOOT0.
  * @param  OB_USER:
  *     @arg FLASH_OPTR_nRST_STOP
  *     @arg FLASH_OPTR_nRST_STDBY
  *     @arg FLASH_OPTR_IWDG_SW
  *     @arg FLASH_OPTR_IWDG_STOP
  *     @arg FLASH_OPTR_IWDG_STDBY
  *     @arg FLASH_OPTR_WWDG_SW
  *     @arg FLASH_OPTR_BFB2
  *     @arg FLASH_OPTR_ECC
  *     @arg FLASH_OPTR_Nboot1
  *     @arg FLASH_OPTR_nSWBOOT0
  *     @arg FLASH_OPTR_nBOOT0
  * @retval None.
  */
void FLASH_Set_UserOptionByte(uint32_t OB_USER)
{
  if(OB_USER != 0)
  {
    // Unlock FLASH to acees register
    HAL_FLASH_Unlock();

    HAL_FLASH_OB_Unlock();

    SET_BIT(FLASH->OPTR, OB_USER);

    SET_BIT(FLASH->CR, FLASH_CR_OPTSTRT);

    while((FLASH->SR & FLASH_SR_BSY) == FLASH_SR_BSY)
    {
    };

    HAL_FLASH_OB_Lock();
    HAL_FLASH_Lock();
  }
}

/**
  * @brief  Clear the FLASH User Option Byte:IWDG_SW / RST_STOP / RST_STDBY / IWDG_STDBY / IWDG_STOP / WWDG_SW / ECC / Nboot1 / nSWBOOT0 / nBOOT0.
  * @param  OB_USER:
  *     @arg FLASH_OPTR_nRST_STOP
  *     @arg FLASH_OPTR_nRST_STDBY
  *     @arg FLASH_OPTR_IWDG_SW
  *     @arg FLASH_OPTR_IWDG_STOP
  *     @arg FLASH_OPTR_IWDG_STDBY
  *     @arg FLASH_OPTR_WWDG_SW
  *     @arg FLASH_OPTR_BFB2
  *     @arg FLASH_OPTR_ECC
  *     @arg FLASH_OPTR_Nboot1
  *     @arg FLASH_OPTR_nSWBOOT0
  *     @arg FLASH_OPTR_nBOOT0
  * @retval None.
    */
void FLASH_Clear_UserOptionByte(uint32_t OB_USER)
{ 
  if(OB_USER != 0)
  {
    // Unlock FLASH to acees register
    HAL_FLASH_Unlock();

    HAL_FLASH_OB_Unlock();

    CLEAR_BIT(FLASH->OPTR, OB_USER);

    SET_BIT(FLASH->CR,FLASH_CR_OPTSTRT);

    while((FLASH->SR & FLASH_SR_BSY) == FLASH_SR_BSY)
    {
    }; 

    HAL_FLASH_OB_Lock();
    HAL_FLASH_Lock();
  } 
}


/**
  * end of FLASH_Function_Definitions @}
  */

#endif /* HAL_FLASH_MODULE_ENABLED */

/**
  * end of FLASH @}
  */
