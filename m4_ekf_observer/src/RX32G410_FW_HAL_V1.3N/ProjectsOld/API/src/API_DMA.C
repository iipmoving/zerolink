/*
DMA配置说明：
T1A,T2A,T3A,T4A,采用TIM3 1.3US触发DMA,
VC, HRTIM,同步采用TIM3 1.3US触发
PAN在检锅时也是采用TIM3 1.3US触发
T12A,T34A采用ADC1 触发，用于ADC WDG检测
UARTRX UARTTX 分别采用UART RX TX 触发
M2M 为数据保存用 
FMAC_PREO FMAC_WRITE占用两个DMA(PREO 可以用M2M WRITE 可以用PAN)
*/



#include "rx32g4xx_config_def.h"
#include "rx32g4xx_hal.h"
#include "system_init.h"
#include "system_bsp.h"

#include 	"api_DMA.h"
#include	"API_gpio.h"


#define		DMA_REQUEST_TXA_UP			DMA_REQUEST_TIM3_UP

#define		DMA_REQUEST_PAN_UP			DMA_REQUEST_TIM7_UP


__weak	void	API_DMA_TxA_IRQHandlerCallBack(uint8_t	num)	// DMA中断
{
}
__weak	void	API_DMA_PAN_IRQHandlerCallBack(uint8_t	num)	// DMA中断
{
}
__weak	void	API_DMA_T1A_IRQHandlerCallBack(uint8_t	num)
{}
__weak	void	Error_Handler(void)
{}	
__weak	void	API_ADC_DMA_M2M_IRQHandlerCallBack(void)	//M2M DMA中断(這個沒用)
{
}
__weak	void	API_DMA_M2M_OverCallback(void)	//M2M DMA中断
{
}


//==========100us常量设置==================================
//============================================
//==========4us常量设置==================================

#define	DMA_Channel_CurrentAdc2			DMA1_Channel1		//use
#define	DMA_Channel_CurrentAdc3			DMA1_Channel3		//use		//以1US為觸發，獲取HRTIM的COUNT值 用于同步1 
#define	DMA_Channel_M2M				DMA1_Channel4		//use	 //內存復制
#define	DMA_Channel_FMAC_PREO		DMA1_Channel5			 //fmac
#define	DMA_Channel_FMAC_WRITE		DMA1_Channel6			 //fmac



#define	DMA_Channel_Pan		DMA2_Channel2		//use
#define	DMA_Channel_VcAdc1		DMA2_Channel3		//use		//以1US為觸發，獲取HRTIM的COUNT值 用于同步1 

#define	DMA_Channel_FMAC_READ		DMA2_Channel6	


#define	DMA_Channel_UART_RX	DMA2_Channel5			//以ADC1為觸發，獲取HRTIM的COUNT值 
#define	DMA_Channel_UART_TX	DMA2_Channel4			//以ADC1為觸發，獲取HRTIM的COUNT值 

#define	DMA_Channel_HrtimPotCh1		DMA1_Channel7	//1_7
#define	DMA_Channel_HrtimPotCh2		DMA1_Channel2	//2_1

#define	DMA_Channel_HrtimPotCh3		DMA2_Channel7	//1_7
#define	DMA_Channel_HrtimPotCh4		DMA2_Channel1	//2_1

//unuse
//DMA1_Channel8
//DMA2_Channel3
//DMA2_Channel8


#define		DMA_REQUEST_TIM_HRTIM_PotCh1			DMA_REQUEST_TIM3_CH1		//TXA 分成两个组， 每个通道使用两个源，一个采AD 一个采HRTIM
#define		DMA_REQUEST_TIM_HRTIM_PotCh2			DMA_REQUEST_TIM3_CH2
#define		DMA_REQUEST_TIM_HRTIM_PotCh3			DMA_REQUEST_TIM3_CH3
#define		DMA_REQUEST_TIM_HRTIM_PotCh4			DMA_REQUEST_TIM3_CH4



// #define	DMA_Channel_TXA_ADC2		DMA2_Channel7			//T2A ADC 值 0.5us采集一次
// #define	DMA_Channel_TXA_HRTIM2		DMA2_Channel8		// T2A	HRTIM 值 0.5us采集一次




enum
{

	
	DMA_IRQ_M2M=				DMA1_Channel4_IRQn,			
	DMA_IRQ_PRIORITY_M2M=		IRQ_PRIORITY_DMA1_Channel4,	
//	DMA_IRQHandler_M2M=				API_DMA1_Channel4_IRQHandler,
	
	DMA_IRQ_M2Mlow=				DMA2_Channel3_IRQn,			
	DMA_IRQ_PRIORITY_M2Mlow=		IRQ_PRIORITY_DMA2_Channel3,	

	DMA_FLAG_TC_CurrentAdc2=DMA_FLAG_TC1,
	DMA_FLAG_HT_CurrentAdc2=DMA_FLAG_HT1,
	DMA_FLAG_TE_CurrentAdc2=DMA_FLAG_TE1,
	DMA_IRQ_CurrentAdc2=DMA1_Channel1_IRQn,	
	DMA_IRQ_PRIORITY_CurrentAdc2=IRQ_PRIORITY_DMA1_Channel1,
	

	DMA_FLAG_TC_Pan		=	DMA_FLAG_TC2,
	DMA_FLAG_HT_Pan		=	DMA_FLAG_HT2,
	DMA_FLAG_TE_Pan		=	DMA_FLAG_TE2,
	DMA_IRQ_Pan			=	DMA2_Channel2_IRQn,	
	DMA_IRQ_PRIORITY_Pan=	IRQ_PRIORITY_DMA2_Channel2,	
	
	DMA_FLAG_TC_HRTIM		=	DMA_FLAG_TC3,
	DMA_FLAG_HT_HRTIM		=	DMA_FLAG_HT3,
	DMA_FLAG_TE_HRTIM		=	DMA_FLAG_TE3,
	DMA_IRQ_HRTIM			=	DMA1_Channel3_IRQn,	
	DMA_IRQ_PRIORITY_HRTIM	=	IRQ_PRIORITY_DMA1_Channel3,	
	
	DMA_FLAG_TC_VC		=	DMA_FLAG_TC3,
	DMA_FLAG_HT_VC		=	DMA_FLAG_HT3,
	DMA_FLAG_TE_VC		=	DMA_FLAG_TE3,
	DMA_IRQ_VC			=	DMA1_Channel3_IRQn,	
	DMA_IRQ_PRIORITY_VC=	IRQ_PRIORITY_DMA1_Channel3,	

	DMA_FLAG_TC_UART_TX		=	DMA_FLAG_TC4,
	DMA_FLAG_HT_UART_TX		=	DMA_FLAG_HT4,
	DMA_FLAG_TE_UART_TX		=	DMA_FLAG_TE4,
	DMA_IRQ_UART_TX			=	DMA2_Channel4_IRQn,	
	DMA_IRQ_PRIORITY_UART_TX=	IRQ_PRIORITY_DMA2_Channel4,	
	
	DMA_FLAG_TC_UART_RX		=	DMA_FLAG_TC5,
	DMA_FLAG_HT_UART_RX		=	DMA_FLAG_HT5,
	DMA_FLAG_TE_UART_RX		=	DMA_FLAG_TE5,
	DMA_IRQ_UART_RX			=	DMA2_Channel5_IRQn,	
	DMA_IRQ_PRIORITY_UART_RX=	IRQ_PRIORITY_DMA2_Channel5,		

