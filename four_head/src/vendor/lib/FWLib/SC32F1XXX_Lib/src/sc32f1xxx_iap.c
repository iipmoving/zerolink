/**
 ******************************************************************************
 * @file    sc32f1xxx_iap.c
 * @author  SOC AE Team
 * @version V1.9-BetaV6
 * @date    2026-04-03
 * @brief   IAP function module
 ******************************************************************************
 * @attention
 *
 *1.This software is supplied by SinOne Microelectronics Co.,Ltd. and is only 
 *intended for use with SinOne products. No other uses are authorized. This 
 *software is owned by SinOne Microelectronics Co.,Ltd. and is protected under 
 *all applicable laws, including copyright laws. 
 *2.The software which is for guidance only aims at providing customers with 
 *coding information regarding their products in order for them to save time. 
 *As a result, SinOne shall not be held liable for any direct, indirect or 
 *consequential damages with respect to any claims arising from the content of 
 *such software and/or the use made by customers of the coding information 
 *contained herein in connection with their products.
 *
 *  COPYRIGHT 2026 SinOne Microelectronics
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "sc32f1xxx_iap.h"

/* Function declaration: Programs FLASH memory from within RAM */
static boolType IAP_Program_InRam(uint32_t Address, uint32_t Data,IAPWRITE_ALIGNTYPE Type);
/* Function declaration: Executes a No Operation (NOP) instruction */
static boolType IAP_NOP(void);
/* Function pointer type definition: Points to a function that performs IAP operations in RAM */
typedef boolType (*pInitIapInRam)(uint32_t Address, uint32_t Data,IAPWRITE_ALIGNTYPE Type);

/* Static memory buffer: Used to store IAP programming functions in RAM */
__ALIGNED(4) static uint8_t IapProgramInRam[180]={0};
/* Initialized to the IAP_NOP function as a placeholder or default operation */
static pInitIapInRam pWriteIapFun=(pInitIapInRam)IAP_NOP;

/* Global variable: Software lock for IAP operations, Set to IAP_ENABLE to unlock IAP operations */
uint32_t IapWriteFlag = IAP_DISABLE;

/** @defgroup IAP_Group1 FLASH Memory Programming functions
 *  @brief   FLASH Memory Programming functions
 *
@verbatim
 ===============================================================================
                     ##### FLASH Memory Programming functions #####
 ===============================================================================
@endverbatim
  * @{
  */

/**
 * @brief  Copy the code data of the function IAP_Program_InRam from ROM to RAM so that the function can be executed in RAM.
 * @param  None
 * @retval None
 */
static void InitFlashToRam(void)
{
		for(uint16_t i=0;i<sizeof(IapProgramInRam);i++)
		{
				IapProgramInRam[i]    =*(uint8_t*)((uint32_t)IAP_Program_InRam-1+i);
		}		
		pWriteIapFun=(pInitIapInRam)((uint32_t)IapProgramInRam|0x01);
}

/**
 * @brief  Reset the space reserved in RAM for the function "IAP_Program_InRam".
 * @param  None
 * @retval None
 */
static void DeInitFlashToRam(void)
{
		for(uint16_t i=0;i<sizeof(IapProgramInRam);i++)
		{
				IapProgramInRam[i]    =0;
		}		
		pWriteIapFun=(pInitIapInRam)IAP_NOP;		
}

/**
 * @brief  A no-operation (NOP) function with a return value.
 * @param  None
 * @retval The boolean value FALSE.
 */
static boolType IAP_NOP(void)
{
		__NOP();
		return FALSE;
}

/**
 * @brief  Program a byte, half-word, or word at a specified address.
 * @param  Address[in]: The target address, i.e., the address where the data will be written.
 * @param  Data[in]: The data to be written.
 * @param  Type[in]: The type of data to be written, which can be byte (IAP_BYTE), half-word (IAP_HALFWORD), or word (IAP_WORD).
 *                  - IAP_BYTE : Represents a single byte of data.
 *                  - IAP_HALFWORD : Represents a half-word (typically 16 bits or 2 bytes) of data.
 *                  - IAP_WORD : Represents a word (typically 32 bits or 4 bytes) of data.
 * @retval boolType: The returned value can be: TRUE or FALSE.
 *                  - FALSE
 *                  - TRUE
 */  
static boolType IAP_Program_InRam(uint32_t Address, uint32_t Data,IAPWRITE_ALIGNTYPE Type)
{
		boolType Flag;
		/* Disable non-maskable interrupts */
		uint32_t	Temp=RCC->NMI_CFG;
		Temp =(Temp&0x0000FFFF)|0xA05F0000;
		RCC->NMI_CFG =Temp&0xFFFF0000;
		/* Disable interrupt requests (IRQ) */
		__disable_irq();
		
		/* Enable writing if the IAP software lock is open */
    /* IAP_WriteCmd(ENABLE); */
		if(IapWriteFlag==IAP_ENABLE)
		{
				IAP->IAP_CON |= ( uint32_t ) IAP_CON_PRG;
		}
		if(IapWriteFlag==IAP_ENABLE)
		{
				switch(Type)
				{
						/* Program a byte (8-bit) at a specified address */
						case IAP_BYTE:
						{
								* ( uint8_t * ) Address = (uint8_t)Data;
								* ( uint8_t * ) Address = (uint8_t)Data;
								if ( (uint8_t)Data == * ( __IO uint8_t * ) Address )
										Flag= TRUE;
								else
										Flag= FALSE;
								break;
						}
						/* Program a halfword (16-bit) at a specified address */
						case IAP_HALFWORD:
						{
								* ( uint16_t * ) Address = (uint16_t)Data;
								* ( uint16_t * ) Address = (uint16_t)Data;
								if ( (uint16_t)Data == * ( __IO uint16_t * ) Address )
										Flag= TRUE;
								else
										Flag= FALSE;						
								break;
						}			
						/* Program a word (32-bit) at a specified address */
						case IAP_WORD:
						{
								* ( uint32_t * ) Address = (uint32_t)Data;
								* ( uint32_t * ) Address = (uint32_t)Data;
								if ( (uint32_t)Data == * ( __IO uint32_t * ) Address )
										Flag= TRUE;
								else
										Flag= FALSE;						
								break;
						}
				}
		}
		/* IAP_WriteCmd(DISABLE); */
    IAP->IAP_CON &= ( uint32_t ) ~IAP_CON_PRG;		
		/* Enable interrupt requests (IRQ) */
    __enable_irq();
		/* Restore the previous non-maskable interrupt (NMI) settings */
		RCC->NMI_CFG =Temp; 
		return Flag;
}	

/**
 * @brief  Program a data array (in bytes, half-word or word) at a specified address.
 * @param  Address[out]: The target address, which is the starting address where the data will be written.
 * @param  ByteArray[out]: A pointer to the array of data to be written.
 * @param  ArraySize[in]: The size of the data array to be written (in bytes, half-word or word).
 * @param  Type[in]: The type of data to be written, which can be byte (IAP_BYTE), half-word (IAP_HALFWORD), or word (IAP_WORD).
 *                  - IAP_BYTE : Represents a single byte of data.
 *                  - IAP_HALFWORD : Represents a half-word (typically 16 bits or 2 bytes) of data.
 *                  - IAP_WORD : Represents a word (typically 32 bits or 4 bytes) of data.
 * @retval Uint16_t, which typically represents the status or result of the operation (e.g., the number of bytes written or an error code).
 */  
