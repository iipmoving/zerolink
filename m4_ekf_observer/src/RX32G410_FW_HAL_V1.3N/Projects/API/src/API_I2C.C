/*********************************************************************************************
Copyright <2024> <Icore Technology (Nanjing)  Co.,Ltd>
All Rights Reserved,
Redistribution and use in source and binary forms, with or without modification, are permitted
provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this list of
conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or other materials provided
with the distribution.
3. Neither the name of the copyright holder nor the names of its contributors may be used to
endorse or promote products derived from this software without specific prior written
permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
OF THE POSSIBILITY OF SUCH DAMAGE.
**********************************************************************************************/

/*
*********************************************************************************************************
*                                              rx32g4xx
*                                           Library Function
*
*                                   Copyright 2024, RX Tech, Corp.
*                                        All Rights Reserved
*
*
* Project      : rx32g4xx
* File         : drv_i2c.c
* By           : moving
*********************************************************************************************************
*/


/* Includes ------------------------------------------------------------------*/
//#include "main.h"
#include "rx32g4xx_config_def.h"
#include "rx32g4xx_hal.h"
#include "system_init.h"
#include "system_bsp.h"


#include "rx32g4xx_hal_i2c.h"
#include	"API_i2c.h"
#include	"API_GPIO.H"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/




#ifdef	I2C1_PORT
#define	I2C_Instance			I2C1
#define	I2C_AF

#else
#define	I2C_Instance			I2C2
#endif


#define I2C_ADDRESS        0x50

/* I2C SPEEDCLOCK define to max value: 400 KHz on R2265*/
#define I2C_SPEEDCLOCK     400000
#define I2C_DUTYCYCLE      I2C_DUTYCYCLE_2



__weak	void	API_I2C_GetValue(I2C_BUFF_DEF*	buff)
{
}	
__weak	void	API_I2C_SetValue(I2C_BUFF_DEF* buff)
{

}	


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

I2C_InitTypeDef		I2c1Init={
  I2C_SPEEDCLOCK,/*uint32_t ClockSpeed;       !< Specifies the clock frequency.
                                  This parameter must be set to a value lower than 400kHz */

  I2C_DUTYCYCLE,/*uint32_t DutyCycle;        !< Specifies the I2C fast mode duty cycle.
                                  This parameter can be a value of @ref I2C_duty_cycle_in_fast_mode */

  I2C_ADDRESS,/*uint32_t OwnAddress1;      !< Specifies the first device own address.
                                  This parameter can be a 7-bit or 10-bit address. */

  I2C_ADDRESSINGMODE_7BIT,/*uint32_t AddressingMode;   !< Specifies if 7-bit or 10-bit addressing mode is selected.
                                  This parameter can be a value of @ref I2C_addressing_mode */

  I2C_DUALADDRESS_DISABLE,/*uint32_t DualAddressMode;  !< Specifies if dual addressing mode is selected.
                                  This parameter can be a value of @ref I2C_dual_addressing_mode */

  0	,/*uint32_t OwnAddress2;      !< Specifies the second device own address if dual addressing mode is selected
                                  This parameter can be a 7-bit address. */

  I2C_GENERALCALL_DISABLE,/*uint32_t GeneralCallMode;  !< Specifies if general call mode is selected.
                                  This parameter can be a value of @ref I2C_general_call_addressing_mode */

  I2C_NOSTRETCH_DISABLE/*uint32_t NoStretchMode;    !< Specifies if nostretch mode is selected.
                                  This parameter can be a value of @ref I2C_nostretch_mode */

};



I2C_HandleTypeDef I2c1Handle;

UserStringDef*	rxbuff;
UserStringDef*	txbuff;

	


I2C_BUFF_DEF	RxBuff,TxBuff;

/* Buffer used for transmission */
uint8_t aTxBuffer[] = " ****I2C_OneBoards communication based on IT****  ****I2C_OneBoards communication based on IT****  ****I2C_OneBoards communication based on IT**** ";

/* Buffer used for reception */
uint8_t aRxBuffer[RXBUFFERSIZE];

/* Private function prototypes -----------------------------------------------*/
static uint16_t Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength);



void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c);

__weak void Error_Handler(void)
{

}	

/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/*
 * main: initialize and start the system
 */
 

 