	DMA_FLAG_TC_Hrtim		=	DMA_FLAG_TC7,
	DMA_FLAG_HT_Hrtim		=	DMA_FLAG_HT7,
	DMA_FLAG_TE_Hrtim		=	DMA_FLAG_TE7,
	DMA_IRQ_Hrtim			=	DMA1_Channel7_IRQn,	
	DMA_IRQ_PRIORITY_Hrtim=	IRQ_PRIORITY_DMA1_Channel7,		

//	DMA_FLAG_TC_T2A		=	DMA_FLAG_TC8,
//	DMA_FLAG_HT_T2A		=	DMA_FLAG_HT8,
//	DMA_FLAG_TE_T2A		=	DMA_FLAG_TE8,
//	DMA_IRQ_T2A			=	DMA2_Channel8_IRQn,	
//	DMA_IRQ_PRIORITY_T2A=	IRQ_PRIORITY_DMA2_Channel8,		

////	DMA_FLAG_TC_T3A		=	DMA_FLAG_TC7,
////	DMA_FLAG_HT_T3A		=	DMA_FLAG_HT7,
////	DMA_FLAG_TE_T3A		=	DMA_FLAG_TE7,
////	DMA_IRQ_T3A			=	DMA1_Channel7_IRQn,	
////	DMA_IRQ_PRIORITY_T3A=	IRQ_PRIORITY_DMA1_Channel7,		

//	DMA_FLAG_TC_T4A		=	DMA_FLAG_TC8,
//	DMA_FLAG_HT_T4A		=	DMA_FLAG_HT8,
//	DMA_FLAG_TE_T4A		=	DMA_FLAG_TE8,
//	DMA_IRQ_T4A			=	DMA1_Channel8_IRQn,	
//	DMA_IRQ_PRIORITY_T4A=	IRQ_PRIORITY_DMA1_Channel8,		

};




typedef struct
{
	DMA_HandleTypeDef*			handle;
  	DMA_Channel_TypeDef    		*Instance;                                                  /*!< Register base address                */
  	DMA_InitTypeDef       		Init;                                                        /*!< DMA communication parameters         */

	uint32_t					it;											//中断号
	uint32_t					it_tc_flag;							//中断标志TC
	uint32_t					it_te_flag;							//中断标志TE
	uint32_t					it_ht_flag;							//中断标志HALF
	uint32_t  					irq;										//中断请求	
	uint32_t  					priority;								//中断优先级                                        /*!< Parent object state                  */

}API_DMA_InitTypeDef;



DMA_HandleTypeDef	 M2M_dma;
DMA_HandleTypeDef    Adc1Vc_dma;		//adc1 dma VC half
DMA_HandleTypeDef    Adc2Current_dma;	//adc2 dma Current1 Current2
DMA_HandleTypeDef    Adc3Current_dma;	//adc3 dma Current3	Current4

DMA_HandleTypeDef    Pan_dma;



DMA_HandleTypeDef    UartTx_dma;
DMA_HandleTypeDef    UartRx_dma;

DMA_HandleTypeDef    HrtimPotCh1_dma;		// 对应ADC2,第一个AD
DMA_HandleTypeDef    HrtimPotCh2_dma;		// 对应ADC2,第二个AD
DMA_HandleTypeDef    HrtimPotCh3_dma;		// 对应ADC3,第一个AD
DMA_HandleTypeDef    HrtimPotCh4_dma;		// 对应ADC3,第二个AD





void	API_DMA1_Channel4_IRQHandler(void);
void	API_ADC_M2M_Dma_Init(void);				//设置M2M DMA


API_DMA_InitTypeDef	M2M_DMA1_CHANNEL4_InitType={
	&M2M_dma,
	DMA_Channel_M2M,
	{
  DMA_REQUEST_MEM2MEM,/*uint32_t Request;                   !< Specifies the request selected for the specified channel.
                                           This parameter can be a value of @ref DMA_request */

  DMA_MEMORY_TO_MEMORY,/*uint32_t Direction;                 !< Specifies if the data will be transferred from memory to peripheral,
                                           from memory to memory or from peripheral to memory.
                                           This parameter can be a value of @ref DMA_Data_transfer_direction */

  DMA_PINC_ENABLE,/*uint32_t PeriphInc;                 !< Specifies whether the Peripheral address register should be incremented or not.
                                           This parameter can be a value of @ref DMA_Peripheral_incremented_mode */

  DMA_MINC_ENABLE,/*uint32_t MemInc;                    !< Specifies whether the memory address register should be incremented or not.
                                           This parameter can be a value of @ref DMA_Memory_incremented_mode */

  DMA_PDATAALIGN_WORD,/*uint32_t PeriphDataAlignment;       !< Specifies the Peripheral data width.
                                           This parameter can be a value of @ref DMA_Peripheral_data_size */

  DMA_MDATAALIGN_WORD,/*uint32_t MemDataAlignment;          !< Specifies the Memory data width.
                                           This parameter can be a value of @ref DMA_Memory_data_size */

  DMA_NORMAL,/*uint32_t Mode;                     !< Specifies the operation mode of the DMAy Channelx.
                                           This parameter can be a value of @ref DMA_mode
                                           @note The circular buffer mode cannot be used if the memory-to-memory
                                                 data transfer is configured on the selected Channel */

  DMA_PRIORITY_LOW,/*uint32_t Priority;                  !< Specifies the software priority for the DMAy Channelx.*/
 	
	
	
	},
		DMA_IT_TC,//uint32_t	it;											中断号
		DMA_FLAG_TC4,//uint32_t	it_tc_flag;							中断标志TC
		DMA_FLAG_TE4,//uint32_t	it_te_flag;							中断标志TE
		DMA_FLAG_HT4,//uint32_t	it_ht_flag;							中断标志HALF
		DMA_IRQ_M2M,//uint32_t  irq;										中断请求	
		DMA_IRQ_PRIORITY_M2M,//uint32_t  priority;								中断优先级   
	
};
#define		M2M_DmaBase	M2M_DMA1_CHANNEL4_InitType






API_DMA_InitTypeDef	ADC2_Current_InitType={
	&Adc2Current_dma,
	DMA_Channel_CurrentAdc2,
	{
  DMA_REQUEST_ADC2,/*uint32_t Request;                   !< Specifies the request selected for the specified channel.
                                           This parameter can be a value of @ref DMA_request */

  DMA_PERIPH_TO_MEMORY,/*uint32_t Direction;                 !< Specifies if the data will be transferred from memory to peripheral,
                                           from memory to memory or from peripheral to memory.
                                           This parameter can be a value of @ref DMA_Data_transfer_direction */

  DMA_PINC_DISABLE,/*uint32_t PeriphInc;                 !< Specifies whether the Peripheral address register should be incremented or not.
                                           This parameter can be a value of @ref DMA_Peripheral_incremented_mode */

  DMA_MINC_ENABLE,/*uint32_t MemInc;                    !< Specifies whether the memory address register should be incremented or not.
                                           This parameter can be a value of @ref DMA_Memory_incremented_mode */

  DMA_PDATAALIGN_HALFWORD,/*uint32_t PeriphDataAlignment;       !< Specifies the Peripheral data width.
                                           This parameter can be a value of @ref DMA_Peripheral_data_size */

  DMA_MDATAALIGN_HALFWORD,/*uint32_t MemDataAlignment;          !< Specifies the Memory data width.
                                           This parameter can be a value of @ref DMA_Memory_data_size */

  DMA_CIRCULAR,/*uint32_t Mode;                     !< Specifies the operation mode of the DMAy Channelx.
                                           This parameter can be a value of @ref DMA_mode
                                           @note The circular buffer mode cannot be used if the memory-to-memory
                                                 data transfer is configured on the selected Channel */

  DMA_PRIORITY_MEDIUM,/*uint32_t Priority;                  !< Specifies the software priority for the DMAy Channelx.*/
 	
	
	
	},
		0,//DMA_IT_TC|DMA_IT_HT,//uint32_t	it;											中断号
		DMA_FLAG_TC_CurrentAdc2,//uint32_t	it_tc_flag;							中断标志TC
		DMA_FLAG_TE_CurrentAdc2,//uint32_t	it_te_flag;							中断标志TE
		DMA_FLAG_HT_CurrentAdc2,//uint32_t	it_ht_flag;							中断标志HALF
		DMA_IRQ_CurrentAdc2,//uint32_t  irq;										中断请求	
		DMA_IRQ_PRIORITY_CurrentAdc2,//uint32_t  priority;								中断优先级   

};
#define	CurrentAdc2_DmaBase			ADC2_Current_InitType