static uint16_t IAP_ProgramArray_InRam( uint32_t Address, uint8_t* ByteArray, uint16_t ArraySize,IAPWRITE_ALIGNTYPE Type )
{
		uint16_t tmpCnt = 0;
		/* Disable non-maskable interrupts */
		uint32_t	Temp=RCC->NMI_CFG;
		Temp =(Temp&0x0000FFFF)|0xA05F0000;
		RCC->NMI_CFG =Temp&0xFFFF0000;
		/* Disable interrupt requests (IRQ) */
		__disable_irq();
	  
		/* Enable writing if the IAP software lock is open */
    /* IAP_WriteCmd(ENABLE); */
		if(IapWriteFlag==IAP_ENABLE)
		{
				IAP->IAP_CON |= ( uint32_t ) IAP_CON_PRG;
		}
		if(IapWriteFlag==IAP_ENABLE)
		{
				do
				{			
						switch(Type)
						{
								/* Program byte (8-bit) Array at a specified address */
								case IAP_BYTE:
								{
										* ( __IO uint8_t* ) Address = ByteArray[tmpCnt];
										* ( __IO uint8_t* ) Address = ByteArray[tmpCnt];
										if ( ByteArray[tmpCnt] != * ( __IO uint8_t * ) Address )
										{
												break;
										}
										Address = Address + 1;
										break;
								}
								/* Program halfword (16-bit) Array at a specified address */
								case IAP_HALFWORD:
								{
										uint16_t* ByteArray16=(uint16_t*)ByteArray;
										if ( ( Address & 0x01 ) == 0 )
										{
												* ( __IO uint16_t* ) Address = ByteArray16[tmpCnt];
												* ( __IO uint16_t* ) Address = ByteArray16[tmpCnt];
												if ( ByteArray16[tmpCnt] != * ( __IO uint16_t * ) Address )
												{
														break;
												}
												Address = Address + 2;
										}
										break;
								}
								/* Program word (32-bit) Array at a specified address */
								case IAP_WORD:
								{
										uint32_t* ByteArray32=(uint32_t*)ByteArray;;
										if ( ( Address & 0x03 ) == 0 )
										{
												* ( __IO uint32_t* ) Address = (uint32_t)ByteArray32[tmpCnt];
												* ( __IO uint32_t* ) Address = (uint32_t)ByteArray32[tmpCnt];							
												if ( ByteArray32[tmpCnt] != * ( __IO uint32_t * ) Address )
												{
														break;			
												}									
												Address = Address + 4;
										}	
										break;					
								}
						}
				}while ( ++tmpCnt < ArraySize );
		}
		/* IAP_WriteCmd(DISABLE); */
    IAP->IAP_CON &= ( uint32_t ) ~IAP_CON_PRG;		
		/* Enable interrupt requests (IRQ) */
    __enable_irq();
		/* Restore the previous non-maskable interrupt (NMI) settings */
		RCC->NMI_CFG =Temp;
		return  tmpCnt;
}

/**
  * @brief  Unlocks the FLASH control register access
  * @param  None
  * @retval boolType: The returned value can be: TRUE or FALSE.
	*                 - FALSE
	*                 - TRUE
  */
boolType IAP_Unlock ( void )
{
		/* Check if software lock is open (IAP write operations are permitted) */
		if(IapWriteFlag==IAP_ENABLE)
		{
#if defined(SC32f10xx)
				/* HIRC must be enabled before IAP operation */
				RCC_Unlock ( 0xFF );
				RCC_HIRCCmd ( ENABLE );
#endif
				/* Authorize the FLASH Registers access */
				IAP->IAPKEY = IAP_KEY1;
				IAP->IAPKEY = IAP_KEY2;
	
				InitFlashToRam();
		}
		if ( IAP->IAPKEY == 0x01 )
				/* Unlock success */
				return TRUE;
		else
				/* Unlock Error */
				return FALSE;
}

/**
  * @brief  Recovers the FLASH control register access
  * @param  None
  * @retval None
  */
void IAP_Lock ( void )
{
    /* Set the LOCK Bit to lock the FLASH Registers access */
    IAP->IAP_CON = ( uint32_t ) IAP_CON_LOCK;
	
		DeInitFlashToRam();
}

/**
  * @brief  Enables or disables IAP programming features
  * @param  FunctionalState[in]:
	*                  - DISABLE:Function disable
	*                  - ENABLE:Function enable
  * @retval None
  */
void IAP_WriteCmd ( FunctionalState NewState )
{
    /* Check the parameters */
    assert_param ( IS_FUNCTIONAL_STATE ( NewState ) );
		
		/* Enable writing if the IAP software lock is open */
		if ( NewState != DISABLE )
		{
				/* Check if software lock is open (IAP write operations are permitted) */
				if(IapWriteFlag==IAP_ENABLE)
				{
						IAP->IAP_CON |= ( uint32_t ) IAP_CON_PRG;
				}
		}
		else
		{
				IAP->IAP_CON &= ~ ( uint32_t ) IAP_CON_PRG;
		}
}

/**
  * @brief  Erases a specified FLASH Sector.
  * @note   If an erase and a program operations are requested simultaneously,
  *         the erase operation is performed before the program one.
  *         Using this function disables the write function.
  * @param  IAP_Sector[in]: The Sector number to be erased.
  * @retval None
  */
void IAP_EraseSector ( uint32_t IAP_Sector )
{
    uint32_t tmpreg;
    IAP->IAP_SNB = 0x4C000000 | IAP_Sector;
    tmpreg = IAP->IAP_CON;
    tmpreg &= ( uint32_t ) ~ ( IAP_CON_ERASE | IAP_CON_CMD | IAP_CON_PRG );
    tmpreg |= ( uint32_t ) ( IAP_CON_SERASE );
		/* Check if software lock is open (IAP write operations are permitted) */
		if(IapWriteFlag==IAP_ENABLE)
		{
				IAP->IAP_CON = tmpreg;
				IAP->IAP_CON |= ( 0x02 << IAP_CON_CMD_Pos );
		}
    
}

#if defined(SC32R803) ||  defined(SC32f12xx)||defined(SC32R805) || defined(SC32R806) || defined(SC32L14xx) || defined(SC32f15xx) || defined(SC32R601)||defined(SC32R807)
/**
  * @brief  Erases a specified FLASH Sector.
  * @note   If an erase and a program operations are requested simultaneously,
  *         the erase operation is performed before the program one.
  *         Using this function disables the write function.
  * @param  IAP_Sector[in]: The Sector number to be erased.
  * @retval None
  */
void IAP_EEPROMEraseSector ( uint32_t IAP_Sector )
{
    uint32_t tmpreg;
    IAP->IAP_SNB = 0x69000000 | IAP_Sector;
    tmpreg = IAP->IAP_CON;
    tmpreg &= (uint32_t)~(IAP_CON_ERASE | IAP_CON_CMD | IAP_CON_PRG);
    tmpreg |= (uint32_t)(IAP_CON_SERASE  );
		/* Check if software lock is open (IAP write operations are permitted) */
		if(IapWriteFlag==IAP_ENABLE)
		{
				IAP->IAP_CON = tmpreg;
				IAP->IAP_CON |= (0x02 << IAP_CON_CMD_Pos);
		}
    IAP->IAP_SNB = 0x4C000000;
}

#endif

#if defined (SC32f10xx)
/**
  * @brief  Program a word (32-bit) at a specified address.
  * @param  Address[in]: specifies the address to be programmed.
  *         This parameter can be any address in Program memory zone or in OTP zone.
  * @param  Data[in]: specifies the data to be programmed.
  * @retval boolType:  The returned value can be: TRUE or FALSE.
	*                 - FALSE
	*                 - TRUE
  */
boolType IAP_ProgramWord ( uint32_t Address, uint32_t Data )
{
    assert_param ( IS_IAP_ADDRESS ( Address ) );
		
		/* Check if the system clock source is HIRC, if using HIRC as system clock, 
	     directly write data to FLASH memory at the specified address */
		if((RCC->RCC_CFG0 &( uint32_t )RCC_CFG0_SYSCLKSW)==RESET)
		{
				/* Check if software lock is open (IAP write operations are permitted) */
				if(IapWriteFlag==IAP_ENABLE)
				{
						* ( __IO uint32_t* ) Address = Data;
				}
				/* Verify the data was written correctly by reading it back */
				if ( Data == * ( __IO uint32_t * ) Address )
						return TRUE;
				else
						return FALSE;
		}
		/* If not using HIRC as system clock, we need to jump to the function pointer pWriteIapFun 
		   which points to the IAP write function that was previously copied to RAM */
		else
		{
				boolType Flag;
				/* Execute the IAP write function from RAM via the function pointer */
				Flag=(*pWriteIapFun)(Address,Data,IAP_WORD);
				/* Return the result from the RAM-executed function */
				return Flag;
		}
}

