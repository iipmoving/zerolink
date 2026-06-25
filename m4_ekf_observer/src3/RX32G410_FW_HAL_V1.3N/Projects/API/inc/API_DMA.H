#ifndef __API_DMA_H__
#define __API_DMA_H__

#include	<stdint.h>


enum{				
	M2M_FREE=0,			//M2M状态
	M2M_IN,					//初始化
	M2M_TXA,				//TXA數據緩存
};								//M2M转换状态	


#ifdef	COMM_UART
#define	DMA_REQUEST_UARTx_RX		DMA_REQUEST_UART3_RX		// uart DMA触发源
#define	DMA_REQUEST_UARTx_TX		DMA_REQUEST_UART3_TX
#else
#define	DMA_REQUEST_UARTx_RX		DMA_REQUEST_UART2_RX		// uart DMA触发源
#define	DMA_REQUEST_UARTx_TX		DMA_REQUEST_UART2_TX
#endif
typedef struct{


	uint32_t SrcAddress; 
	uint32_t DstAddress;
  uint32_t DataLength;
	uint32_t res; 

}API_DMA_RecoverDef;



#if 1
typedef	enum
{
	ChDmaM2M=0,
	ChDmaVcAdc1,	
	ChDmaCurrentAdc2,	//2
	ChDmaCurrentAdc3,	
	ChDmaPan,
	ChDmaTx,
	ChDmaRx,	

	ChDmaHrtimPotCh1,
	ChDmaHrtimPotCh2,
	ChDmaHrtimPotCh3,
	ChDmaHrtimPotCh4,



	ChDmaMax,	
}API_DMA_CH_ENUM;	
#else
typedef	enum
{
	ChDmaM2M=0,
	ChDmaHrtim1us,		//HRTIM起点位置  
	ChDmaPan,
	ChDmaTx,
	ChDmaRx,	

	ChDmaCurrent1,
	ChDmaCurrent2,
	ChDmaCurrent3,	
	ChDmaCurrent4,				
	ChDmaHrtim,		
	ChDmaMax,	



}API_DMA_CH_ENUM;	


#endif
typedef	enum
{
	DMA_HT=0,			//DMA半缓存中断
	DMA_TC,				//DMA全缓存中断
}API_DMA_IT_ENUM;	


void	API_DMA_REST(uint8_t ch,uint16_t size);			//将DMA数据恢复
void		API_DMA_Init(void);														//M2M	 初始化
void		API_DMA_STOP(API_DMA_CH_ENUM ch);
void		API_DMA_START(API_DMA_CH_ENUM ch);													//开启M2M DMA
void		API_DMA_RECOVER(API_DMA_CH_ENUM ch,API_DMA_RecoverDef* recover);	//恢复M2M DMA指针
uint32_t 	API_DMA_GetDmaCndtr(API_DMA_CH_ENUM ch);				//得到当前DMA保存的位置
uint32_t* 	API_DMA_GetDmaCndtrAddress(API_DMA_CH_ENUM ch);//得到当前DMA保存的位置的地址
uint32_t*	API_DMA_GetDmaHandle(API_DMA_CH_ENUM ch);	//得到DMA的HANDLE

#endif			//__API_DMA_H__