void API_I2C_Init (void)
{
    
    // output the contect of message
//    printf("I2C_OneBoard_ComIT... \r\n");

	txbuff	=(UserStringDef*)MemNew(txbuff,I2C_BuffMax);				//初始化当前数据缓存（使用前需初始化)
	rxbuff	=(UserStringDef*)MemNew(rxbuff,I2C_BuffMax);				//初始化当前数据缓存（使用前需初始化)

	

    /*##-1- Configure the I2C peripheral ######################################*/
	
	
    I2c1Handle.Instance        	= 	I2C_Instance;;
	I2c1Handle.Init				=	I2c1Init;
	


    if(HAL_I2C_Init(&I2c1Handle) != HAL_OK)
    {
        /* Initialization Error */
        Error_Handler();
    }

    SET_BIT(I2c1Handle.Instance->CR1, I2C_CR1_ACK);
    __HAL_I2C_ENABLE_IT(&I2c1Handle, I2C_IT_EVT | I2C_IT_BUF | I2C_IT_ERR);    
    /*##-1- Put I2C2 peripheral in reception process ###########################*/
//    if (HAL_I2C_Slave_Receive_IT(&I2c1Handle, (uint8_t *)aRxBuffer, RXBUFFERSIZE) != HAL_OK)
//    {
//        /* Transfer error in reception process */
//        Error_Handler();
//    }

//RX32G410,DEBUG_POWER_OUTX,SimZero,I2C1_PORT,PrintMessage,Fmac9,

#ifdef	DEBUG_POWER_OUT
	
	printf("DEBUG_POWER_OUT is Power out by hart... \r\n");
#else
	printf("DEBUG_POWER_OUTx is Power out by soft... \r\n");
#endif	

#ifdef	SimZero
	
	printf("SimZero is zero out by soft... \r\n");
#else
	printf("SimZerox is Power out by hart... \r\n");
#endif	

#ifdef	I2C1_PORT
	
	printf("I2C1 is prepare to receive data... \r\n");

#endif	

#ifdef	I2C2_PORT
	
	printf("I2C2 is prepare to receive data... \r\n");
#endif

#ifdef	PrintMessage
	
	printf("PrintMessage is uart send debug message... \r\n");
#else
	printf("PrintMessagex is uart not send debug message... \r\n");
#endif	

#ifdef	Fmac9
	
	printf("Fmac9 is Fmac aFilterCoeffB is 9 Levle... \r\n");
#endif

#ifdef	Fmac5
	
	printf("Fmac5 is Fmac aFilterCoeffB is 5 Levle... \r\n");
#endif

}

/* Private functions ---------------------------------------------------------*/

/**
  * Initializes the Global MSP.
  */
void HAL_MspInit(void)
{
    /* USER CODE BEGIN MspInit 0 */
    
    /* USER CODE END MspInit 0 */
    
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

    /* USER CODE BEGIN MspInit 1 */

    /* USER CODE END MspInit 1 */
}







/**
  * @brief I2C MSP Initialization
  *        This function configures the hardware resources used in this example:
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration
  *           - DMA configuration for transmission request by peripheral
  *           - NVIC configuration for DMA interrupt request enable
  * @param hi2c: I2C handle pointer
  * @retval None
  */