API_DMA_InitTypeDef	ADC3_Current_InitType={
	&Adc3Current_dma,
	DMA_Channel_CurrentAdc3,
	{
  DMA_REQUEST_ADC3,/*uint32_t Request;                   !< Specifies the request selected for the specified channel.
                                           This parameter can be a value of @ref DMA_request */

  DMA_PERIPH_TO_MEMORY,/*uint32_t Direction;                 !< Specifies if the data will be transferred from memory to peripheral,
                                           from memory to memory or from peripheral to memory.
                                           This parameter can be a value of @ref DMA_Data_transfer_direction */

  DMA_PINC_DISABLE,/*uint32_t PeriphInc;                 !< Specifies whether the Peripheral address register should be incremented or not.
                                           This parameter can be a value of @ref DMA_Peripheral_incremented_mode */

  DMA_MINC_ENABLE,/*uint32_t MemInc;                    !< Specifies whether the memory address register should be incremented or not.
                                           This parameter can be a value of @ref DMA_Memory_incremented_mode */

  DMA_PDATAALIGN_HALFWORD,/*uint32_t PeriphDataAlignment;       !< Specifies the Peripheral data width.
                                           This parameter can be a value of @ref DMA_Peripheral_data_size */

  DMA_MDATAALIGN_HALFWORD,/*uint32_t MemDataAlignment;          !< Specifies the Memory data width.
                                           This parameter can be a value of @ref DMA_Memory_data_size */

  DMA_CIRCULAR,/*uint32_t Mode;                     !< Specifies the operation mode of the DMAy Channelx.
                                           This parameter can be a value of @ref DMA_mode
                                           @note The circular buffer mode cannot be used if the memory-to-memory
                                                 data transfer is configured on the selected Channel */

  DMA_PRIORITY_MEDIUM,/*uint32_t Priority;                  !< Specifies the software priority for the DMAy Channelx.*/
 	
	
	
	},
		0,//DMA_IT_TC|DMA_IT_HT,//uint32_t	it;											中断号
		0,//uint32_t	it_tc_flag;							中断标志TC
		0,//uint32_t	it_te_flag;							中断标志TE
		0,//uint32_t	it_ht_flag;							中断标志HALF
		0,//uint32_t  irq;										中断请求	
		0,//uint32_t  priority;								中断优先级   

};
#define	CurrentAdc3_DmaBase			ADC3_Current_InitType


API_DMA_InitTypeDef	ADC1_Vc_InitType={
	&Adc1Vc_dma,
	DMA_Channel_VcAdc1,
	{
  DMA_REQUEST_ADC1,/*uint32_t Request;                   !< Specifies the request selected for the specified channel.
                                           This parameter can be a value of @ref DMA_request */

  DMA_PERIPH_TO_MEMORY,/*uint32_t Direction;                 !< Specifies if the data will be transferred from memory to peripheral,
                                           from memory to memory or from peripheral to memory.
                                           This parameter can be a value of @ref DMA_Data_transfer_direction */

  DMA_PINC_DISABLE,/*uint32_t PeriphInc;                 !< Specifies whether the Peripheral address register should be incremented or not.
                                           This parameter can be a value of @ref DMA_Peripheral_incremented_mode */

  DMA_MINC_ENABLE,/*uint32_t MemInc;                    !< Specifies whether the memory address register should be incremented or not.
                                           This parameter can be a value of @ref DMA_Memory_incremented_mode */

  DMA_PDATAALIGN_HALFWORD,/*uint32_t PeriphDataAlignment;       !< Specifies the Peripheral data width.
                                           This parameter can be a value of @ref DMA_Peripheral_data_size */

  DMA_MDATAALIGN_HALFWORD,/*uint32_t MemDataAlignment;          !< Specifies the Memory data width.
                                           This parameter can be a value of @ref DMA_Memory_data_size */

  DMA_NORMAL,/*uint32_t Mode;                     !< Specifies the operation mode of the DMAy Channelx.
                                           This parameter can be a value of @ref DMA_mode
                                           @note The circular buffer mode cannot be used if the memory-to-memory
                                                 data transfer is configured on the selected Channel */

  DMA_PRIORITY_MEDIUM,/*uint32_t Priority;                  !< Specifies the software priority for the DMAy Channelx.*/
 	
	
	
	},
		0,//DMA_IT_TC|DMA_IT_HT,//uint32_t	it;											中断号
		0,//uint32_t	it_tc_flag;							中断标志TC
		0,//uint32_t	it_te_flag;							中断标志TE
		0,//uint32_t	it_ht_flag;							中断标志HALF
		0,//uint32_t  irq;										中断请求	
		0,//uint32_t  priority;								中断优先级   

};
#define	VcAdc1_DmaBase			ADC1_Vc_InitType

// API_DMA_InitTypeDef	HRTIM_DMA1_CHANNEL3_InitType={
// 	&Tim3Hrtim_dma,
// 	DMA_Channel_HRTIM1,
// 	{
//   		DMA_REQUEST_HRTIM1_F,/*uint32_t Request;                   !< Specifies the request selected for the specified channel.
//                                            This parameter can be a value of @ref DMA_request */

//   		DMA_PERIPH_TO_MEMORY,/*uint32_t Direction;                 !< Specifies if the data will be transferred from memory to peripheral,
//                                            from memory to memory or from peripheral to memory.
//                                            This parameter can be a value of @ref DMA_Data_transfer_direction */

//   		DMA_PINC_DISABLE,/*uint32_t PeriphInc;                 !< Specifies whether the Peripheral address register should be incremented or not.
//                                            This parameter can be a value of @ref DMA_Peripheral_incremented_mode */

//   		DMA_MINC_ENABLE,/*uint32_t MemInc;                    !< Specifies whether the memory address register should be incremented or not.
//                                            This parameter can be a value of @ref DMA_Memory_incremented_mode */

//   		DMA_PDATAALIGN_HALFWORD,/*uint32_t PeriphDataAlignment;       !< Specifies the Peripheral data width.
//                                            This parameter can be a value of @ref DMA_Peripheral_data_size */

//   		DMA_MDATAALIGN_HALFWORD,/*uint32_t MemDataAlignment;          !< Specifies the Memory data width.
//                                            This parameter can be a value of @ref DMA_Memory_data_size */

// 			DMA_NORMAL,				/*uint32_t Mode;                     !< Specifies the operation mode of the DMAy Channelx.
//                                            This parameter can be a value of @ref DMA_mode
//                                            @note The circular buffer mode cannot be used if the memory-to-memory
//                                                  data transfer is configured on the selected Channel */

//   		DMA_PRIORITY_LOW,/*uint32_t Priority;                  !< Specifies the software priority for the DMAy Channelx.*/
 	
	
	
// 	},
// 		0,//DMA_IT_TC|DMA_IT_HT,//uint32_t	it;											中断号
// 		DMA_FLAG_TC_HRTIM,//uint32_t	it_tc_flag;							中断标志TC
// 		DMA_FLAG_TE_HRTIM,//uint32_t	it_te_flag;							中断标志TE
// 		DMA_FLAG_HT_HRTIM,//uint32_t	it_ht_flag;							中断标志HALF
// 		DMA_IRQ_HRTIM,//uint32_t  irq;										中断请求	
// 		DMA_IRQ_PRIORITY_HRTIM,//uint32_t  priority;								中断优先级   

// };
// #define	Hrtim1us_DmaBase	HRTIM_DMA1_CHANNEL3_InitType
		


// API_DMA_InitTypeDef	ADC1_DMA2_CHANNEL1_InitType={
// 	&AdcTempeAdc1_dma,
// 	DMA_Channel_T34A,
// 	{
//   DMA_REQUEST_ADC1,/*uint32_t Request;                   !< Specifies the request selected for the specified channel.
//                                            This parameter can be a value of @ref DMA_request */

//   DMA_PERIPH_TO_MEMORY,/*uint32_t Direction;                 !< Specifies if the data will be transferred from memory to peripheral,
//                                            from memory to memory or from peripheral to memory.
//                                            This parameter can be a value of @ref DMA_Data_transfer_direction */

//   DMA_PINC_DISABLE,/*uint32_t PeriphInc;                 !< Specifies whether the Peripheral address register should be incremented or not.
//                                            This parameter can be a value of @ref DMA_Peripheral_incremented_mode */

//   DMA_MINC_ENABLE,/*uint32_t MemInc;                    !< Specifies whether the memory address register should be incremented or not.
//                                            This parameter can be a value of @ref DMA_Memory_incremented_mode */

//   DMA_PDATAALIGN_HALFWORD,/*uint32_t PeriphDataAlignment;       !< Specifies the Peripheral data width.
//                                            This parameter can be a value of @ref DMA_Peripheral_data_size */

