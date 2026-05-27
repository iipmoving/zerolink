
#include "rx32g4xx_config_def.h"
#include "rx32g4xx_hal.h"






#include	"API_UART.h"

__weak  void    API_UART_RxEventCallback(uint16_t size)
{
    
}

		
typedef struct
{
   UART_TypeDef*			Instance;                      /*!< OPAMP instance's registers base address   */
   UART_InitTypeDef*	Init;                           /*!< OPAMP required parameters */

	
}UART_TOTAL_InitTypeDef;	
		
UART_InitTypeDef	const 	Uart1_InitTypeSet=
{
  Uart1BaudRate,/*uint32_t BaudRate;                  !< This member configures the UART communication baud rate.
                                           The baud rate is computed using the following formula:
                                           - IntegerDivider = ((PCLKx) / (16 * (huart->Init.BaudRate)))
                                           - FractionalDivider = ((IntegerDivider - ((uint32_t) IntegerDivider)) * 16) + 0.5 */

  UART_WORDLENGTH_8B,/*uint32_t WordLength;                !< Specifies the number of data bits transmitted or received in a frame.
                                           This parameter can be a value of @ref UART_Word_Length */

  UART_STOPBITS_1,/*uint32_t StopBits;                  !< Specifies the number of stop bits transmitted.
                                           This parameter can be a value of @ref UART_Stop_Bits */

  UART_PARITY_NONE,/*uint32_t Parity;                    !< Specifies the parity mode.
                                           This parameter can be a value of @ref UART_Parity
                                           @note When parity is enabled, the computed parity is inserted
                                                 at the MSB position of the transmitted data (9th bit when
                                                 the word length is set to 9 data bits; 8th bit when the
                                                 word length is set to 8 data bits). */

  UART_MODE_TX_RX,/*uint32_t Mode;                      !< Specifies whether the Receive or Transmit mode is enabled or disabled.
                                           This parameter can be a value of @ref UART_Mode */

  UART_HWCONTROL_NONE,/*uint32_t HwFlowCtl;                 !< Specifies whether the hardware flow control mode is enabled or disabled.*/
 
};
#define	Uart1_InitTypeSet	Uart1_InitTypeSet
#define	Uart2_InitTypeSet	Uart1_InitTypeSet
#define	Uart3_InitTypeSet	Uart1_InitTypeSet

UART_TOTAL_InitTypeDef	UartBase[3]={

{
	UART1,
	(UART_InitTypeDef*)&Uart1_InitTypeSet,
},

{
	UART2,
	(UART_InitTypeDef*)&Uart2_InitTypeSet,
},
{
	UART3,
	(UART_InitTypeDef*)&Uart3_InitTypeSet,
},

};

UART_HandleTypeDef UARTHandle[3];






void UARTx_Config(UART_HandleTypeDef* uartx,UART_TOTAL_InitTypeDef* uartBase);
__weak		void Error_Handler(void)
{}	

//void    API_UART_SetDmaRxHandle(uint8_t ch,uint32_t * handle)
//{
//    UARTHandle[ch].hdmarx=handle;
//}
//void    API_UART_SetDmaTxHandle(uint8_t ch,uint32_t * handle)
//{
//    UARTHandle[ch].hdmatx=handle;
//}

void    API_UART_TxOver_IRQHandler(uint8_t ch)
{
#ifdef	COMM_UART	
     HAL_DMA_IRQHandler(UARTHandle[2].hdmatx);
#else
     HAL_DMA_IRQHandler(UARTHandle[1].hdmatx);
#endif
	
}



void API_UART_Init(uint8_t ch,uint32_t * rxHandle,uint32_t* txHandle)
{


		UARTx_Config(&UARTHandle[ch],&UartBase[ch]);
    UARTHandle[ch].hdmarx=(DMA_HandleTypeDef*)rxHandle;
    UARTHandle[ch].hdmatx=(DMA_HandleTypeDef*)txHandle; 
		UARTHandle[ch].hdmatx->Parent=&UARTHandle[ch];

}	

void	API_UART_DMA_SendValue(uint8_t ch, uint8_t * value,uint16_t size)
{
		if(HAL_UART_Transmit_DMA(&UARTHandle[ch],value,size))
		{
		        Error_Handler();
		}
}	