/**
  * @brief  Program a half word (16-bit) at a specified address.
  * @param  Address[in]: specifies the address to be programmed.
  *         This parameter can be any address in Program memory zone or in OTP zone.
  * @param  Data[in]: specifies the data to be programmed.
  * @retval boolType: The returned value can be: TRUE or FALSE.
	*                 - FALSE
	*                 - TRUE
  */
boolType IAP_ProgramHalfWord ( uint32_t Address, uint16_t Data )
{
    assert_param ( IS_IAP_ADDRESS ( Address ) );

		/* Check if the system clock source is HIRC, if using HIRC as system clock, 
	     directly write data to FLASH memory at the specified address */
		if((RCC->RCC_CFG0 &( uint32_t )RCC_CFG0_SYSCLKSW)==RESET)
		{
				/* Check if software lock is open (IAP write operations are permitted) */
				if(IapWriteFlag==IAP_ENABLE)
				{
						* ( __IO uint16_t* ) Address = Data;
				}
				/* Verify the data was written correctly by reading it back */
				if ( Data == * ( __IO uint16_t * ) Address )
						return TRUE;
				else
						return FALSE;
		}	
		/* If not using HIRC as system clock, we need to jump to the function pointer pWriteIapFun
       which points to the IAP write function that was previously copied to RAM */
		else
		{
				boolType Flag;
				/* Execute the IAP write function from RAM via the function pointer */
				Flag=(*pWriteIapFun)(Address,Data,IAP_HALFWORD);
				/* Return the result from the RAM-executed function */
				return Flag;	
		}
}

/**
  * @brief  Program a byte (8-bit) at a specified address.
  * @param  Address[in]: specifies the address to be programmed.
  *         This parameter can be any address in Program memory zone or in OTP zone.
  * @param  Data[in]: specifies the data to be programmed.
  * @retval boolType:  The returned value can be: TURE or FALSE.
	*                 - FALSE
	*                 - TRUE
  */
boolType IAP_ProgramByte ( uint32_t Address, uint8_t Data )
{
    assert_param ( IS_IAP_ADDRESS ( Address ) );
		
		/* Check if the system clock source is HIRC, if using HIRC as system clock, 
	     directly write data to FLASH memory at the specified address */
		if((RCC->RCC_CFG0 &( uint32_t )RCC_CFG0_SYSCLKSW)==RESET)
		{
				/* Check if software lock is open (IAP write operations are permitted) */
				if(IapWriteFlag==IAP_ENABLE)
				{
						* ( uint8_t* ) Address = Data;
				}
				/* Verify the data was written correctly by reading it back */
				if ( Data == * ( uint8_t * ) Address )
						return TRUE;
				else
						return FALSE;
		}
		/* If not using HIRC as system clock, we need to jump to the function pointer pWriteIapFun
       which points to the IAP write function that was previously copied to RAM */
		else
		{
				boolType Flag;
			  /* Execute the IAP write function from RAM via the function pointer */
				Flag=(*pWriteIapFun)(Address,Data,IAP_BYTE);
			  /* Return the result from the RAM-executed function */
				return Flag;
		}
}

#elif defined(SC32R803)|| defined(SC32f15xx) || defined(SC32R601)
/**
  * @brief  Program a word (32-bit) at a specified address.
  * @param  Address[in]: specifies the address to be programmed.
  *         This parameter can be any address in Program memory zone or in OTP zone.
  * @param  Data[in]: specifies the data to be programmed.
  * @retval boolType:  The returned value can be: TRUE or FALSE.
	*                 - FALSE
	*                 - TRUE
  */
boolType IAP_ProgramWord ( uint32_t Address, uint32_t Data )
{
    assert_param ( IS_IAP_ADDRESS ( Address ) );
		
		/* Check if the system clock source is HIRC, if using HIRC as system clock, 
	     directly write data to FLASH memory at the specified address */
		if(((RCC->RCC_CFG0 &( uint32_t )RCC_CFG0_SYSCLKSW)==RESET)\
			 ||(((RCC->RCC_CFG0 &( uint32_t )RCC_CFG0_SYSCLKSW)!=RESET)&&((RCC->RCC_CFG0 &( uint32_t )RCC_CFG0_SYSCLKSEL)==(0x2UL << RCC_CFG0_SYSCLKSEL_Pos))))
		{
				/* Check if software lock is open (IAP write operations are permitted) */
				if(IapWriteFlag==IAP_ENABLE)
				{
						* ( __IO uint32_t* ) Address = Data;
				}
				/* Verify the data was written correctly by reading it back */
				if ( Data == * ( __IO uint32_t * ) Address )
						return TRUE;
				else
						return FALSE;
		}
		/* If not using HIRC as system clock, we need to jump to the function pointer pWriteIapFun 
		   which points to the IAP write function that was previously copied to RAM */
		else
		{
				boolType Flag;
				/* Execute the IAP write function from RAM via the function pointer */
				Flag=(*pWriteIapFun)(Address,Data,IAP_WORD);
				/* Return the result from the RAM-executed function */
				return Flag;
		}
}

/**
  * @brief  Program a half word (16-bit) at a specified address.
  * @param  Address[in]: specifies the address to be programmed.
  *         This parameter can be any address in Program memory zone or in OTP zone.
  * @param  Data[in]: specifies the data to be programmed.
  * @retval boolType: The returned value can be: TRUE or FALSE.
	*                 - FALSE
	*                 - TRUE
  */
boolType IAP_ProgramHalfWord ( uint32_t Address, uint16_t Data )
{
    assert_param ( IS_IAP_ADDRESS ( Address ) );

		/* Check if the system clock source is HIRC, if using HIRC as system clock, 
	     directly write data to FLASH memory at the specified address */
		if(((RCC->RCC_CFG0 &( uint32_t )RCC_CFG0_SYSCLKSW)==RESET)\
			 ||(((RCC->RCC_CFG0 &( uint32_t )RCC_CFG0_SYSCLKSW)!=RESET)&&((RCC->RCC_CFG0 &( uint32_t )RCC_CFG0_SYSCLKSEL)==(0x2UL << RCC_CFG0_SYSCLKSEL_Pos))))
		{
				/* Check if software lock is open (IAP write operations are permitted) */
				if(IapWriteFlag==IAP_ENABLE)
				{
						* ( __IO uint16_t* ) Address = Data;
				}
				/* Verify the data was written correctly by reading it back */
				if ( Data == * ( __IO uint16_t * ) Address )
						return TRUE;
				else
						return FALSE;
		}	
		/* If not using HIRC as system clock, we need to jump to the function pointer pWriteIapFun
       which points to the IAP write function that was previously copied to RAM */
		else
		{
				boolType Flag;
				/* Execute the IAP write function from RAM via the function pointer */
				Flag=(*pWriteIapFun)(Address,Data,IAP_HALFWORD);
				/* Return the result from the RAM-executed function */
				return Flag;	
		}
}

/**
  * @brief  Program a byte (8-bit) at a specified address.
  * @param  Address[in]: specifies the address to be programmed.
  *         This parameter can be any address in Program memory zone or in OTP zone.
  * @param  Data[in]: specifies the data to be programmed.
  * @retval boolType:  The returned value can be: TURE or FALSE.
	*                 - FALSE
	*                 - TRUE
  */