//   DMA_MDATAALIGN_HALFWORD,/*uint32_t MemDataAlignment;          !< Specifies the Memory data width.
//                                            This parameter can be a value of @ref DMA_Memory_data_size */

//   DMA_CIRCULAR,/*uint32_t Mode;                     !< Specifies the operation mode of the DMAy Channelx.
//                                            This parameter can be a value of @ref DMA_mode
//                                            @note The circular buffer mode cannot be used if the memory-to-memory
//                                                  data transfer is configured on the selected Channel */

//   DMA_PRIORITY_LOW,/*uint32_t Priority;                  !< Specifies the software priority for the DMAy Channelx.*/
// 	
//	
//	
// 	},
// 		0,//uint32_t	it;											中断号
// 		DMA_FLAG_TC1,//uint32_t	it_tc_flag;							中断标志TC
// 		DMA_FLAG_TE1,//uint32_t	it_te_flag;							中断标志TE
// 		DMA_FLAG_HT1,//uint32_t	it_ht_flag;							中断标志HALF
// 		0,//uint32_t  irq;										中断请求	
// 		0,//uint32_t  priority;								中断优先级   

// };
// #define	TempeAdc1_DmaBase	ADC1_DMA2_CHANNEL1_InitType



API_DMA_InitTypeDef	TIM7_DMA2_CHANNEL2_InitType={
	&Pan_dma,
	DMA_Channel_Pan,
	{
  DMA_REQUEST_PAN_UP,/*DMA_REQUEST_ADC2 DMA_REQUEST_TIM3_UP,uint32_t Request;                   !< Specifies the request selected for the specified channel.
                                           This parameter can be a value of @ref DMA_request */

  DMA_PERIPH_TO_MEMORY,/*uint32_t Direction;                 !< Specifies if the data will be transferred from memory to peripheral,
                                           from memory to memory or from peripheral to memory.
                                           This parameter can be a value of @ref DMA_Data_transfer_direction */

  DMA_PINC_DISABLE,/*uint32_t PeriphInc;                 !< Specifies whether the Peripheral address register should be incremented or not.
                                           This parameter can be a value of @ref DMA_Peripheral_incremented_mode */

  DMA_MINC_ENABLE,/*uint32_t MemInc;                    !< Specifies whether the memory address register should be incremented or not.
                                           This parameter can be a value of @ref DMA_Memory_incremented_mode */

  DMA_PDATAALIGN_HALFWORD,/*uint32_t PeriphDataAlignment;       !< Specifies the Peripheral data width.
                                           This parameter can be a value of @ref DMA_Peripheral_data_size */

  DMA_MDATAALIGN_HALFWORD,/*uint32_t MemDataAlignment;          !< Specifies the Memory data width.
                                           This parameter can be a value of @ref DMA_Memory_data_size */

  DMA_NORMAL,/*uint32_t Mode;                     !< Specifies the operation mode of the DMAy Channelx.
                                           This parameter can be a value of @ref DMA_mode
                                           @note The circular buffer mode cannot be used if the memory-to-memory
                                                 data transfer is configured on the selected Channel */

  DMA_PRIORITY_LOW,/*uint32_t Priority;                  !< Specifies the software priority for the DMAy Channelx.*/
 	
	
	
	},
		DMA_IT_TC,//uint32_t	it;											中断号
		DMA_FLAG_TC_Pan,//uint32_t	it_tc_flag;							中断标志TC
		DMA_FLAG_TE_Pan,//uint32_t	it_te_flag;							中断标志TE
		DMA_FLAG_HT_Pan,//uint32_t	it_ht_flag;							中断标志HALF
		DMA_IRQ_Pan,//uint32_t  irq;										中断请求	
		DMA_IRQ_PRIORITY_Pan,//uint32_t  priority;								中断优先级   

};
#define	Pan_DmaBase	TIM7_DMA2_CHANNEL2_InitType


API_DMA_InitTypeDef	UART_TX_DMA2_CHANNEL4_InitType={
	&UartTx_dma,
	DMA_Channel_UART_TX,
	{
  DMA_REQUEST_UARTx_TX,/*uint32_t Request;                   !< Specifies the request selected for the specified channel.
                                           This parameter can be a value of @ref DMA_request */

  DMA_MEMORY_TO_PERIPH,/*uint32_t Direction;                 !< Specifies if the data will be transferred from memory to peripheral,
                                           from memory to memory or from peripheral to memory.
                                           This parameter can be a value of @ref DMA_Data_transfer_direction */

  DMA_PINC_DISABLE,/*uint32_t PeriphInc;                 !< Specifies whether the Peripheral address register should be incremented or not.
                                           This parameter can be a value of @ref DMA_Peripheral_incremented_mode */

  DMA_MINC_ENABLE,/*uint32_t MemInc;                    !< Specifies whether the memory address register should be incremented or not.
                                           This parameter can be a value of @ref DMA_Memory_incremented_mode */

  DMA_PDATAALIGN_BYTE,/*uint32_t PeriphDataAlignment;       !< Specifies the Peripheral data width.
                                           This parameter can be a value of @ref DMA_Peripheral_data_size */

  DMA_MDATAALIGN_BYTE,/*uint32_t MemDataAlignment;          !< Specifies the Memory data width.
                                           This parameter can be a value of @ref DMA_Memory_data_size */

  DMA_NORMAL,/*uint32_t Mode;                     !< Specifies the operation mode of the DMAy Channelx.
                                           This parameter can be a value of @ref DMA_mode
                                           @note The circular buffer mode cannot be used if the memory-to-memory
                                                 data transfer is configured on the selected Channel */

  DMA_PRIORITY_LOW,/*uint32_t Priority;                  !< Specifies the software priority for the DMAy Channelx.*/
 	
	
	
	},
		DMA_IT_TC,//DMA_IT_TC|DMA_IT_HT,//uint32_t	it;											中断号
		DMA_FLAG_TC_UART_TX,//uint32_t	it_tc_flag;							中断标志TC
		DMA_FLAG_TE_UART_TX,//uint32_t	it_te_flag;							中断标志TE
		DMA_FLAG_HT_UART_TX,//uint32_t	it_ht_flag;							中断标志HALF
		DMA_IRQ_UART_TX,//uint32_t  irq;												中断请求	
		DMA_IRQ_PRIORITY_UART_TX,//uint32_t  priority;					中断优先级   

};
#define	UartTx_DmaBase	UART_TX_DMA2_CHANNEL4_InitType

API_DMA_InitTypeDef	UART_RX_DMA2_CHANNEL4_InitType={
	&UartRx_dma,
	DMA_Channel_UART_RX,
	{
  DMA_REQUEST_UARTx_RX,/*uint32_t Request;                   !< Specifies the request selected for the specified channel.
                                           This parameter can be a value of @ref DMA_request */

  DMA_PERIPH_TO_MEMORY,/*uint32_t Direction;                 !< Specifies if the data will be transferred from memory to peripheral,
                                           from memory to memory or from peripheral to memory.
                                           This parameter can be a value of @ref DMA_Data_transfer_direction */

  DMA_PINC_DISABLE,/*uint32_t PeriphInc;                 !< Specifies whether the Peripheral address register should be incremented or not.
                                           This parameter can be a value of @ref DMA_Peripheral_incremented_mode */

  DMA_MINC_ENABLE,/*uint32_t MemInc;                    !< Specifies whether the memory address register should be incremented or not.
                                           This parameter can be a value of @ref DMA_Memory_incremented_mode */

  DMA_PDATAALIGN_BYTE,/*uint32_t PeriphDataAlignment;       !< Specifies the Peripheral data width.
                                           This parameter can be a value of @ref DMA_Peripheral_data_size */

  DMA_MDATAALIGN_BYTE,/*uint32_t MemDataAlignment;          !< Specifies the Memory data width.
                                           This parameter can be a value of @ref DMA_Memory_data_size */

  DMA_NORMAL,/*uint32_t Mode;                     !< Specifies the operation mode of the DMAy Channelx.
                                           This parameter can be a value of @ref DMA_mode
                                           @note The circular buffer mode cannot be used if the memory-to-memory
                                                 data transfer is configured on the selected Channel */

  DMA_PRIORITY_LOW,/*uint32_t Priority;                  !< Specifies the software priority for the DMAy Channelx.*/
 	
	
	
	},
		0,//DMA_IT_TC|DMA_IT_HT,//uint32_t	it;											中断号
		DMA_FLAG_TC_UART_RX,//uint32_t	it_tc_flag;							中断标志TC
		DMA_FLAG_TE_UART_RX,//uint32_t	it_te_flag;							中断标志TE
		DMA_FLAG_HT_UART_RX,//uint32_t	it_ht_flag;							中断标志HALF
		DMA_IRQ_UART_RX,//uint32_t  irq;										中断请求	
		DMA_IRQ_PRIORITY_UART_RX,//uint32_t  priority;								中断优先级   

};
#define	UartRx_DmaBase	UART_RX_DMA2_CHANNEL4_InitType