void	API_UART_DMA_ReadValue(uint8_t ch, uint8_t * value,uint16_t size)
{
		if(HAL_UARTEx_ReceiveToIdle_DMA(&UARTHandle[ch],value,size))
		{
		        Error_Handler();
		}
}

void UARTx_Config(UART_HandleTypeDef* uartx,UART_TOTAL_InitTypeDef* uartBase)
{
    uartx->Instance        = uartBase->Instance;

    uartx->Init=*(uartBase->Init);
    if(HAL_UART_Init(uartx) != HAL_OK)
    {
        Error_Handler();
    }
}




/**
  * @brief  UART error callbacks
  * @param  UARTHandle: UART handle
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *UARTHandle)
{

}





void HAL_UART_MspInit(UART_HandleTypeDef *hUART)
{
    GPIO_InitTypeDef  GPIO_InitStruct;
        
    if(hUART->Instance == UART1)
    {
        __HAL_RCC_UART1_CLK_ENABLE();
//        __HAL_RCC_GPIOA_CLK_ENABLE();
//        __HAL_RCC_DMA1_CLK_ENABLE();
//        __HAL_RCC_DMAMUX1_CLK_ENABLE();
//        
//        hdma1.Instance                 = DMA1_Channel1;
//        hdma1.Init.Request             = DMA_REQUEST_UART1_TX;
//        hdma1.Init.Direction           = DMA_MEMORY_TO_PERIPH;
//        hdma1.Init.PeriphInc           = DMA_PINC_DISABLE;
//        hdma1.Init.MemInc              = DMA_MINC_ENABLE;
//        hdma1.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
//        hdma1.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
//        hdma1.Init.Mode                = DMA_NORMAL;
//        hdma1.Init.Priority            = DMA_PRIORITY_VERY_HIGH;
//        hdma1.Parent                   = &UART1Handle;
//        
//        HAL_DMA_Init(&hdma1);
//        
//        GPIO_InitStruct.Pin       = GPIO_PIN_9;
//        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
//        GPIO_InitStruct.Pull      = GPIO_PULLUP;
//        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
//        GPIO_InitStruct.Alternate = GPIO_AF7;
//        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
//        
        HAL_NVIC_SetPriority(UART1_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(UART1_IRQn);
//    
//        HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 1, 1);
//        HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
    }
    else if(hUART->Instance == UART2)
    {
        __HAL_RCC_UART2_CLK_ENABLE();

  			__HAL_UART_ENABLE_IT(&UARTHandle[Uart2],UART_IT_IDLE);  
        HAL_NVIC_SetPriority(UART2_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(UART2_IRQn);
    
//        HAL_NVIC_SetPriority(DMA2_Channel2_IRQn, 1, 1);
//        HAL_NVIC_EnableIRQ(DMA2_Channel2_IRQn);

		}
		if(hUART->Instance == UART3)
    {
        __HAL_RCC_UART3_CLK_ENABLE();
		
				__HAL_UART_ENABLE_IT(&UARTHandle[Uart3],UART_IT_IDLE);


        HAL_NVIC_SetPriority(UART3_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(UART3_IRQn);
  
    }
    if(hUART->Instance == UART4)
    {
        __HAL_RCC_UART4_CLK_ENABLE();		
		
        HAL_NVIC_SetPriority(UART4_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(UART4_IRQn);		
		
	}
}

/**
  * @brief UART MSP De-Initialization
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO configuration to their default state
  * @param hUART: UART handle pointer
  * @retval None
  */
void HAL_UART_MspDeInit(UART_HandleTypeDef *hUART)
{
    /*##-1- Reset peripherals ##################################################*/
    __HAL_RCC_UART3_FORCE_RESET();
    __HAL_RCC_UART3_RELEASE_RESET();    
    __HAL_RCC_UART2_FORCE_RESET();
    __HAL_RCC_UART2_RELEASE_RESET();
    __HAL_RCC_UART1_FORCE_RESET();
    __HAL_RCC_UART1_RELEASE_RESET();
    
//    /*##-2- Disable peripherals and GPIO Clocks #################################*/
//    /* Configure UART1 Tx as alternate function  */
//    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9);
//    /* Configure UART1 Rx as alternate function  */
//    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_15);
}