boolType IAP_ProgramByte ( uint32_t Address, uint8_t Data )
{
    assert_param ( IS_IAP_ADDRESS ( Address ) );
		
		/* Check if the system clock source is HIRC, if using HIRC as system clock, 
	     directly write data to FLASH memory at the specified address */
		if(((RCC->RCC_CFG0 &( uint32_t )RCC_CFG0_SYSCLKSW)==RESET)\
			 ||(((RCC->RCC_CFG0 &( uint32_t )RCC_CFG0_SYSCLKSW)!=RESET)&&((RCC->RCC_CFG0 &( uint32_t )RCC_CFG0_SYSCLKSEL)==(0x2UL << RCC_CFG0_SYSCLKSEL_Pos))))
		{
				/* Check if software lock is open (IAP write operations are permitted) */
				if(IapWriteFlag==IAP_ENABLE)
				{
						* ( uint8_t* ) Address = Data;
				}
				/* Verify the data was written correctly by reading it back */
				if ( Data == * ( uint8_t * ) Address )
						return TRUE;
				else
						return FALSE;
		}
		/* If not using HIRC as system clock, we need to jump to the function pointer pWriteIapFun
       which points to the IAP write function that was previously copied to RAM */
		else
		{
				boolType Flag;
			  /* Execute the IAP write function from RAM via the function pointer */
				Flag=(*pWriteIapFun)(Address,Data,IAP_BYTE);
			  /* Return the result from the RAM-executed function */
				return Flag;
		}
}

#elif defined(SC32f12xx) || defined(SC32R805) || defined(SC32R806) || defined(SC32L14xx)||defined(SC32R807)
/**
  * @brief  Program a word (32-bit) at a specified address.
  * @param  Address[in]: specifies the address to be programmed.
  *         This parameter can be any address in Program memory zone or in OTP zone.
  * @param  Data[in]: specifies the data to be programmed.
  * @retval boolType:  The returned value can be: TRUE or FALSE.
	*                 - FALSE
	*                 - TRUE
  */
boolType IAP_ProgramWord ( uint32_t Address, uint32_t Data )
{
    assert_param ( IS_IAP_ADDRESS ( Address ) );
		
		/* Check if the system clock source is HIRC, if using HIRC as system clock, 
	     directly write data to FLASH memory at the specified address */
		if(((RCC->RCC_CFG0 &( uint32_t )RCC_CFG0_SYSCLKSW)==RESET)\
			 ||(((RCC->RCC_CFG0 &( uint32_t )RCC_CFG0_SYSCLKSW)!=RESET)&&((RCC->RCC_CFG0 &( uint32_t )RCC_CFG0_SYSCLKSEL)==(0x2UL << RCC_CFG0_SYSCLKSEL_Pos))))
		{
				/* Check if software lock is open (IAP write operations are permitted) */
				if(IapWriteFlag==IAP_ENABLE)
				{
						* ( __IO uint32_t* ) Address = Data;
				}
				/* Verify the data was written correctly by reading it back */
				if ( Data == * ( __IO uint32_t * ) Address )
						return TRUE;
				else
						return FALSE;
		}
		/* If not using HIRC as system clock, we need to jump to the function pointer pWriteIapFun 
		   which points to the IAP write function that was previously copied to RAM */
		else
		{
				boolType Flag;
				/* Execute the IAP write function from RAM via the function pointer */
				Flag=(*pWriteIapFun)(Address,Data,IAP_WORD);
				/* Return the result from the RAM-executed function */
				return Flag;
		}
}

/**
  * @brief  Program a byte (8-bit) at a specified address.
  * @param  Address[in]: specifies the address to be programmed.
  *         This parameter can be any address in Program memory zone or in OTP zone.
  * @param  Data[in]: specifies the data to be programmed.
  * @retval boolType:  The returned value can be: TURE or FALSE.
	*                 - FALSE
	*                 - TRUE
  */
boolType IAP_ProgramByte ( uint32_t Address, uint8_t Data )
{
    assert_param ( IS_IAP_ADDRESS ( Address ) );

    uint32_t Addr_Temp = Address & 0xFFFFFFFC;
    /*Read in the data for the aligned address*/
    uint32_t Data_Temp = * ( __IO uint32_t* ) Addr_Temp;
    /*Inserts the data to be written*/
    switch ( Address & 0x00000003 )
    {
				case 0:
				{
						Data_Temp |= ( uint32_t ) Data;
						break;
				}
				case 1:
				{
						Data_Temp |= ( uint32_t ) ( Data << 8 );
						break;
				}
				case 2:
				{
						Data_Temp |= ( uint32_t ) ( Data << 16 );
						break;
				}
				default:
				{
						Data_Temp |= ( uint32_t ) ( Data << 24 );
						break;
				}
    }
		/* Check if the system clock source is HIRC, if using HIRC as system clock, 
	     directly write data to FLASH memory at the specified address */
		if(((RCC->RCC_CFG0 &( uint32_t )RCC_CFG0_SYSCLKSW)==RESET)\
			 ||(((RCC->RCC_CFG0 &( uint32_t )RCC_CFG0_SYSCLKSW)!=RESET)&&((RCC->RCC_CFG0 &( uint32_t )RCC_CFG0_SYSCLKSEL)==(0x2UL << RCC_CFG0_SYSCLKSEL_Pos))))
		{
				/*Write incoming data*/
				/* Check if software lock is open (IAP write operations are permitted) */
				if(IapWriteFlag==IAP_ENABLE)
				{
						* ( __IO uint32_t* ) Addr_Temp = Data_Temp;
				}
				/* Verify the data was written correctly by reading it back */
				if ( Data_Temp == * ( __IO uint32_t * ) Addr_Temp )
						return TRUE;
				else
						return FALSE;
		}
		/* If not using HIRC as system clock, we need to jump to the function pointer pWriteIapFun
       which points to the IAP write function that was previously copied to RAM */
		else
		{
				boolType Flag;
			  /* Execute the IAP write function from RAM via the function pointer */
				Flag=(*pWriteIapFun)(Addr_Temp,Data_Temp,IAP_WORD);
			  /* Return the result from the RAM-executed function */
				return Flag;
		}
}

/**
  * @brief  Program a half word (16-bit) at a specified address.
  * @param  Address[in]: specifies the address to be programmed.
  *         This parameter can be any address in Program memory zone or in OTP zone.
  * @param  Data[in]: specifies the data to be programmed.
  * @retval boolType: The returned value can be: TRUE or FALSE.
	*                 - FALSE
	*                 - TRUE
  */
boolType IAP_ProgramHalfWord ( uint32_t Address, uint16_t Data )
{
    assert_param ( IS_IAP_ADDRESS ( Address ) );

    uint32_t Addr_Temp = Address & 0xFFFFFFFC;
    /*Read in the data for the aligned address*/
    uint32_t Data_Temp = * ( __IO uint32_t* ) Addr_Temp;
    /*Inserts the data to be written*/
    switch ( Address & 0x00000003 )
    {
				case 0:
				{
						Data_Temp |= ( uint32_t ) Data;
						break;
				}
				case 1:
				{
						return FALSE;  //The address is misaligned, wrong 
				}
				case 2:
				{
						Data_Temp |= ( uint32_t ) ( Data << 16 );
						break;
				}
				case 3:
				{
						return FALSE;  //The address is misaligned, wrong
				}
    }
		/* Check if the system clock source is HIRC, if using HIRC as system clock, 
	     directly write data to FLASH memory at the specified address */
		if(((RCC->RCC_CFG0 &( uint32_t )RCC_CFG0_SYSCLKSW)==RESET)\
			 ||(((RCC->RCC_CFG0 &( uint32_t )RCC_CFG0_SYSCLKSW)!=RESET)&&((RCC->RCC_CFG0 &( uint32_t )RCC_CFG0_SYSCLKSEL)==(0x2UL << RCC_CFG0_SYSCLKSEL_Pos))))
		{
				/*Write incoming data*/
				/* Check if software lock is open (IAP write operations are permitted) */
				if(IapWriteFlag==IAP_ENABLE)
				{
						* ( __IO uint32_t* ) Addr_Temp = Data_Temp;
				}
				/* Verify the data was written correctly by reading it back */
				if ( Data_Temp == * ( __IO uint32_t * ) Addr_Temp )
						return TRUE;
				else
						return FALSE;
		}
		/* If not using HIRC as system clock, we need to jump to the function pointer pWriteIapFun
       which points to the IAP write function that was previously copied to RAM */
		else
		{
				boolType Flag;
			  /* Execute the IAP write function from RAM via the function pointer */
				Flag=(*pWriteIapFun)(Addr_Temp,Data_Temp,IAP_WORD);
			  /* Return the result from the RAM-executed function */
				return Flag;
		}
}
#endif