//API_DMA_InitTypeDef	TIM3_DMA2_CHANNEL7_InitType={
//	&AdcT1A_dma,
//	DMA_Channel_TXA_ADC1,
//	{  
//  DMA_REQUEST_TXA_ADC1,/*uint32_t Request;                   !< Specifies the request selected for the specified channel.
//                                           This parameter can be a value of @ref DMA_request */

//  DMA_PERIPH_TO_MEMORY,/*uint32_t Direction;                 !< Specifies if the data will be transferred from memory to peripheral,
//                                           from memory to memory or from peripheral to memory.
//                                           This parameter can be a value of @ref DMA_Data_transfer_direction */

//  DMA_PINC_DISABLE,/*uint32_t PeriphInc;                 !< Specifies whether the Peripheral address register should be incremented or not.
//                                           This parameter can be a value of @ref DMA_Peripheral_incremented_mode */

//  DMA_MINC_ENABLE,/*uint32_t MemInc;                    !< Specifies whether the memory address register should be incremented or not.
//                                           This parameter can be a value of @ref DMA_Memory_incremented_mode */

//  DMA_PDATAALIGN_HALFWORD,/*uint32_t PeriphDataAlignment;       !< Specifies the Peripheral data width.
//                                           This parameter can be a value of @ref DMA_Peripheral_data_size */

//  DMA_MDATAALIGN_HALFWORD,/*uint32_t MemDataAlignment;          !< Specifies the Memory data width.
//                                           This parameter can be a value of @ref DMA_Memory_data_size */

//  DMA_NORMAL,/*uint32_t Mode;                     !< Specifies the operation mode of the DMAy Channelx.
//                                           This parameter can be a value of @ref DMA_mode
//                                           @note The circular buffer mode cannot be used if the memory-to-memory
//                                                 data transfer is configured on the selected Channel */

//  	DMA_PRIORITY_MEDIUM,/*uint32_t Priority;                  !< Specifies the software priority for the DMAy Channelx.*/
// 	
//	
//	
//	},
//		DMA_IT_TC,//DMA_IT_TC|DMA_IT_HT,//uint32_t	it;											中断号
//		DMA_FLAG_TC_T1A,//uint32_t	it_tc_flag;							中断标志TC
//		DMA_FLAG_TE_T1A,//uint32_t	it_te_flag;							中断标志TE
//		DMA_FLAG_HT_T1A,//uint32_t	it_ht_flag;							中断标志HALF
//		DMA_IRQ_T1A,//uint32_t  irq;										中断请求	
//		DMA_IRQ_PRIORITY_T1A,//uint32_t  priority;								中断优先级   

//};
//#define		Current_ADC1_DmaBase					TIM3_DMA2_CHANNEL7_InitType


//#define	DMA_Channel_HrtimA		DMA_Channel_FMAC_READ	//2_6
API_DMA_InitTypeDef	HRTIM_POTCH1_InitType={
	&HrtimPotCh1_dma,
	DMA_Channel_HrtimPotCh1,
	{  
 		DMA_REQUEST_TIM_HRTIM_PotCh1,/*uint32_t Request;                   !< Specifies the request selected for the specified channel.
                                          This parameter can be a value of @ref DMA_request */

 		DMA_PERIPH_TO_MEMORY,/*uint32_t Direction;                 !< Specifies if the data will be transferred from memory to peripheral,
                                          from memory to memory or from peripheral to memory.
                                          This parameter can be a value of @ref DMA_Data_transfer_direction */

 		DMA_PINC_DISABLE,/*uint32_t PeriphInc;                 !< Specifies whether the Peripheral address register should be incremented or not.
                                          This parameter can be a value of @ref DMA_Peripheral_incremented_mode */

 		DMA_MINC_ENABLE,/*uint32_t MemInc;                    !< Specifies whether the memory address register should be incremented or not.
                                          This parameter can be a value of @ref DMA_Memory_incremented_mode */

 		DMA_PDATAALIGN_HALFWORD,/*uint32_t PeriphDataAlignment;       !< Specifies the Peripheral data width.
                                          This parameter can be a value of @ref DMA_Peripheral_data_size */

 		DMA_MDATAALIGN_HALFWORD,/*uint32_t MemDataAlignment;          !< Specifies the Memory data width.
                                          This parameter can be a value of @ref DMA_Memory_data_size */

 		DMA_NORMAL,/*uint32_t Mode;                     !< Specifies the operation mode of the DMAy Channelx.
                                          This parameter can be a value of @ref DMA_mode
                                          @note The circular buffer mode cannot be used if the memory-to-memory
                                                data transfer is configured on the selected Channel */

 		DMA_PRIORITY_HIGH,/*uint32_t Priority;                  !< Specifies the software priority for the DMAy Channelx.*/
	
	
	
	},
		DMA_IT_TC,//DMA_IT_TC|DMA_IT_HT,//uint32_t	it;											中断号
		DMA_FLAG_TC_Hrtim,//uint32_t	it_tc_flag;							中断标志TC
		DMA_FLAG_TE_Hrtim,//uint32_t	it_te_flag;							中断标志TE
		DMA_FLAG_HT_Hrtim,//uint32_t	it_ht_flag;							中断标志HALF
		DMA_IRQ_Hrtim,//uint32_t  irq;										中断请求	
		DMA_IRQ_PRIORITY_Hrtim,//uint32_t  priority;								中断优先级   

};
#define		HrtimPotCh1_DmaBase					HRTIM_POTCH1_InitType

API_DMA_InitTypeDef	HRTIM_POTCH2_InitType={
	&HrtimPotCh2_dma,
	DMA_Channel_HrtimPotCh2,
	{  
 		DMA_REQUEST_TIM_HRTIM_PotCh2,/*uint32_t Request;                   !< Specifies the request selected for the specified channel.
                                          This parameter can be a value of @ref DMA_request */

 		DMA_PERIPH_TO_MEMORY,/*uint32_t Direction;                 !< Specifies if the data will be transferred from memory to peripheral,
                                          from memory to memory or from peripheral to memory.
                                          This parameter can be a value of @ref DMA_Data_transfer_direction */

 		DMA_PINC_DISABLE,/*uint32_t PeriphInc;                 !< Specifies whether the Peripheral address register should be incremented or not.
                                          This parameter can be a value of @ref DMA_Peripheral_incremented_mode */

 		DMA_MINC_ENABLE,/*uint32_t MemInc;                    !< Specifies whether the memory address register should be incremented or not.
                                          This parameter can be a value of @ref DMA_Memory_incremented_mode */

 		DMA_PDATAALIGN_HALFWORD,/*uint32_t PeriphDataAlignment;       !< Specifies the Peripheral data width.
                                          This parameter can be a value of @ref DMA_Peripheral_data_size */

 		DMA_MDATAALIGN_HALFWORD,/*uint32_t MemDataAlignment;          !< Specifies the Memory data width.
                                          This parameter can be a value of @ref DMA_Memory_data_size */

 		DMA_NORMAL,/*uint32_t Mode;                     !< Specifies the operation mode of the DMAy Channelx.
                                          This parameter can be a value of @ref DMA_mode
                                          @note The circular buffer mode cannot be used if the memory-to-memory
                                                data transfer is configured on the selected Channel */

 		DMA_PRIORITY_HIGH,/*uint32_t Priority;                  !< Specifies the software priority for the DMAy Channelx.*/
	
	
	
	},
		0,//DMA_IT_TC|DMA_IT_HT,//uint32_t	it;											中断号
		0,//uint32_t	it_tc_flag;							中断标志TC
		0,//uint32_t	it_te_flag;							中断标志TE
		0,//uint32_t	it_ht_flag;							中断标志HALF
		0,//uint32_t  irq;										中断请求	
		0,//uint32_t  priority;								中断优先级   

};
#define		HrtimPotCh2_DmaBase					HRTIM_POTCH2_InitType