void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
    

	GPIO_InitTypeDef  GPIO_InitStruct;
	
    if (hi2c->Instance == I2C1)
    {
        /*##-1- Enable peripherals and GPIO Clocks #################################*/
        /*## Enable peripherals and GPIO Clocks #################################*/
//        I2Cx_GPIO_CLK_ENABLE();
        I2C1_CLK_ENABLE();
        I2C1_FORCE_RESET();
        I2C1_RELEASE_RESET();
        
        /*##-2- Configure peripheral GPIO ##########################################*/
        /* I2C1 TX/RX GPIO pin configuration  */
//        GPIO_InitStruct.Pin       = I2C1_SCL_PIN | I2C1_SDA_PIN;
//        GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
//        GPIO_InitStruct.Pull      = GPIO_PULLUP;
//        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
//        GPIO_InitStruct.Alternate = I2C1_GPIO_AF;
//        HAL_GPIO_Init(I2C1_GPIO_PORT, &GPIO_InitStruct);
        
        /*##-3- Configure the NVIC for I2C ########################################*/
        /* NVIC for I2Cx */
        HAL_NVIC_SetPriority(I2C1_ER_IRQn, IRQ_PRIORITY_I2C1_ER, 2);
        HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
        
        HAL_NVIC_SetPriority(I2C1_EV_IRQn, IRQ_PRIORITY_I2C1_EV, 1);
        HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
    }
        
    else if (hi2c->Instance == I2C2)
    {
        /*##-1- Enable peripherals and GPIO Clocks #################################*/
        /*## Enable peripherals and GPIO Clocks #################################*/   
//        I2Cx_GPIO_CLK_ENABLE();
        I2C2_CLK_ENABLE();
        I2C2_FORCE_RESET();
        I2C2_RELEASE_RESET();
        
        /*##-2- Configure peripheral GPIO ##########################################*/
        /* I2C2 TX/RX GPIO pin configuration  */
//        GPIO_InitStruct.Pin       = I2C2_SCL_PIN | I2C2_SDA_PIN;
//        GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
//        GPIO_InitStruct.Pull      = GPIO_PULLUP;
//        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
//        GPIO_InitStruct.Alternate = I2C2_GPIO_AF;
//        HAL_GPIO_Init(I2C2_GPIO_PORT, &GPIO_InitStruct);
        
        /*##-3- Configure the NVIC for I2C ########################################*/
        /* NVIC for I2Cx */
        HAL_NVIC_SetPriority(I2C2_ER_IRQn, IRQ_PRIORITY_I2C2_ER, 2);
        HAL_NVIC_EnableIRQ(I2C2_ER_IRQn);
    
        HAL_NVIC_SetPriority(I2C2_EV_IRQn, IRQ_PRIORITY_I2C2_EV, 1);
        HAL_NVIC_EnableIRQ(I2C2_EV_IRQn);
    }
}