/**
  * @brief  Read a word (32-bit) at a specified address.
  * @param  Address[in]: specifies the address to be read.
  *         This parameter can be any address in Program memory zone or in OTP zone.
  * @retval uint32_t: The data to be read.
  */
uint32_t IAP_ReadWord ( uint32_t Address )
{
    assert_param ( IS_IAP_ADDRESS ( Address ) );

    uint32_t Data;
    Data = * ( __IO uint32_t* ) Address;
    return Data;
}

/**
  * @brief  Read a half word (16-bit) at a specified address.
  * @param  Address[in]: specifies the address to be read.
  *         This parameter can be any address in Program memory zone or in OTP zone.
  * @retval uint16_t: The data to be read.
  */
uint16_t IAP_ReadHalfWord ( uint32_t Address )
{
    assert_param ( IS_IAP_ADDRESS ( Address ) );

    uint16_t Data;
    Data = * ( __IO uint16_t* ) Address;
    return Data;
}

/**
  * @brief  Read a byte (8-bit) at a specified address.
  * @param  Address[in]: specifies the address to be read.
  *         This parameter can be any address in Program memory zone or in OTP zone.
  * @retval uint8_t: The data to be read.
  */
uint8_t IAP_ReadByte ( uint32_t Address )
{
    assert_param ( IS_IAP_ADDRESS ( Address ) );

    uint8_t Data;
    Data = * ( uint8_t* ) Address;
    return Data;
}

#if defined(SC32f12xx) || defined(SC32R805) || defined(SC32R806) || defined(SC32L14xx)||defined(SC32R807)
/**
  * @brief  Program a word (32-bit) array of configurable length.
  * @param  Address[in]: specifies the address to be program.
  *         This parameter can be any address in Program memory zone or in OTP zone.
  * @param  ByteArray[out]:An array pointer to write data.
  * @param  ArraySize[in]:Array size.
  * @retval uint16_t:The length of the array that needs to be written.
  */
uint16_t IAP_ProgramWordArray ( uint32_t Address, uint32_t* ByteArray, uint16_t ArraySize )
{
    assert_param ( IS_IAP_ADDRESS ( Address ) );
		/* Counter for successfully programmed words */
    uint16_t tmpCnt = 0;
		/* Check if the system clock source is HIRC, if using HIRC as system clock, 
	     directly write data to FLASH memory at the specified address */
		if(((RCC->RCC_CFG0 &( uint32_t )RCC_CFG0_SYSCLKSW)==RESET)\
			 ||(((RCC->RCC_CFG0 &( uint32_t )RCC_CFG0_SYSCLKSW)!=RESET)&&((RCC->RCC_CFG0 &( uint32_t )RCC_CFG0_SYSCLKSEL)==(0x2UL << RCC_CFG0_SYSCLKSEL_Pos))))
		{
				/* Check if address is 32-bit aligned (required for word programming) */
				if ( ( Address & 0x03 ) == 0 )
				{
						/* Check if software lock is open (IAP write operations are permitted) */
						if(IapWriteFlag==IAP_ENABLE)
						{
								/* Program each word in the array until completion or error */
								do
								{
										* ( __IO uint32_t* ) Address = ByteArray[tmpCnt];
										/* Verify the write operation by reading back the value */
										if ( ByteArray[tmpCnt] != * ( __IO uint32_t * ) Address )
												break;
										/* Move to next 32-bit address */
										Address = Address + 4;
								/* Continue until all words are programmed */
								} while ( ++tmpCnt < ArraySize );
						}
				}
				/* Return count of successfully programmed words */
				return tmpCnt;
		}
		/* If not using HIRC as system clock, we need to jump to the function pointer pWriteIapFun
       which points to the IAP write function that was previously copied to RAM */
		else
		{
				/* Define function pointer type for the RAM-based IAP programming function */
				typedef uint16_t (*pInitFlashInRam)( uint32_t Address, uint8_t* ByteArray, uint16_t ArraySize,IAPWRITE_ALIGNTYPE Type );
				/* Allocate and initialize RAM buffer for the IAP programming function */
				__ALIGNED(4) uint8_t IapWriteInRam[256]={0};
				/* Copy the IAP programming function from FLASH to RAM */
				for(uint16_t i=0;i<sizeof(IapWriteInRam);i++)
				{
						IapWriteInRam[i] =*(uint8_t*)((uint32_t)IAP_ProgramArray_InRam-1+i);
				}
				/* Create function pointer to the RAM-based function and execute it */
				/* The "|0x01" operation sets the Thumb bit for ARM architecture compatibility.
				   In ARM architecture, the least significant bit (LSB) of a function pointer
				   indicates the instruction set (0 for ARM, 1 for Thumb). This ensures the
				   function executes in Thumb mode, which is typically used for RAM-based code.*/
				pInitFlashInRam p=(pInitFlashInRam)((uint32_t)IapWriteInRam|0x01);
				tmpCnt =(*p)(Address,(uint8_t*)ByteArray,ArraySize,IAP_WORD);
				/* Clear the RAM buffer */
				for(uint16_t i=0;i<sizeof(IapWriteInRam);i++)
				{
						IapWriteInRam[i] =0;
				}
				/* Return count of successfully programmed words */
				return tmpCnt;		
		}
}

#elif defined(SC32f10xx)
/**
  * @brief  Program a word (32-bit) array of configurable length.
  * @param  Address[in]: specifies the address to be program.
  *         This parameter can be any address in Program memory zone or in OTP zone.
  * @param  ByteArray[out]:An array pointer to write data.
  * @param  ArraySize[in]:Array size.
  * @retval uint16_t:The length of the array that needs to be written.
  */
uint16_t IAP_ProgramWordArray ( uint32_t Address, uint32_t* ByteArray, uint16_t ArraySize )
{
    assert_param ( IS_IAP_ADDRESS ( Address ) );
		/* Counter for successfully programmed words */
    uint16_t tmpCnt = 0;
		/* Check if the system clock source is HIRC, if using HIRC as system clock, 
	     directly write data to FLASH memory at the specified address */
		if((RCC->RCC_CFG0 &( uint32_t )RCC_CFG0_SYSCLKSW)==RESET)
		{
				/* Check if address is 32-bit aligned (required for word programming) */
				if ( ( Address & 0x03 ) == 0 )
				{
						/* Check if software lock is open (IAP write operations are permitted) */
						if(IapWriteFlag==IAP_ENABLE)
						{
								/* Program each word in the array until completion or error */
								do
								{
										* ( __IO uint32_t* ) Address = ByteArray[tmpCnt];
										/* Verify the write operation by reading back the value */
										if ( ByteArray[tmpCnt] != * ( __IO uint32_t * ) Address )
												break;
										/* Move to next 32-bit address */
										Address = Address + 4;
								/* Continue until all words are programmed */
								} while ( ++tmpCnt < ArraySize );
						}
				}
				/* Return count of successfully programmed words */
				return tmpCnt;
		}
		/* If not using HIRC as system clock, we need to jump to the function pointer pWriteIapFun
       which points to the IAP write function that was previously copied to RAM */
		else
		{
				/* Define function pointer type for the RAM-based IAP programming function */
				typedef uint16_t (*pInitFlashInRam)( uint32_t Address, uint8_t* ByteArray, uint16_t ArraySize,IAPWRITE_ALIGNTYPE Type );
				/* Allocate and initialize RAM buffer for the IAP programming function */
				__ALIGNED(4) uint8_t IapWriteInRam[256]={0};
				/* Copy the IAP programming function from FLASH to RAM */
				for(uint16_t i=0;i<sizeof(IapWriteInRam);i++)
				{
						IapWriteInRam[i] =*(uint8_t*)((uint32_t)IAP_ProgramArray_InRam-1+i);
				}
				/* Create function pointer to the RAM-based function and execute it */
				/* The "|0x01" operation sets the Thumb bit for ARM architecture compatibility.
				   In ARM architecture, the least significant bit (LSB) of a function pointer
				   indicates the instruction set (0 for ARM, 1 for Thumb). This ensures the
				   function executes in Thumb mode, which is typically used for RAM-based code.*/
				pInitFlashInRam p=(pInitFlashInRam)((uint32_t)IapWriteInRam|0x01);
				tmpCnt =(*p)(Address,(uint8_t*)ByteArray,ArraySize,IAP_WORD);
				/* Clear the RAM buffer */
				for(uint16_t i=0;i<sizeof(IapWriteInRam);i++)
				{
						IapWriteInRam[i] =0;
				}
				/* Return count of successfully programmed words */
				return tmpCnt;		
		}
}