API_DMA_InitTypeDef	HRTIM_POTCH3_InitType={
	&HrtimPotCh3_dma,
	DMA_Channel_HrtimPotCh3,
	{  
 		DMA_REQUEST_TIM_HRTIM_PotCh3,/*uint32_t Request;                   !< Specifies the request selected for the specified channel.
                                          This parameter can be a value of @ref DMA_request */

 		DMA_PERIPH_TO_MEMORY,/*uint32_t Direction;                 !< Specifies if the data will be transferred from memory to peripheral,
                                          from memory to memory or from peripheral to memory.
                                          This parameter can be a value of @ref DMA_Data_transfer_direction */

 		DMA_PINC_DISABLE,/*uint32_t PeriphInc;                 !< Specifies whether the Peripheral address register should be incremented or not.
                                          This parameter can be a value of @ref DMA_Peripheral_incremented_mode */

 		DMA_MINC_ENABLE,/*uint32_t MemInc;                    !< Specifies whether the memory address register should be incremented or not.
                                          This parameter can be a value of @ref DMA_Memory_incremented_mode */

 		DMA_PDATAALIGN_HALFWORD,/*uint32_t PeriphDataAlignment;       !< Specifies the Peripheral data width.
                                          This parameter can be a value of @ref DMA_Peripheral_data_size */

 		DMA_MDATAALIGN_HALFWORD,/*uint32_t MemDataAlignment;          !< Specifies the Memory data width.
                                          This parameter can be a value of @ref DMA_Memory_data_size */

 		DMA_NORMAL,/*uint32_t Mode;                     !< Specifies the operation mode of the DMAy Channelx.
                                          This parameter can be a value of @ref DMA_mode
                                          @note The circular buffer mode cannot be used if the memory-to-memory
                                                data transfer is configured on the selected Channel */

 		DMA_PRIORITY_MEDIUM,/*uint32_t Priority;                  !< Specifies the software priority for the DMAy Channelx.*/
	
	
	
	},
		0,//DMA_IT_TC|DMA_IT_HT,//uint32_t	it;											中断号
		0,//uint32_t	it_tc_flag;							中断标志TC
		0,//uint32_t	it_te_flag;							中断标志TE
		0,//uint32_t	it_ht_flag;							中断标志HALF
		0,//uint32_t  irq;										中断请求	
		0,//uint32_t  priority;								中断优先级   

};
#define		HrtimPotCh3_DmaBase					HRTIM_POTCH3_InitType

API_DMA_InitTypeDef	HRTIM_POTCH4_InitType={
	&HrtimPotCh4_dma,
	DMA_Channel_HrtimPotCh4,
	{  
 		DMA_REQUEST_TIM_HRTIM_PotCh4,/*uint32_t Request;                   !< Specifies the request selected for the specified channel.
                                          This parameter can be a value of @ref DMA_request */

 		DMA_PERIPH_TO_MEMORY,/*uint32_t Direction;                 !< Specifies if the data will be transferred from memory to peripheral,
                                          from memory to memory or from peripheral to memory.
                                          This parameter can be a value of @ref DMA_Data_transfer_direction */

 		DMA_PINC_DISABLE,/*uint32_t PeriphInc;                 !< Specifies whether the Peripheral address register should be incremented or not.
                                          This parameter can be a value of @ref DMA_Peripheral_incremented_mode */

 		DMA_MINC_ENABLE,/*uint32_t MemInc;                    !< Specifies whether the memory address register should be incremented or not.
                                          This parameter can be a value of @ref DMA_Memory_incremented_mode */

 		DMA_PDATAALIGN_HALFWORD,/*uint32_t PeriphDataAlignment;       !< Specifies the Peripheral data width.
                                          This parameter can be a value of @ref DMA_Peripheral_data_size */

 		DMA_MDATAALIGN_HALFWORD,/*uint32_t MemDataAlignment;          !< Specifies the Memory data width.
                                          This parameter can be a value of @ref DMA_Memory_data_size */

 		DMA_NORMAL,/*uint32_t Mode;                     !< Specifies the operation mode of the DMAy Channelx.
                                          This parameter can be a value of @ref DMA_mode
                                          @note The circular buffer mode cannot be used if the memory-to-memory
                                                data transfer is configured on the selected Channel */

 		DMA_PRIORITY_MEDIUM,/*uint32_t Priority;                  !< Specifies the software priority for the DMAy Channelx.*/
	
	
	
	},
		0,//DMA_IT_TC|DMA_IT_HT,//uint32_t	it;											中断号
		0,//uint32_t	it_tc_flag;							中断标志TC
		0,//uint32_t	it_te_flag;							中断标志TE
		0,//uint32_t	it_ht_flag;							中断标志HALF
		0,//uint32_t  irq;										中断请求	
		0,//uint32_t  priority;								中断优先级   

};
#define		HrtimPotCh4_DmaBase					HRTIM_POTCH4_InitType



//#if 1 



//#endif

// API_DMA_InitTypeDef*		DmaHrtimStr[]=
// {		//用于检测HRTIM多通道CNT值
// 		&DmaHrtimA,
// 		&DmaHrtimB,			
// 		&DmaHrtimC,
// 		&DmaHrtimD,
// 		&DmaHrtimE,
// 		&DmaHrtimF,
// };


#if 1




API_DMA_InitTypeDef*		DmaBaseStr[]=
{
		&M2M_DmaBase,
		&VcAdc1_DmaBase,					//HRTIM同步 DMA计数
		&CurrentAdc2_DmaBase,					//HRTIM同步 DMA计数
		&CurrentAdc3_DmaBase,			//谐振电流(4组共同保存)	
		&Pan_DmaBase,					//检锅数据
		&UartTx_DmaBase,	
		&UartRx_DmaBase,	
		&HrtimPotCh1_DmaBase,
		&HrtimPotCh2_DmaBase,
		&HrtimPotCh3_DmaBase,		
		&HrtimPotCh4_DmaBase,

	};
#else

API_DMA_InitTypeDef*		DmaBaseStr[]=
{
		&M2M_DmaBase,
		&Hrtim1us_DmaBase,				//hrtim 1us值与通道1 对应

		&Pan_DmaBase,					//检锅数据
		&UartTx_DmaBase,	
		&UartRx_DmaBase,	

		&Current_ADC1_DmaBase,			//谐振电流1(单独保存)	
		&Current_HRTIM1_DmaBase,					
		&Current_ADC2_DmaBase,	
		&Current_HRTIM2_DmaBase,				
		&Hrtim_DmaBase,					//HRTIM同步 DMA计数 与通道2对应
};

#endif




//-------变量定义-----------------------------------------
static	uint8_t M2M_Status=0;
static	uint8_t M2Mlow_Status=0;
/********************************************************************************
    FileName    : void 	FanSetSpeed(uint8_t t_fandiv);				//风机转速取各炉头最大值
    Author      :  rsl
    Version     :  V1.0.1
    Brief       :  风机转速

    Date        :  2018-10-19
    Modify      :
                   2018-10-19 创建

    Copyright (c)    Foshan XinSun Electronic Technology CO.,Ltd
********************************************************************************/



/**
  * @brief  DMA conversion complete callback
  * @note   This function is executed when the transfer complete interrupt
  *         is generated
  * @retval None
  */
static void TransferComplete(DMA_HandleTypeDef *dma_handle)
{

	API_ADC_DMA_M2M_IRQHandlerCallBack();					//M2M_dma 回调
	
	M2M_Status=M2M_FREE;

}

uint32_t*	API_DMA_GetDmaHandle(API_DMA_CH_ENUM ch)	//得到DMA的HANDLE
{
	return	(uint32_t*)(DmaBaseStr[ch]->handle);
}