void	API_UART_IRQHandler(void)
{
	__HAL_UART_CLEAR_FLAG(&UARTHandle[UARTX],UART_FLAG_RXNE);

}	

#if 0
int fputc(int ch, FILE *fp)
{
    if(IS_UART_INSTANCE(UART_SEL))
    {

    }

}
#endif


void	API_UART_RxIdle_IRQHandler(void)
{
		  HAL_UART_IRQHandler(&UARTHandle[UARTX]);

}	


void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    API_UART_RxEventCallback( Size);

}



void API_UART_ExportToCsv(int16_t *s_array,int16_t *d_array, uint16_t length)
{
	#if 0
    printf("Index,SourceValue,DestValue\r\n");  // CSV±ęĚâ
    for (uint16_t i = 0; i < length; i++) {
        printf("%d,%d,%d,\r\n", i, s_array[i],d_array[i]);
    }
	#endif
}
void API_UART_ExportToCsv32(uint32_t *s_array,uint32_t *d_array, uint32_t *d1_array,uint16_t length)
{
		#if 1
    printf("Index,SourceValue32,DestValue32\r\n");  // CSV±ęĚâ
    for (uint16_t i = 0; i < length; i++) {
        printf("%d,%d,%d,%d\r\n", i, s_array[i],d_array[i],d1_array[i]);
    }
		#endif
}
#if 0
#include 	<stdio.h>
#include	<stdarg.h>
#include	<string.h>

typedef struct         //IHÖ÷°ĺÍ¨Ń¶ĎÂ·˘żŘÖĆÄÚČÝ˝áąąĚĺ
{
    uint16_t point;						//¶ÓÎ˛Ö¸Őë
    char  value[TxBuffSize];            //°ëą¦ÂĘ×´Ě¬Ł¬¸ß4Î»ĘÇ×ÜĘ±ĽäŁ¬µÍËÄÎ»ĘÇĽÓČČĘ±Ľä
}UartBuffDef;		


typedef struct         //IHÖ÷°ĺÍ¨Ń¶ĎÂ·˘żŘÖĆÄÚČÝ˝áąąĚĺ
{
	uint8_t	buffNum;			//·ÖłÉÁ˝¸ö¶ÓÁĐ
	UartBuffDef	Buff[2];		//
	
}UartBuffTxDef;

UartBuffTxDef	UartTxBuff;



void cprintf(const char *fmt,...)			//´ňÓˇµ˝»ş´ć		//UART1 DMA´«Ęä
{
#if 1
	char*	TxAddress;				//TxBuffÖ¸Őë
	uint16_t	EndPoint;			//Î˛Ö¸Őë
	
	va_list args;
	__va_start(args,fmt);
	
	TxAddress=UartTxBuff.Buff[UartTxBuff.buffNum].value;
	EndPoint=UartTxBuff.Buff[UartTxBuff.buffNum].point;
	
	TxAddress+=EndPoint;

	vsnprintf(TxAddress,TxBuffSize,fmt,args);
	__va_end(args);
	
	EndPoint=strlen(TxAddress);		//ÔöĽÓµÄĘýľÝł¤¶Č
	UartTxBuff.Buff[UartTxBuff.buffNum].point+=EndPoint;
#endif
	//UART_DMA_Send_u8(UartUser,TxBuff.value+TxBuff.point,TxBuff.point);			//Íů»ş´ćÇř¶ÓÎ˛ÔöĽÓ
}	


void	DlgTxSend(void)
{
	uint8_t*	TxAddress;
	uint16_t	len;
	TxAddress=(uint8_t*)UartTxBuff.Buff[UartTxBuff.buffNum].value;
	len=UartTxBuff.Buff[UartTxBuff.buffNum].point;
	if(len)													//ÓĐĘýľÝĐčŇŞ´ňÓˇ
	{	
//		UART_DMA_Send_u8(UartUser,TxAddress,len);
	}	
	UartTxBuff.Buff[UartTxBuff.buffNum].point=0;			//´ňÓˇÍęłÉ
	
	
}
#endif