/**
  * @brief  Program a half word (16-bit) array of configurable length.
  * @param  Address[in]: specifies the address to be program.
  *         This parameter can be any address in Program memory zone or in OTP zone.
  * @param  ByteArray[out]:An array pointer to write data.
  * @param  ArraySize[in]:Array size.
  * @retval uint16_t :The length of the array that needs to be written.
  */
uint16_t IAP_ProgramHalfWordArray ( uint32_t Address, uint16_t* ByteArray, uint16_t ArraySize )
{
    assert_param ( IS_IAP_ADDRESS ( Address ) );
		/* Counter for successfully programmed half-words */
    uint16_t tmpCnt = 0;
		/* Check if the system clock source is HIRC, if using HIRC as system clock, 
	     directly write data to FLASH memory at the specified address */
		if((RCC->RCC_CFG0 &( uint32_t )RCC_CFG0_SYSCLKSW)==RESET)
		{
				/* Check if address is 16-bit aligned (required for half-word programming) */
				if ( ( Address & 0x01 ) == 0 )
				{
						/* Check if software lock is open (IAP write operations are permitted) */
						if(IapWriteFlag==IAP_ENABLE)
						{
								/* Program each half-word in the array until completion or error */
								do
								{
										* ( __IO uint16_t* ) Address = ByteArray[tmpCnt];
										/* Verify the write operation by reading back the value */
										if ( ByteArray[tmpCnt] != * ( __IO uint16_t * ) Address )
												break;
										/* Move to next 16-bit address */
										Address = Address + 2;
								/* Continue until all half-words are programmed */
								} while ( ++tmpCnt < ArraySize );
					  }
				}
				/* Return count of successfully programmed half-words */
				return tmpCnt;
		}
		/* If not using HIRC as system clock, we need to jump to the function pointer pWriteIapFun
       which points to the IAP write function that was previously copied to RAM */
		else
		{
				/* Define function pointer type for the RAM-based IAP programming function */
				typedef uint16_t (*pInitFlashInRam)( uint32_t Address, uint8_t* ByteArray, uint16_t ArraySize,IAPWRITE_ALIGNTYPE Type );
				/* Allocate and initialize RAM buffer for the IAP programming function */
				__ALIGNED(4) uint8_t IapWriteInRam[256]={0};
				/* Copy the IAP programming function from FLASH to RAM */
				for(uint16_t i=0;i<sizeof(IapWriteInRam);i++)
				{
						IapWriteInRam[i] =*(uint8_t*)((uint32_t)IAP_ProgramArray_InRam-1+i);
				}	
				/* Create function pointer to the RAM-based function and execute it */
				/* The "|0x01" operation sets the Thumb bit for ARM architecture compatibility.
				   In ARM architecture, the least significant bit (LSB) of a function pointer
				   indicates the instruction set (0 for ARM, 1 for Thumb). This ensures the
				   function executes in Thumb mode, which is typically used for RAM-based code.*/
				pInitFlashInRam p=(pInitFlashInRam)((uint32_t)IapWriteInRam|0x01);
				tmpCnt =(*p)(Address,(uint8_t*)ByteArray,ArraySize,IAP_HALFWORD);
				/* Clear the RAM buffer */
				for(uint16_t i=0;i<sizeof(IapWriteInRam);i++)
				{
						IapWriteInRam[i] =0;
				}
				/* Return count of successfully programmed half-words */
				return tmpCnt;
		}
}

/**
  * @brief  Program a byte (8-bit) array of configurable length.
  * @param  Address[in]: specifies the address to be program.
  *         This parameter can be any address in Program memory zone or in OTP zone.
  * @param  ByteArray[out]:An array pointer to write data.
  * @param  ArraySize[in]:Array size.
  * @retval uint16_t :The length of the array that needs to be written.
  */
uint16_t IAP_ProgramByteArray ( uint32_t Address, uint8_t* ByteArray, uint16_t ArraySize )
{
    assert_param ( IS_IAP_ADDRESS ( Address ) );
		/* Counter for successfully programmed bytes */
    uint16_t tmpCnt = 0;
		/* Check if the system clock source is HIRC, if using HIRC as system clock, 
	     directly write data to FLASH memory at the specified address */
		if((RCC->RCC_CFG0 &( uint32_t )RCC_CFG0_SYSCLKSW)==RESET)
		{
				/* Check if software lock is open (IAP write operations are permitted) */
				if(IapWriteFlag==IAP_ENABLE)
				{
						/* Program each byte in the array until completion or error */
						do
						{
								* ( __IO uint8_t* ) Address = ByteArray[tmpCnt];
								/* Verify the write operation by reading back the value */
								if ( ByteArray[tmpCnt] != * ( __IO uint8_t * ) Address )
										break;
								/* Move to next byte address */
								Address = Address + 1;
						/* Continue until all bytes are programmed */
						} while ( ++tmpCnt < ArraySize );
				}
				/* Return count of successfully programmed bytes */
				return tmpCnt;
		}
		/* If not using HIRC as system clock, we need to jump to the function pointer pWriteIapFun
       which points to the IAP write function that was previously copied to RAM */
		else
		{
				/* Define function pointer type for the RAM-based IAP programming function */
				typedef uint16_t (*pInitFlashInRam)( uint32_t Address, uint8_t* ByteArray, uint16_t ArraySize,IAPWRITE_ALIGNTYPE Type );
				/* Allocate and initialize RAM buffer for the IAP programming function */
				__ALIGNED(4) uint8_t IapWriteInRam[256]={0};	
				/* Copy the IAP programming function from FLASH to RAM */
				for(uint16_t i=0;i<sizeof(IapWriteInRam);i++)
				{
						IapWriteInRam[i] =*(uint8_t*)((uint32_t)IAP_ProgramArray_InRam-1+i);
				}	
				/* Create function pointer to the RAM-based function and execute it */
				/* The "|0x01" operation sets the Thumb bit for ARM architecture compatibility.
				   In ARM architecture, the least significant bit (LSB) of a function pointer
				   indicates the instruction set (0 for ARM, 1 for Thumb). This ensures the
				   function executes in Thumb mode, which is typically used for RAM-based code.*/
				pInitFlashInRam p=(pInitFlashInRam)((uint32_t)IapWriteInRam|0x01);
				tmpCnt =(*p)(Address,(uint8_t*)ByteArray,ArraySize,IAP_BYTE);
				/* Clear the RAM buffer */
				for(uint16_t i=0;i<sizeof(IapWriteInRam);i++)
				{
						IapWriteInRam[i] =0;
				}			
				/* Return count of successfully programmed bytes */
				return tmpCnt;	
		}
}

#elif defined(SC32R803) || defined(SC32f15xx) || defined(SC32R601)
/**
  * @brief  Program a word (32-bit) array of configurable length.
  * @param  Address[in]: specifies the address to be program.
  *         This parameter can be any address in Program memory zone or in OTP zone.
  * @param  ByteArray[out]:An array pointer to write data.
  * @param  ArraySize[in]:Array size.
  * @retval uint16_t:The length of the array that needs to be written.
  */