uint32_t 	API_DMA_GetDmaCndtr(API_DMA_CH_ENUM ch)
{
		uint32_t  cnctr;
			
		cnctr=__HAL_DMA_GET_COUNTER(DmaBaseStr[ch]);

		return 	cnctr;
}	
uint32_t* 	API_DMA_GetDmaCndtrAddress(API_DMA_CH_ENUM ch)
{
		return	(uint32_t*)&(DmaBaseStr[ch]->handle->Instance->CNDTR);

}	


uint8_t 		API_ADC_GetM2Mstatus(void)
{
		return 	M2M_Status;
}	
void 		API_ADC_SetM2Mstatus(uint8_t m2m_sta)
{
			M2M_Status=m2m_sta;
}	
/**
  * @brief  DMA conversion error callback
  * @note   This function is executed when the transfer error interrupt
  *         is generated during DMA transfer
  * @retval None
  */
//static void TransferError(DMA_HandleTypeDef *M2M_dma)
//{
//  transferErrorDetected = 1;
//	M2M_Status=M2M_ERR;
//}
//static void TransferErrorTxA(DMA_HandleTypeDef *M2M_dma)
//{
//  transferErrorDetected = 1;
//}






void	API_DMA1_Channel4_IRQHandler()
{	
//  HAL_DMA_IRQHandler(&M2M_dma);
	
  uint32_t flag_it = M2M_DmaBase.handle->DmaBaseAddress->ISR;
//  uint32_t source_it = M2M_DmaBase.handle->Instance->CCR;
  /* Half Transfer Complete Interrupt management ******************************/
  if (flag_it&M2M_DmaBase.it_ht_flag)
  {
	__HAL_DMA_CLEAR_FLAG(M2M_DmaBase.handle,M2M_DmaBase.it_tc_flag|M2M_DmaBase.it_te_flag|M2M_DmaBase.it_ht_flag);
		
		M2M_Status=M2M_FREE;
		API_DMA_M2M_OverCallback();					//M2M 傳遞完成

	}
}

//void	API_DMA2_Channel3_IRQHandler()
//{	
////  HAL_DMA_IRQHandler(&M2M_dma);
//	
//  uint32_t flag_it = M2Mlow_DmaBase.handle->DmaBaseAddress->ISR;
////  uint32_t source_it = M2M_DmaBase.handle->Instance->CCR;
//  /* Half Transfer Complete Interrupt management ******************************/
//  if (flag_it&M2Mlow_DmaBase.it_tc_flag)
//  {	
//		__HAL_DMA_CLEAR_FLAG(M2Mlow_DmaBase.handle,M2Mlow_DmaBase.it_tc_flag|M2Mlow_DmaBase.it_te_flag|M2Mlow_DmaBase.it_ht_flag);

//		M2Mlow_Status=M2M_FREE;
////		API_DMA_M2M_OverCallback();					//M2M 傳遞完成

//	}
//}


//void	API_ADC_M2M_COPY(uint32_t* aSRC_Buffer,uint32_t* aDST_Buffer,uint32_t length)
//{
//		while(M2M_Status!=M2M_FREE)
//		{}
//			M2M_Status=M2M_BUSY;
//			if(HAL_DMA_Start_IT(&M2M_dma, (uint32_t)aSRC_Buffer, (uint32_t)aDST_Buffer, length) != HAL_OK)
//			{
//        /* Transfer Error */
//        Error_Handler();
//			}
//	
//}

void API_DMA_RecoverBase(API_DMA_InitTypeDef *dmaBase,API_DMA_RecoverDef* recover)
{
	DMA_HandleTypeDef*	hdma=dmaBase->handle;
	//停止DMA传输
	__HAL_DMA_DISABLE(hdma);

	//清除所有DMA标志位
	__HAL_DMA_CLEAR_FLAG(hdma,dmaBase->it_tc_flag|dmaBase->it_te_flag|dmaBase->it_ht_flag);

		hdma->Instance->CNDTR = (uint32_t)recover->DataLength;					//设置数据长度

	//设置新的数据长度
	if(recover->DataLength)														//数据长度为0，关DMA
	{
	
		if(recover->DstAddress)
		{
			hdma->Instance->CMAR = (uint32_t)recover->DstAddress;				//目标地址
		}
		if(recover->SrcAddress)
		{	
			hdma->Instance->CPAR = (uint32_t)recover->SrcAddress;				//源地址
		}
		//重新启动DMA
		__HAL_DMA_ENABLE(hdma);
	}
}

void		API_DMA_InitBase(API_DMA_InitTypeDef *dmaBase)
{
	
	  /* DMA controller clock enable */
		__HAL_RCC_DMAMUX1_CLK_ENABLE();
		__HAL_RCC_DMA1_CLK_ENABLE();
		__HAL_RCC_DMA2_CLK_ENABLE();	
		DMA_HandleTypeDef*	dma_handel=dmaBase->handle;
		
		dma_handel->Instance=dmaBase->Instance;
		dma_handel->Init=dmaBase->Init;
		
		HAL_DMA_Init(dma_handel);		
	
		__HAL_DMA_CLEAR_FLAG(dma_handel,dmaBase->it_ht_flag|dmaBase->it_tc_flag|dmaBase->it_te_flag);

		if(dmaBase->it)				//0:不需要中断
		{	
			__HAL_DMA_ENABLE_IT(dma_handel,dmaBase->it);
		
			HAL_NVIC_SetPriority(dmaBase->irq, dmaBase->priority, 0);
			HAL_NVIC_EnableIRQ(dmaBase->irq);
		}
	
}
void		API_DMA_START(API_DMA_CH_ENUM ch)
{
		__HAL_DMA_ENABLE(DmaBaseStr[ch]->handle);
}	
void		API_DMA_STOP(API_DMA_CH_ENUM ch)
{
		__HAL_DMA_DISABLE(DmaBaseStr[ch]->handle);
}	

void	API_DMA_REST(uint8_t ch,uint16_t size)
{
		__HAL_DMA_DISABLE(DmaBaseStr[ch]->handle);
		DmaBaseStr[ch]->handle->Instance->CNDTR=size;
		__HAL_DMA_ENABLE(DmaBaseStr[ch]->handle);
}	



void	API_DMA_Init(void)
{
	
	for(uint8_t i=0;i<ChDmaMax;i++)
	{
			API_DMA_InitBase(DmaBaseStr[i]);
	}
	M2M_Status=M2M_FREE;
	M2Mlow_Status=M2M_FREE;
}	
void		API_DMA_RECOVER(API_DMA_CH_ENUM ch,API_DMA_RecoverDef* recover)
{
		if(recover->DataLength)
		{	
	
			switch(ch)
			{	
				case	ChDmaM2M:

			
					while(M2M_Status!=M2M_FREE);
					M2M_Status=M2M_IN;
					API_DMA_RecoverBase((DmaBaseStr[ch]),recover);			
					while(M2M_Status==M2M_IN);
					while(DmaBaseStr[ch]->handle->Instance->CNDTR);
					break;
				
//				case		ChDmaM2Mlow:
//				
//					while(M2Mlow_Status!=M2M_FREE);
//					M2Mlow_Status=M2M_IN;

//					API_DMA_RecoverBase((DmaBaseStr[ch]),recover);			
//					while(M2Mlow_Status==M2M_IN);

//				

//				break;
			default:
							API_DMA_RecoverBase((DmaBaseStr[ch]),recover);		
				break;
		}	
	}
	else
	{
		
				API_DMA_STOP(ch);			
	
	}
	


	
}

