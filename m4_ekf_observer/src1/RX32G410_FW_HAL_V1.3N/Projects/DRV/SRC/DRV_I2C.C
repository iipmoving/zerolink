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
#include	"drv_i2c.h"
#include	"DRV_GPIO.H"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define I2C_ADDRESS        0x50

/* I2C SPEEDCLOCK define to max value: 400 KHz on R2265*/
#define I2C_SPEEDCLOCK     400000
#define I2C_DUTYCYCLE      I2C_DUTYCYCLE_2

GPIO_BaseInitTypeDef I2C1_SCL={
        
        I2C1_GPIO_PORT,
        
        {
            I2C1_SCL_PIN,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_AF_OD,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PULLUP,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            I2C1_GPIO_AF,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
    };

GPIO_BaseInitTypeDef I2C1_SDA={
        
        I2C1_GPIO_PORT,
        
        {
            I2C1_SDA_PIN,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_AF_OD,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PULLUP,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            I2C1_GPIO_AF,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
    };

GPIOsetGroupDef	 I2C1_Gpio={
        1,
        sizeof(int)*2,
        0,
        0,  
        &I2C1_SCL,	/*int	channel;*/
        &I2C1_SDA,
};	



typedef struct                          //与TIMX对应的差别项，
{
    uint8_t  		timch;                  //与TIMX对应的序号
    uint8_t         address;               	//地址
    uint8_t         master;                 //主从
    uint8_t         res3;                   //保留
    I2C_TypeDef*    I2Cx;               	//TIM1,~17   
	int	            channel;                //输出的通道    @arg TIM_CHANNEL_1: TIM Channel 1 selected
	IRQn_Type	    irqx;                   //中断号


}I2C_setDef;	


typedef struct                      //完整TIM设置集合
{
		I2C_setDef*									set;                 //I2C差异点设置 
		I2C_InitTypeDef*							init;               	//定时器时基设置 
		uint32_t*									readBuff;
		uint32_t*									sendBuff;	
		GPIOsetGroupDef*							gpioGroup;				//TIM GPIO设置
		I2C_HandleTypeDef	handle;
	
}I2cStrDef;	



I2cStrDef	I2c1Str;

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


#if 0
I2C_HandleTypeDef I2c1Handle={

  /*I2C_TypeDef                *Instance;      !< I2C registers base address               */

  /*I2C_InitTypeDef            Init;           !< I2C communication parameters             */

  /*uint8_t                    *pBuffPtr;      !< Pointer to I2C transfer buffer           */

  /*uint16_t                   XferSize;       !< I2C transfer size                        */

  /*__IO uint16_t              XferCount;      !< I2C transfer counter                     */

  /*__IO uint32_t              XferOptions;    !< I2C transfer options                     */

  __IO uint32_t              PreviousState;  /*!< I2C communication Previous state and mode
                                                  context for internal usage               */

  DMA_HandleTypeDef          *hdmatx;        /*!< I2C Tx DMA handle parameters             */

  DMA_HandleTypeDef          *hdmarx;        /*!< I2C Rx DMA handle parameters             */

  HAL_LockTypeDef            Lock;           /*!< I2C locking object                       */

  __IO HAL_I2C_StateTypeDef  State;          /*!< I2C communication state                  */

  __IO HAL_I2C_ModeTypeDef   Mode;           /*!< I2C communication mode                   */

  __IO uint32_t              ErrorCode;      /*!< I2C Error code                           */

  __IO uint32_t              Devaddress;     /*!< I2C Target device address                */

  __IO uint32_t              Memaddress;     /*!< I2C Target memory address                */

  __IO uint32_t              MemaddSize;     /*!< I2C Target memory address  size          */

  __IO uint32_t              EventCount;     /*!< I2C Event counter                        */


#if (USE_HAL_I2C_REGISTER_CALLBACKS == 1)
  void (* MasterTxCpltCallback)(struct __I2C_HandleTypeDef *hi2c);           /*!< I2C Master Tx Transfer completed callback */
  void (* MasterRxCpltCallback)(struct __I2C_HandleTypeDef *hi2c);           /*!< I2C Master Rx Transfer completed callback */
  void (* SlaveTxCpltCallback)(struct __I2C_HandleTypeDef *hi2c);            /*!< I2C Slave Tx Transfer completed callback  */
  void (* SlaveRxCpltCallback)(struct __I2C_HandleTypeDef *hi2c);            /*!< I2C Slave Rx Transfer completed callback  */
  void (* ListenCpltCallback)(struct __I2C_HandleTypeDef *hi2c);             /*!< I2C Listen Complete callback              */
  void (* MemTxCpltCallback)(struct __I2C_HandleTypeDef *hi2c);              /*!< I2C Memory Tx Transfer completed callback */
  void (* MemRxCpltCallback)(struct __I2C_HandleTypeDef *hi2c);              /*!< I2C Memory Rx Transfer completed callback */
  void (* ErrorCallback)(struct __I2C_HandleTypeDef *hi2c);                  /*!< I2C Error callback                        */
  void (* AbortCpltCallback)(struct __I2C_HandleTypeDef *hi2c);              /*!< I2C Abort callback                        */

  void (* AddrCallback)(struct __I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode);  /*!< I2C Slave Address Match callback */

  void (* MspInitCallback)(struct __I2C_HandleTypeDef *hi2c);                /*!< I2C Msp Init callback                     */
  void (* MspDeInitCallback)(struct __I2C_HandleTypeDef *hi2c);              /*!< I2C Msp DeInit callback                   */

#endif  /* USE_HAL_I2C_REGISTER_CALLBACKS */


};

#endif

I2C_HandleTypeDef I2c1Handle;



/* Buffer used for transmission */
uint8_t aTxBuffer[] = " ****I2C_OneBoards communication based on IT****  ****I2C_OneBoards communication based on IT****  ****I2C_OneBoards communication based on IT**** ";

/* Buffer used for reception */
uint8_t aRxBuffer[RXBUFFERSIZE];

/* Private function prototypes -----------------------------------------------*/
static uint16_t Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength);
void HAL_BSP_PB_Callback(uint32_t GPIO_Pin);
void Error_Handler(void);

/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/*
 * main: initialize and start the system
 */
 
#define	 I2cxHandle	i2cxStr->handle	
 
void DrvI2cInit (I2cStrDef* i2cxStr)
{
    
    // output the contect of message
    printf("I2C_OneBoard_ComIT... \r\n");

    
    /*##-1- Configure the I2C peripheral ######################################*/
	
	
    I2cxHandle.Instance        	= 	i2cxStr->set->I2Cx;;
	I2cxHandle.Init				=	*(i2cxStr->init);
	


    if(HAL_I2C_Init(&I2cxHandle) != HAL_OK)
    {
        /* Initialization Error */
        Error_Handler();
    }


    
    /*##-1- Put I2C2 peripheral in reception process ###########################*/
    if (HAL_I2C_Slave_Receive_IT(&I2cxHandle, (uint8_t *)aRxBuffer, RXBUFFERSIZE) != HAL_OK)
    {
        /* Transfer error in reception process */
        Error_Handler();
    }

    printf("I2C2 is prepare to receive data... \r\n");

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
    
    if (hi2c->Instance == I2C1)
    {
        /*##-1- Enable peripherals and GPIO Clocks #################################*/
        /*## Enable peripherals and GPIO Clocks #################################*/
        I2Cx_GPIO_CLK_ENABLE();
        I2C1_CLK_ENABLE();
        I2C1_FORCE_RESET();
        I2C1_RELEASE_RESET();
        
        /*##-2- Configure peripheral GPIO ##########################################*/
        /* I2C1 TX/RX GPIO pin configuration  */
		HAL_GPIO_BaseInit(&I2C1_SDA);
		HAL_GPIO_BaseInit(&I2C1_SCL);
        
        /*##-3- Configure the NVIC for I2C ########################################*/
        /* NVIC for I2Cx */
        HAL_NVIC_SetPriority(I2C1_ER_IRQn, 0, 2);
        HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
        
        HAL_NVIC_SetPriority(I2C1_EV_IRQn, 0, 1);
        HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
    }
        
    else if (hi2c->Instance == I2C2)
    {
        /*##-1- Enable peripherals and GPIO Clocks #################################*/
        /*## Enable peripherals and GPIO Clocks #################################*/   
        I2Cx_GPIO_CLK_ENABLE();
        I2C2_CLK_ENABLE();
        I2C2_FORCE_RESET();
        I2C2_RELEASE_RESET();
        
        /*##-2- Configure peripheral GPIO ##########################################*/
        /* I2C1 TX/RX GPIO pin configuration  */

        
        /*##-3- Configure the NVIC for I2C ########################################*/
        /* NVIC for I2Cx */
        HAL_NVIC_SetPriority(I2C2_ER_IRQn, 0, 2);
        HAL_NVIC_EnableIRQ(I2C2_ER_IRQn);
    
        HAL_NVIC_SetPriority(I2C2_EV_IRQn, 0, 1);
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

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{

}



#include	"commClass.H"


enum
{
		I2C_WAIT_ADDR=0,			//等待地址
		I2C_READ_VALUE=1,			//读取数据
		I2C_SEND_VALUE=2,			//发送数据
};	


UserStringDef*	rxbuff;
UserStringDef*	txbuff;

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

#define	 I2c1Handle	I2c1Str.handle	



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

void I2C1_IRQHandler(void)
{
	
	static		uint8_t		I2cStatus=I2C_WAIT_ADDR;
	uint8_t*	rxDataBuff=(uint8_t*)&(rxbuff->p);
	uint8_t*	txDataBuff=(uint8_t*)&(txbuff->p);
	uint8_t		bf[16];

	
	I2cIrqCn++;

	
  /* Check ADDR flag value in ISR register */
  if(__HAL_I2C_GET_FLAG(&I2c1Handle, I2C_FLAG_ADDR)) {
		
		__HAL_I2C_CLEAR_FLAG(&I2c1Handle,I2C_FLAG_ADDR);
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
	}		
	if(__HAL_I2C_GET_FLAG(&I2c1Handle,I2C_FLAG_STOPF))
	{

			rxbuff->len=rxbuff->res;				//数据长度
			rxbuff->num++;
			memcpy(bf,rxDataBuff,rxbuff->len);
			I2cStatus=I2C_WAIT_ADDR;
			__HAL_I2C_CLEAR_FLAG(&I2c1Handle,I2C_FLAG_STOPF);
			__HAL_I2C_ENABLE_IT(&I2c1Handle,I2C_CR2_ITBUFEN);
//			RtosFunTable[I2cSlaveIrqFun].Function();				//回调函数，由用户实现，需要接收到数据立即执行时使用
		
	}		
	if(__HAL_I2C_GET_FLAG(&I2c1Handle,I2C_FLAG_AF))
	{

			I2cStatus=I2C_WAIT_ADDR;					//ACK回应失败，用于回读数据最后一个字节
			__HAL_I2C_CLEAR_FLAG(&I2c1Handle,I2C_FLAG_AF);
			__HAL_I2C_ENABLE_IT(&I2c1Handle,I2C_CR2_ITBUFEN);

		
	}	
	if(__HAL_I2C_GET_FLAG(&I2c1Handle,I2C_FLAG_BERR))			//总线错误

	{
		
		__HAL_I2C_CLEAR_FLAG(&I2c1Handle,I2C_FLAG_BERR);			//恢复I2C总线
		
		I2C1_FORCE_RESET() ;            
		I2C1_RELEASE_RESET();  
		
//		I2C1_Slave_IT_Init();
//		I2C1_Stop();						//停止I2C 等待超时
	}	
	
		
	
	if(__HAL_I2C_GET_FLAG(&I2c1Handle,I2C_FLAG_TXE))	
	{
		__HAL_I2C_CLEAR_FLAG(&I2c1Handle,I2C_FLAG_TXE);
//		I2C_Transmit_Data8(I2C1,0);		//清TXE
//		I2C1_Stop();		
	}	
	
	if(I2cIrqCn>10)
	{
		I2cIrqCn=0;	
		I2C1_FORCE_RESET() ;            
		I2C1_RELEASE_RESET();  		
//		I2C1_Stop();						//关中断
	}		
	

	
	
}	



/******************************************************************************/
/*                  Peripherals Interrupt Handlers                            */
/*  Add the Interrupt Handler here for the used peripheral(s), for the        */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_rx32g4xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles I2C event interrupt request.
  * @param  None
  * @retval None
  * @Note   This function is redefined in "main.h" and related to I2C data transmission
  */
void I2C1_EV_IRQHandler(void)
{
    
}

void I2C2_EV_IRQHandler(void)
{
    
}

/**
  * @brief  This function handles I2C error interrupt request.
  * @param  None
  * @retval None
  * @Note   This function is redefined in "main.h" and related to I2C error
  */
void I2C1_ER_IRQHandler(void)
{
    
}

void I2C2_ER_IRQHandler(void)
{
    
}