uint16_t IAP_ProgramWordArray ( uint32_t Address, uint32_t* ByteArray, uint16_t ArraySize )
{
    assert_param ( IS_IAP_ADDRESS ( Address ) );
		/* Counter for successfully programmed words */
    uint16_t tmpCnt = 0;
		/* Check if the system clock source is HIRC, if using HIRC as system clock, 
	     directly write data to FLASH memory at the specified address */
		if(((RCC->RCC_CFG0 &( uint32_t )RCC_CFG0_SYSCLKSW)==RESET)\
			 ||(((RCC->RCC_CFG0 &( uint32_t )RCC_CFG0_SYSCLKSW)!=RESET)&&((RCC->RCC_CFG0 &( uint32_t )RCC_CFG0_SYSCLKSEL)==(0x2UL << RCC_CFG0_SYSCLKSEL_Pos))))
		{
				/* Check if address is 32-bit aligned (required for word programming) */
				if ( ( Address & 0x03 ) == 0 )
				{
						/* Check if software lock is open (IAP write operations are permitted) */
						if(IapWriteFlag==IAP_ENABLE)
						{
								/* Program each word in the array until completion or error */
								do
								{
										* ( __IO uint32_t* ) Address = ByteArray[tmpCnt];
										/* Verify the write operation by reading back the value */
										if ( ByteArray[tmpCnt] != * ( __IO uint32_t * ) Address )
												break;
										/* Move to next 32-bit address */
										Address = Address + 4;
								/* Continue until all words are programmed */
								} while ( ++tmpCnt < ArraySize );
						}
				}
				/* Return count of successfully programmed words */
				return tmpCnt;
		}
		/* If not using HIRC as system clock, we need to jump to the function pointer pWriteIapFun
       which points to the IAP write function that was previously copied to RAM */
		else
		{
				/* Define function pointer type for the RAM-based IAP programming function */
				typedef uint16_t (*pInitFlashInRam)( uint32_t Address, uint8_t* ByteArray, uint16_t ArraySize,IAPWRITE_ALIGNTYPE Type );
				/* Allocate and initialize RAM buffer for the IAP programming function */
				__ALIGNED(4) uint8_t IapWriteInRam[256]={0};
				/* Copy the IAP programming function from FLASH to RAM */
				for(uint16_t i=0;i<sizeof(IapWriteInRam);i++)
				{
						IapWriteInRam[i] =*(uint8_t*)((uint32_t)IAP_ProgramArray_InRam-1+i);
				}
				/* Create function pointer to the RAM-based function and execute it */
				/* The "|0x01" operation sets the Thumb bit for ARM architecture compatibility.
				   In ARM architecture, the least significant bit (LSB) of a function pointer
				   indicates the instruction set (0 for ARM, 1 for Thumb). This ensures the
				   function executes in Thumb mode, which is typically used for RAM-based code.*/
				pInitFlashInRam p=(pInitFlashInRam)((uint32_t)IapWriteInRam|0x01);
				tmpCnt =(*p)(Address,(uint8_t*)ByteArray,ArraySize,IAP_WORD);
				/* Clear the RAM buffer */
				for(uint16_t i=0;i<sizeof(IapWriteInRam);i++)
				{
						IapWriteInRam[i] =0;
				}
				/* Return count of successfully programmed words */
				return tmpCnt;		
		}
}

/**
  * @brief  Program a half word (16-bit) array of configurable length.
  * @param  Address[in]: specifies the address to be program.
  *         This parameter can be any address in Program memory zone or in OTP zone.
  * @param  ByteArray[out]:An array pointer to write data.
  * @param  ArraySize[in]:Array size.
  * @retval uint16_t :The length of the array that needs to be written.
  */
uint16_t IAP_ProgramHalfWordArray ( uint32_t Address, uint16_t* ByteArray, uint16_t ArraySize )
{
    assert_param ( IS_IAP_ADDRESS ( Address ) );
		/* Counter for successfully programmed half-words */
    uint16_t tmpCnt = 0;
		/* Check if the system clock source is HIRC, if using HIRC as system clock, 
	     directly write data to FLASH memory at the specified address */
		if(((RCC->RCC_CFG0 &( uint32_t )RCC_CFG0_SYSCLKSW)==RESET)\
			 ||(((RCC->RCC_CFG0 &( uint32_t )RCC_CFG0_SYSCLKSW)!=RESET)&&((RCC->RCC_CFG0 &( uint32_t )RCC_CFG0_SYSCLKSEL)==(0x2UL << RCC_CFG0_SYSCLKSEL_Pos))))
		{
				/* Check if address is 16-bit aligned (required for half-word programming) */
				if ( ( Address & 0x01 ) == 0 )
				{
						/* Check if software lock is open (IAP write operations are permitted) */
						if(IapWriteFlag==IAP_ENABLE)
						{
								/* Program each half-word in the array until completion or error */
								do
								{
										* ( __IO uint16_t* ) Address = ByteArray[tmpCnt];
										/* Verify the write operation by reading back the value */
										if ( ByteArray[tmpCnt] != * ( __IO uint16_t * ) Address )
												break;
										/* Move to next 16-bit address */
										Address = Address + 2;
								/* Continue until all half-words are programmed */
								} while ( ++tmpCnt < ArraySize );
					  }
				}
				/* Return count of successfully programmed half-words */
				return tmpCnt;
		}
		/* If not using HIRC as system clock, we need to jump to the function pointer pWriteIapFun
       which points to the IAP write function that was previously copied to RAM */
		else
		{
				/* Define function pointer type for the RAM-based IAP programming function */
				typedef uint16_t (*pInitFlashInRam)( uint32_t Address, uint8_t* ByteArray, uint16_t ArraySize,IAPWRITE_ALIGNTYPE Type );
				/* Allocate and initialize RAM buffer for the IAP programming function */
				__ALIGNED(4) uint8_t IapWriteInRam[256]={0};
				/* Copy the IAP programming function from FLASH to RAM */
				for(uint16_t i=0;i<sizeof(IapWriteInRam);i++)
				{
						IapWriteInRam[i] =*(uint8_t*)((uint32_t)IAP_ProgramArray_InRam-1+i);
				}	
				/* Create function pointer to the RAM-based function and execute it */
				/* The "|0x01" operation sets the Thumb bit for ARM architecture compatibility.
				   In ARM architecture, the least significant bit (LSB) of a function pointer
				   indicates the instruction set (0 for ARM, 1 for Thumb). This ensures the
				   function executes in Thumb mode, which is typically used for RAM-based code.*/
				pInitFlashInRam p=(pInitFlashInRam)((uint32_t)IapWriteInRam|0x01);
				tmpCnt =(*p)(Address,(uint8_t*)ByteArray,ArraySize,IAP_HALFWORD);
				/* Clear the RAM buffer */
				for(uint16_t i=0;i<sizeof(IapWriteInRam);i++)
				{
						IapWriteInRam[i] =0;
				}
				/* Return count of successfully programmed half-words */
				return tmpCnt;
		}
}

/**
  * @brief  Program a byte (8-bit) array of configurable length.
  * @param  Address[in]: specifies the address to be program.
  *         This parameter can be any address in Program memory zone or in OTP zone.
  * @param  ByteArray[out]:An array pointer to write data.
  * @param  ArraySize[in]:Array size.
  * @retval uint16_t :The length of the array that needs to be written.
  */