/**
  * @brief  I2C error callbacks.
  * @param  I2c1Handle: I2C handle
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *I2c1Handle)
{
    Error_Handler();
}




#include	"commClass.H"


enum
{
		I2C_WAIT_ADDR=0,			//等待地址
		I2C_READ_VALUE=1,			//读取数据
		I2C_SEND_VALUE=2,			//发送数据
};	




UserStringDef*	getI2cRxBuffAdr(void)
{
		return rxbuff;
}	

UserStringDef*	getI2cTxBuffAdr(void)
{
		return txbuff;
}	


uint32_t  sendvalue;
uint8_t		I2cIrqCn;			//I2C中断产生次数
static		uint8_t		I2cStatus=I2C_WAIT_ADDR;



__STATIC_INLINE void __HAL_I2C_Transmit_Data8(I2C_HandleTypeDef *I2C_handle, uint8_t Data)
{
  MODIFY_REG(I2C_handle->Instance->DR, I2C_DR_DR, Data);
}

__STATIC_INLINE uint8_t __HAL_I2C_Receive_Data8(I2C_HandleTypeDef *I2C_handle)
{
  return (uint8_t)(READ_BIT(I2C_handle->Instance->DR, I2C_DR_DR));
}
__STATIC_INLINE uint32_t __HAL_I2C_Get_TransferDirection(I2C_HandleTypeDef *I2C_handle)
{
  return (uint32_t)(READ_BIT(I2C_handle->Instance->SR2, I2C_SR2_TRA));
}

void API_I2C1_EV_IRQHandler(void)
{
	


	uint8_t		recive[16];			//读取到的数据（方便调试
//	I2C_BUFF_DEF*	rxbuff=&RxBuff;
//	I2C_BUFF_DEF*	txbuff=&TxBuff;	
//	uint8_t*	rxDataBuff=(uint8_t*)&(rxbuff->buff);
//	uint8_t*	txDataBuff=(uint8_t*)&(txbuff->buff);


	uint8_t*	rxDataBuff=(uint8_t*)&(rxbuff->p);
	uint8_t*	txDataBuff=(uint8_t*)&(txbuff->p);



	
//	I2cIrqCn++;

	
  /* Check ADDR flag value in ISR register */
  if(__HAL_I2C_GET_FLAG(&I2c1Handle, I2C_FLAG_ADDR)) {
		
//		__HAL_I2C_CLEAR_FLAG(&I2c1Handle,I2C_FLAG_ADDR);
	  
	  __HAL_I2C_CLEAR_ADDRFLAG(&I2c1Handle);
	  
		__HAL_I2C_ENABLE_IT(&I2c1Handle,I2C_CR2_ITBUFEN);
    /* Verify the slave transfer direction, a write direction, slave enters transmitter mode */
    if(__HAL_I2C_Get_TransferDirection(&I2c1Handle) == I2C_SR2_TRA) {
      /* Enable buffer interrupts */
			I2cStatus=I2C_SEND_VALUE;
      /* Clear ADDR flag value in ISR register */
			txbuff->res=0;
    }
    else {
      /* Clear ADDR flag value in ISR register */
			I2cStatus=I2C_READ_VALUE;
			//----------回送第一个数据--------------------------------------
			rxbuff->res=0;
      /* Error */
    }
  }
	
	
	if(__HAL_I2C_GET_FLAG(&I2c1Handle,I2C_FLAG_RXNE))
	{

			if(I2cStatus==I2C_READ_VALUE)
			{	
					rxDataBuff[rxbuff->res]=__HAL_I2C_Receive_Data8(&I2c1Handle);
					rxbuff->res++;
					if(rxbuff->res>rxbuff->max)
					{	
							rxbuff->res=0;
					}
			}
			else
			{
				
				API_GPIO_WritePin(DebugB_pin,1);

				I2cStatus=I2C_WAIT_ADDR;

					__HAL_I2C_DISABLE_IT(&I2c1Handle,I2C_CR2_ITBUFEN);

					SET_BIT(I2c1Handle.Instance->CR1, I2C_CR1_ACK);			 
					__HAL_I2C_ENABLE_IT(&I2c1Handle,I2C_IT_EVT|I2C_IT_ERR); 


					rxbuff->res=0;
					rxDataBuff[rxbuff->res]=__HAL_I2C_Receive_Data8(&I2c1Handle);
			}
			
	}	
	
			if(__HAL_I2C_GET_FLAG(&I2c1Handle,I2C_FLAG_TXE))
			{
				if(I2cStatus==I2C_SEND_VALUE)
				{	
					
					__HAL_I2C_Transmit_Data8(&I2c1Handle,txDataBuff[txbuff->res]);
					txbuff->res++;
					if(txbuff->res>txbuff->max)
					{
							txbuff->res=0;
					}
				}else
				{
					API_GPIO_WritePin(DebugB_pin,1);
					I2cStatus=I2C_WAIT_ADDR;
					
					__HAL_I2C_DISABLE_IT(&I2c1Handle,I2C_CR2_ITBUFEN);
					SET_BIT(I2c1Handle.Instance->CR1, I2C_CR1_ACK);			 
					__HAL_I2C_ENABLE_IT(&I2c1Handle,I2C_IT_EVT|I2C_IT_ERR); 

					txbuff->res=0;
					__HAL_I2C_Transmit_Data8(&I2c1Handle,txDataBuff[txbuff->res]);
				}
				
			}				
	
	
	
#if 0	
	switch(I2cStatus)
	{
		case	I2C_READ_VALUE:
			if(__HAL_I2C_GET_FLAG(&I2c1Handle,I2C_FLAG_RXNE))
			{

					rxDataBuff[rxbuff->res]=__HAL_I2C_Receive_Data8(&I2c1Handle);
					rxbuff->res++;
					if(rxbuff->res>rxbuff->max)
					{	
							rxbuff->res=0;
					}	
			}	
		break;


		case	I2C_SEND_VALUE:
			if(__HAL_I2C_GET_FLAG(&I2c1Handle,I2C_FLAG_TXE))
			{

					__HAL_I2C_Transmit_Data8(&I2c1Handle,txDataBuff[txbuff->res]);
					txbuff->res++;
					if(txbuff->res>txbuff->max)
					{
							txbuff->res=0;
					}			
			}			
		
		break;
		case	I2C_WAIT_ADDR:			//通讯错

			  __HAL_I2C_CLEAR_ADDRFLAG(&I2c1Handle);
		I2C1_FORCE_RESET() ;            
		I2C1_RELEASE_RESET();  
			
		break;	
	}	
