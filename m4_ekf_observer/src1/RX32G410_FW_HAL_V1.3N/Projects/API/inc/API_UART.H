#ifndef __API_UART_H__
#define __API_UART_H__


#include	<stdint.h>

#define	Uart1BaudRate	115200

enum
{
		Uart1=0,
		Uart2,
		Uart3,
		UartMax,
};	
#define	TxBuffSize		1024


#ifdef    COMM_UART

    #define     UARTX       Uart3		//???

#else


    #define     UARTX      Uart2		//????

#endif



void API_UART_Init(uint8_t ch,uint32_t * rxHandle,uint32_t *txHandle);	

// void    API_UART_SetDmaRxHandle(uint8_t ch,uint32_t * handle);	//????DMA HANDLE
// void    API_UART_SetDmaTxHandle(uint8_t ch,uint32_t * handle);

void	API_UART_DMA_SendValue(uint8_t ch, uint8_t * value,uint16_t size);
void	API_UART_DMA_ReadValue(uint8_t ch, uint8_t * value,uint16_t size);

void API_UART_ExportToCsv(int16_t *s_array,int16_t *d_array, uint16_t length);//输出CSV格式 
void API_UART_ExportToCsv32(uint32_t *s_array,uint32_t *d_array, uint32_t *d1_array,uint16_t length);



#endif			/*__API_UART_H__*/