#if 0
void		API_DMA_SetConfing(uint8_t ch)
{
		if(ch==ADC_Select_VcIc)
		{	


			API_ADC_SetConfing_DMA(&AdcT12aHandle, (uint32_t *)TxA_ADC_DmaBuff.T12A, TxA_ADC_DMA_BUFF_NUM);
			API_ADC_SetConfing_DMA(&AdcT34aHandle, (uint32_t *)TxA_ADC_DmaBuff.T34A, TxA_ADC_DMA_BUFF_NUM);
			API_ADC_SetConfing_DMA(&AdcVcIcHandle, (uint32_t *)VcIc_ADC_Buff, 		VcIc_ADC_GROUP_NUM);
			
//			__HAL_DMA_ENABLE_IT(&AdcT12A_dma, (DMA_IT_TC | DMA_IT_HT | DMA_IT_TE));	
		}

		if(ch==ADC_Select_Tempe)
		{		
	
			API_ADC_SetConfing_DMA(&AdcT12aHandle, (uint32_t *)&Tempe_ADC_Buff, TxA_ADC_GROUP_NUM);
			API_ADC_SetConfing_DMA(&AdcT34aHandle, (uint32_t *)&res_ADC_Buff, TxA_ADC_GROUP_NUM);
			API_ADC_SetConfing_DMA(&AdcVcIcHandle, (uint32_t *)&FanAd_ADC_Buff, VcIc_ADC_GROUP_NUM);
		}

}



		if(ch==ADC_Select_VcIc)
		{	
			API_ADC_DMA_Recover(&AdcT12aHandle, (uint32_t *)TxA_ADC_DmaBuff.T12A, TxA_ADC_DMA_BUFF_NUM);
			API_ADC_DMA_Recover(&AdcT34aHandle, (uint32_t *)TxA_ADC_DmaBuff.T34A, TxA_ADC_DMA_BUFF_NUM);
			API_ADC_DMA_Recover(&AdcVcIcHandle, (uint32_t *)VcIc_ADC_Buff, VcIc_ADC_GROUP_NUM);
//			__HAL_DMA_ENABLE_IT(&AdcT12A_dma, (DMA_IT_TC | DMA_IT_HT | DMA_IT_TE));				
		}
		if(ch==ADC_Select_Tempe)
		{	
			API_ADC_DMA_Recover(&AdcT12aHandle, (uint32_t *)&Tempe_ADC_Buff, TxA_ADC_GROUP_NUM);
			API_ADC_DMA_Recover(&AdcT34aHandle, (uint32_t *)&res_ADC_Buff, TxA_ADC_GROUP_NUM);
			API_ADC_DMA_Recover(&AdcVcIcHandle, (uint32_t *)&FanAd_ADC_Buff, VcIc_ADC_GROUP_NUM);
//			__HAL_DMA_DISABLE_IT(&AdcT12A_dma, (DMA_IT_TC | DMA_IT_HT | DMA_IT_TE));				
		}		



#endif






#if 0
uint8_t 	API_DMA_M2M_FROM_VcIc(uint32_t*	aDST_Buffer,uint32_t buff_size)		//读取ADC2的缓存到DST
{	
		uint32_t* aSRC_Buffer=(uint32_t*)&VcIc_ADC_Buff;
		if(M2M_Status==M2M_FREE)
		{
			M2M_Status=M2M_VCIC_START;		//开始VCIC转换
			if(HAL_DMA_Start_IT(&M2M_dma, (uint32_t)aSRC_Buffer, (uint32_t)aDST_Buffer, buff_size) != HAL_OK)
			{
        /* Transfer Error */
        Error_Handler();
				M2M_Status=M2M_ERR;
			}
			uint16_t delay=0;
			do
			{
					if(M2M_Status==M2M_VCIC_END)
					{		
						return M2M_Status;
					}	
			}while(delay++<1000);

		
		}
		M2M_Status=M2M_ERR;
		return M2M_Status;
}



uint8_t 	API_DMA_M2M_FROM_T34A(uint32_t*	aDST_Buffer,uint32_t buff_size)		//读取ADC1 ADC3的缓存到DST
{	
		TxA_ADC_Buff_DEF* aSRC_Buffer=&TxA_ADC_DmaBuff;
		if(M2M_Status==M2M_FREE)
		{	
			M2M_Status=M2M_TxA_START;		//开始VCIC转换	

//			if(__HAL_DMA_GET_COUNTER(&M2M_dma)<TxA_ADC_GROUP_NUM)		//根据当前DMA COUNT确定ADC读取完成的半区
//			{
//			M2M_Status+=TxA_ADC_GROUP_NUM;
//			}		
	
			if(HAL_DMA_Start_IT(&M2M_dma, (uint32_t)aSRC_Buffer, (uint32_t)aDST_Buffer, buff_size) != HAL_OK)
			{
        /* Transfer Error */
        Error_Handler();
				M2M_Status=M2M_ERR;
			}
			uint16_t delay=0;
			do
			{
					if(M2M_Status==M2M_TxA_END)
					{		
						return M2M_Status;
					}	
			}while(delay++<1000);


		}	
		M2M_Status=M2M_ERR;
		
		return M2M_Status;
}





#endif




void	API_DMA1_Current_IRQHandler(void)
{

//	HAL_DMA_IRQHandler(&AdcT12A_dma);
	DMA_HandleTypeDef* hdma=HrtimPotCh1_DmaBase.handle;
	
	uint32_t flag_it = hdma->DmaBaseAddress->ISR;
	
//	__HAL_DMA_CLEAR_FLAG(hdma,DMA_FLAG_TC7|DMA_FLAG_TE7|DMA_FLAG_HT7);
//	hdma->DmaBaseAddress->IFCR = ((uint32_t)DMA_ISR_HTIF1 << (hdma->ChannelIndex & 0x1FU));
		hdma->DmaBaseAddress->IFCR = ((uint32_t)DMA_ISR_HTIF7 |DMA_ISR_GIF7|DMA_ISR_TCIF7|DMA_ISR_TEIF7);
	
//	if(flag_it & ((uint32_t)DMA_FLAG_HT1 << (hdma->ChannelIndex & 0x1FU)))
//	{	

		API_DMA_TxA_IRQHandlerCallBack(0);					//半缓存区满   回调

//		
//	}
//	if(flag_it & ((uint32_t)DMA_FLAG_TC1 << (hdma->ChannelIndex & 0x1FU)))
//	{	


//		API_DMA_TxA_IRQHandlerCallBack(1);					//全缓存区满 回调
	
//	}
		
}		


// void	API_DMA2_Channel7_IRQHandler(void)
// {

// 	DMA_HandleTypeDef* hdma=&AdcT1A_dma;
	
// 	// uint32_t flag_it = hdma->DmaBaseAddress->ISR;
	
	
// 	hdma->DmaBaseAddress->IFCR = ((uint32_t)DMA_ISR_GIF1 << (hdma->ChannelIndex & 0x1FU));

		
// 	// if(flag_it&DMA_FLAG_HT_Pan)
// 	// {	

// 	// 	API_DMA_T1A_IRQHandlerCallBack(DMA_HT);					//半缓存区满   回调
		
// 	// }

// 	// if(flag_it&DMA_FLAG_TC_Pan)	
// 	// {	

// 		API_DMA_T1A_IRQHandlerCallBack(DMA_TC);					//全缓存区满 回调
// 	// }
		
// }
void	API_DMA2_PAN_IRQHandler(void)
{

//	HAL_DMA_IRQHandler(&AdcT12A_dma);
	DMA_HandleTypeDef* hdma=Pan_DmaBase.handle;
	
	uint32_t flag_it = hdma->DmaBaseAddress->ISR;
//  uint32_t source_it = hdma->DmaBaseAddress->IFCR;	
	
	
//	__HAL_DMA_CLEAR_FLAG(hdma,DMA_FLAG_TC1|DMA_FLAG_TE1|DMA_FLAG_HT1);
	hdma->DmaBaseAddress->IFCR = ((uint32_t)DMA_ISR_GIF1 << (hdma->ChannelIndex & 0x1FU));
//	hdma->DmaBaseAddress->IFCR |= ((uint32_t)DMA_ISR_HTIF1 |DMA_ISR_GIF1|DMA_ISR_TCIF1|DMA_ISR_TEIF1);

//	if(flag_it & ((uint32_t)DMA_FLAG_HT1 << (hdma->ChannelIndex & 0x1FU)))
		
//	if(flag_it&DMA_FLAG_HT_Pan)
//	{	

//		API_DMA_PAN_IRQHandlerCallBack(1);					//半缓存区满   回调
//		API_GPIO_WritePin(DebugA_pin,0);
//		
//	}
//	if(flag_it & ((uint32_t)DMA_FLAG_TC1 << (hdma->ChannelIndex & 0x1FU)))
	if(flag_it&DMA_FLAG_TC_Pan)	
	{	

	
		API_DMA_PAN_IRQHandlerCallBack(0);					//全缓存区满 回调
	
	}
		
}		