#endif
	
	
	
	if(__HAL_I2C_GET_FLAG(&I2c1Handle,I2C_FLAG_STOPF))
	{	  		
		
		  __HAL_I2C_DISABLE_IT(&I2c1Handle, I2C_IT_EVT | I2C_IT_BUF | I2C_IT_ERR);

			/* Clear STOPF flag */
			__HAL_I2C_CLEAR_STOPFLAG(&I2c1Handle);

			/* Disable Acknowledge */
			CLEAR_BIT(I2c1Handle.Instance->CR1, I2C_CR1_ACK);
		

		
	
			rxbuff->len=rxbuff->res;				//数据长度
			rxbuff->num++;
			memcpy(recive,rxDataBuff,rxbuff->len);				 


			I2cStatus=I2C_WAIT_ADDR;
//			__HAL_I2C_DISABLE_IT(&I2c1Handle,I2C_CR2_ITBUFEN);
		
			SET_BIT(I2c1Handle.Instance->CR1, I2C_CR1_ACK);			 
			__HAL_I2C_ENABLE_IT(&I2c1Handle,I2C_IT_EVT|I2C_IT_ERR); 
//			RtosFunTable[I2cSlaveIrqFun].Function();				//回调函数，由用户实现，需要接收到数据立即执行时使用
//			API_I2C_GetValue(rxDataBuff);
			

	}		

		
	
//	if(__HAL_I2C_GET_FLAG(&I2c1Handle,I2C_FLAG_TXE))	
//	{
//		__HAL_I2C_CLEAR_FLAG(&I2c1Handle,I2C_FLAG_TXE);
//		__HAL_I2C_Transmit_Data8(&I2c1Handle,0);		//清TXE
////		I2C1_Stop();		
//	}	
	

	
//	if(I2cIrqCn>10)
//	{
//		I2cIrqCn=0;	
//		I2C1_FORCE_RESET() ;            
//		I2C1_RELEASE_RESET();  		
////		I2C1_Stop();						//关中断
//	}		

	API_GPIO_WritePin(DebugB_pin,0);
}	



void	API_I2C1_ER_IRQHandler(void)
{
//	HAL_I2C_ER_IRQHandler(&I2c1Handle);
#if 1


	
	if(__HAL_I2C_GET_FLAG(&I2c1Handle,I2C_FLAG_AF))
	{
		
			I2cStatus=I2C_WAIT_ADDR;					//ACK回应失败，用于回读数据最后一个字节
			__HAL_I2C_CLEAR_FLAG(&I2c1Handle,I2C_FLAG_AF);
			__HAL_I2C_DISABLE_IT(&I2c1Handle,I2C_CR2_ITBUFEN);
		
		
	}

	if(__HAL_I2C_GET_FLAG(&I2c1Handle,I2C_FLAG_BERR))			//总线错误

	{
	API_GPIO_WritePin(DebugB_pin,1);

	
		__HAL_I2C_CLEAR_FLAG(&I2c1Handle,I2C_FLAG_BERR);			//恢复I2C总线

    /* Workaround: Start cannot be generated after a misplaced Stop */
		SET_BIT(I2c1Handle.Instance->CR1, I2C_CR1_SWRST);	
		
		I2C1_FORCE_RESET() ;            
		I2C1_RELEASE_RESET();  

		__HAL_I2C_DISABLE_IT(&I2c1Handle,I2C_CR2_ITBUFEN);
		SET_BIT(I2c1Handle.Instance->CR1, I2C_CR1_ACK);			 
		__HAL_I2C_ENABLE_IT(&I2c1Handle,I2C_IT_EVT|I2C_IT_ERR); 
					txbuff->res=0;
					rxbuff->res=0;
		

	}	
	I2cStatus=I2C_WAIT_ADDR;					//ACK回应失败，用于回读数据最后一个字节

#endif

	API_GPIO_WritePin(DebugB_pin,0);	
}	

void  API_I2C_CheckBuffMax(void)
{
    if(rxbuff->max!=I2C_BuffMax)
    {
				rxbuff->max=I2C_BuffMax;
//        API_GPIO_WritePin(DebugA_pin,1);
//        API_GPIO_WritePin(DebugA_pin,0);
    }

}