uint16_t IAP_ProgramByteArray ( uint32_t Address, uint8_t* ByteArray, uint16_t ArraySize )
{
    assert_param ( IS_IAP_ADDRESS ( Address ) );
		/* Counter for successfully programmed bytes */
    uint16_t tmpCnt = 0;
		/* Check if the system clock source is HIRC, if using HIRC as system clock, 
	     directly write data to FLASH memory at the specified address */
		if(((RCC->RCC_CFG0 &( uint32_t )RCC_CFG0_SYSCLKSW)==RESET)\
			 ||(((RCC->RCC_CFG0 &( uint32_t )RCC_CFG0_SYSCLKSW)!=RESET)&&((RCC->RCC_CFG0 &( uint32_t )RCC_CFG0_SYSCLKSEL)==(0x2UL << RCC_CFG0_SYSCLKSEL_Pos))))
		{
				/* Check if software lock is open (IAP write operations are permitted) */
				if(IapWriteFlag==IAP_ENABLE)
				{
						/* Program each byte in the array until completion or error */
						do
						{
								* ( __IO uint8_t* ) Address = ByteArray[tmpCnt];
								/* Verify the write operation by reading back the value */
								if ( ByteArray[tmpCnt] != * ( __IO uint8_t * ) Address )
										break;
								/* Move to next byte address */
								Address = Address + 1;
						/* Continue until all bytes are programmed */
						} while ( ++tmpCnt < ArraySize );
				}
				/* Return count of successfully programmed bytes */
				return tmpCnt;
		}
		/* If not using HIRC as system clock, we need to jump to the function pointer pWriteIapFun
       which points to the IAP write function that was previously copied to RAM */
		else
		{
				/* Define function pointer type for the RAM-based IAP programming function */
				typedef uint16_t (*pInitFlashInRam)( uint32_t Address, uint8_t* ByteArray, uint16_t ArraySize,IAPWRITE_ALIGNTYPE Type );
				/* Allocate and initialize RAM buffer for the IAP programming function */
				__ALIGNED(4) uint8_t IapWriteInRam[256]={0};	
				/* Copy the IAP programming function from FLASH to RAM */
				for(uint16_t i=0;i<sizeof(IapWriteInRam);i++)
				{
						IapWriteInRam[i] =*(uint8_t*)((uint32_t)IAP_ProgramArray_InRam-1+i);
				}	
				/* Create function pointer to the RAM-based function and execute it */
				/* The "|0x01" operation sets the Thumb bit for ARM architecture compatibility.
				   In ARM architecture, the least significant bit (LSB) of a function pointer
				   indicates the instruction set (0 for ARM, 1 for Thumb). This ensures the
				   function executes in Thumb mode, which is typically used for RAM-based code.*/
				pInitFlashInRam p=(pInitFlashInRam)((uint32_t)IapWriteInRam|0x01);
				tmpCnt =(*p)(Address,(uint8_t*)ByteArray,ArraySize,IAP_BYTE);
				/* Clear the RAM buffer */
				for(uint16_t i=0;i<sizeof(IapWriteInRam);i++)
				{
						IapWriteInRam[i] =0;
				}			
				/* Return count of successfully programmed bytes */
				return tmpCnt;	
		}
}

#endif
/**
  * @brief  Read a Word (32-bit) array of configurable length.
  * @param  Address[in]: specifies the address to be program.
  *         This parameter can be any address in Program memory zone or in OTP zone.
  * @param  ByteArray[out]:An array pointer to write data.
  * @param  ArraySize[in]:Array size.
  * @retval uint16_t:The length of the array that needs to be written.
  */
uint16_t IAP_ReadWordArray ( uint32_t Address, uint32_t* ByteArray, uint16_t ArraySize )
{
    assert_param ( IS_IAP_ADDRESS ( Address ) );

    uint16_t tmpCnt = 0;
    if ( ( Address & 0x03 ) == 0 )
    {
        do
        {
            ByteArray[tmpCnt] = * ( __IO uint32_t* ) Address;
            Address = Address + 4;

        } while ( ++tmpCnt < ArraySize );
    }
    return tmpCnt;
}

/**
  * @brief  Read a half word (16-bit) array of configurable length.
  * @param  Address[in]: specifies the address to be read.
  *         This parameter can be any address in Program memory zone or in OTP zone.
  * @param  ByteArray[out]:An array pointer to read data.
  * @param  ArraySize[in]:Array size.
  * @retval uint16_t:The length of the array that needs to be read.
  */
uint16_t IAP_ReadHalfWordArray ( uint32_t Address, uint16_t* ByteArray, uint16_t ArraySize )
{
    assert_param ( IS_IAP_ADDRESS ( Address ) );

    uint16_t tmpCnt = 0;
    if ( ( Address & 0x01 ) == 0 )
    {
        do
        {
            ByteArray[tmpCnt] = * ( __IO uint16_t* ) Address;
            Address = Address + 2;

        } while ( ++tmpCnt < ArraySize );
    }
    return tmpCnt;
}

/**
  * @brief  Read a byte (8-bit) array of configurable length.
  * @param  Address[in]: specifies the address to be read.
  *         This parameter can be any address in Program memory zone or in OTP zone.
  * @param  ByteArray[out]:An array pointer to read data.
  * @param  ArraySize[in]:Array size.
  * @retval uint16_t:The length of the array that needs to be read.
  */
uint16_t IAP_ReadByteArray ( uint32_t Address, uint8_t* ByteArray, uint16_t ArraySize )
{
    assert_param ( IS_IAP_ADDRESS ( Address ) );

    uint16_t tmpCnt = 0;
    do
    {
        ByteArray[tmpCnt] = * ( __IO uint8_t* ) Address;
        Address = Address + 1;

    } while ( ++tmpCnt < ArraySize );

    return tmpCnt;
}

/**
 * @}
 */
/* End of IAP_Group1.	*/

/** @defgroup IAP_Group2 Software reset functions
 *  @brief   Software reset functions
 *
@verbatim
 ===============================================================================
                     ##### Software reset  functions #####
 ===============================================================================
@endverbatim
  * @{
  */

/**
  * @brief  IAP Software Reset.
  * @param  IAP_BTLDType[in]:
  *                   - IAP_BTLD_APPROM :Select the APPROM region
  *                   - IAP_BTLD_LDROM:Select the LDROM region
  *                   - IAP_BTLD_SRAM:Select the SRAM region
	*
  */
void IAP_SoftwareReset ( IAP_BTLD_TypeDef IAP_BTLDType )
{
    IAP->IAP_CON = ( IAP_BTLDType | IAP_CON_RST );
}
/**
 * @}
 */
/* End of IAP_Group2.	*/

#if defined(SC32f15xx) || defined(SC32R601)
/** @defgroup IAP_Group3 Automatic continuous burning  functions
 *  @brief   Automatic continuous burning  functions
 *
@verbatim
 ===============================================================================
                     ##### Automatic continuous burning  functions #####
 ===============================================================================
@endverbatim
  * @{
  */

/**
  * @brief  IAP Software Reset.
  * @param  IAP_BTLDType[in]: 
  *                   - IAP_BTLD_APPROM :Select the APPROM region
  *                   - IAP_BTLD_LDROM:Select the LDROM region
  *                   - IAP_BTLD_SRAM:Select the SRAM region
	*
  */
void IAP_DMAENCmd(FunctionalState NewState)
{
  assert_param(IS_FUNCTIONAL_STATE(NewState));
	
  if(NewState != DISABLE)
  {
    IAP->IAP_CON |= (uint32_t)IAP_CON_DMAEN;
  }
  else
  {
    IAP->IAP_CON &= ~(uint32_t)IAP_CON_DMAEN;
  }
}

/**
  * @brief  IAP Software Reset.
  * @param  IAP_BTLDType[in]: 
  *                   - IAP_BTLD_APPROM :Select the APPROM region
  *                   - IAP_BTLD_LDROM:Select the LDROM region
  *                   - IAP_BTLD_SRAM:Select the SRAM region
	*
  */
void IAP_CONTBurnlength(uint8_t Burnlength)
{
   IAP->IAP_CON &=  (uint32_t)IAP_CON_CONT;
	
	 IAP->IAP_CON |=  Burnlength<<IAP_CON_CONT_Pos;
}
/**
 * @}
 */
/* End of IAP_Group3.	*/
#endif

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT SOC Microelectronics *****END OF FILE****/
