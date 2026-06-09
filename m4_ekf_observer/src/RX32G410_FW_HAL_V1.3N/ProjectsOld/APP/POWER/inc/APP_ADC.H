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
* File         : DRV_MCU.h
* By           : MOVING
*********************************************************************************************************
*/

#ifndef __APP_ADC_H__
#define __APP_ADC_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * @file    APP_ADC.H
 * @brief   ADC 数据采集公共接口 (待拆分)
 * @layer   APP (FIXME: 含废弃枚举和 DRV 混合声明, 待随 .c 拆分逐步清理)
 * @deps    <stdio.h> <stdint.h> <stdbool.h>
 *
 * FIXME: include guard 使用正常 #define, 拆分后的新模块应使用 //#define (L0阻断)
 */


	

/* === INTERFACE STRUCTS (OWNER) ==================================
 * 本模块是以下结构体的 Owner (生产者).
 * @STRUCT 标记由 check_structs.py 解析验证.
 *
 * 消费者: app_power 需声明 Power_AdcOut_IN_t 副本 (AI-MANAGED 段)
 * ================================================================ */

#pragma pack(4)

/* @STRUCT AdcPowerOut  owner=app_adc  suffix=OUT */
typedef struct {
    uint16_t power[4];         /* offset=0, size=8 */
    void     *sync_addr;       /* offset=8, size=4 */
    uint8_t  dma_ready;        /* offset=12, size=1 */
} AdcPowerOut;                 /* sizeof=16 */

/* @STRUCT AdcRawInput  owner=app_adc  suffix=OUT */
typedef struct {
    uint32_t txa_current[4];   /* offset=0, size=16, AdcGroupT1A..T4A */
    uint32_t voltage;           /* offset=16, size=4, AdcGroupVoltage */
    uint32_t phase[4];          /* offset=20, size=16, AdcGroupPhase1..4 */
    uint32_t ceil_q[4];         /* offset=36, size=16, AdcGroupCeilQ1..4 */
} AdcRawInput;                  /* sizeof=52 */

/* @STRUCT APP_ADC_AWD_DNTR_DEF  owner=app_adc  suffix=OUT */
typedef struct {
    uint8_t  num;                  /* offset=0, size=1 */
    uint8_t  ch;                   /* offset=1, size=1 */
    uint16_t res;                  /* offset=2, size=2 */
    uint32_t dntr[5];              /* offset=4, size=20 */
} APP_ADC_AWD_DNTR_DEF;            /* sizeof=24 */

#pragma pack()

/* === END INTERFACE STRUCTS === */

/* Exported typedef ----------------------------------------------------------*/
enum
{																				//ADC1 group 对应的AD采集通道号，保存数据的序号索引
	HRTIM_Upd=1,		//HRTim更新的位置  HRTIM_CNTR=0
	HRTIM_HighOn,		//上管开通位置（上管死区后）
	HRTIM_HighOff,			//上管关断位置（上管死区后）
	HRTIM_LowOn,		//下管开通位置（下管死区后）
	HRTIM_LowOff,			//下管关断位置（上管死区后）
	HRTIM_Max,
	
};	//hrtim buff index
enum
{																				//ADC1 group 对应的AD采集通道号，保存数据的序号索引

	


		AdcGroupPower1=0,		//MASTER CURRENT AD
		AdcGroupPower2,			//SLAVE CURRENT AD	
		AdcGroupPower3,			//MASTER CURRENT AD
		AdcGroupPower4,			//SLAVE CURRENT AD		
	
	
		AdcGroupVoltage,

		AdcGroupT1A,					//谐振电流
		AdcGroupT2A,					//slave
		AdcGroupT3A,					//master
		AdcGroupT4A,					//slave
	
		AdcGroupBottom1,
		AdcGroupBottom2,
		AdcGroupBottom3,
		AdcGroupBottom4,

		AdcGroupIgbt1,
		AdcGroupIgbt2,
		AdcGroupIgbt3,
		AdcGroupIgbt4,		
		
		AdcGroupFan,
		
		AdcGroupCeilQ1,			//MASTER Q值 AD
		AdcGroupCeilQ2,			//SLAVE Q值 AD	
		AdcGroupCeilQ3,			//MASTER Q值 AD
		AdcGroupCeilQ4,			//SLAVE Q值 AD		

		AdcGroupPhase1,			//MASTER 相位 AD
		AdcGroupPhase2,			//SLAVE 相位 AD	
		AdcGroupPhase3,			//MASTER 相位 AD
		AdcGroupPhase4,			//SLAVE 相位 AD	

		AdcGroupPhaseDown1,			//MASTER 相位 AD
		AdcGroupPhaseDown2,			//SLAVE 相位 AD	
		AdcGroupPhaseDown3,			//MASTER 相位 AD
		AdcGroupPhaseDown4,			//SLAVE 相位 AD	

		AdcGroupMax,	
	
};	

#define		HRTIM_PER1US		0X300		//1us对应的HRTIM计数值
#define	APP_ADC_DNTR_BUFF_MAX 5		//AWD缓存最大值

enum
{
	DNTR_T12A=0,
	DNTR_T34A,
};	

enum
{
	chT1A=0,
	chT2A,
	chT3A,
	chT4A,
};	
enum
{
	pluseStart=0,
	pluseHighOn,
	pluseHighOff,
	pluseLowOn,
	pluseLowOff,
	pluseDown,
	pluseLowScr,
	pluseEnd,
};



/* Exported define -----------------------------------------------------------*/
	
#define		DIV_VALUE		1		//一阶滤波系数1/8
#define		DIV_BASE		16		//一阶滤波系数

#define		DIV_VALUE_POWER		1		//一阶滤波系数1/8
#define		DIV_BASE_POWER		32		//一阶滤波系数

#define		DIV_VALUE_Q		1		//一阶滤波系数1/8
#define		DIV_BASE_Q		2		//一阶滤波系数


#define		DMA_HRTIM		0		//ADC DMA读取HRTIM值，用于测试TIM3 DMA位置准确
			
#define	HRTIM_OFFSET		3			//TIM hrtim Dma 与ADC DMA的偏移

/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/


/**
  * @brief synchro start 初始化表格
  * @param 多个互补输出TIMX同步定时器初始化
  * @retval None
  */







/* Exported function prototypes ----------------------------------------------*/
//void			APP_ADC_ZERO_IrqFun(void);					//每100us进行ADC求和
uint8_t		AdcValueFun(void);					//统一处理ADC值 1:有效值输出
//void		APP_ADC_AVG_Fun(void);			//每20ms 得到一组ADC数据
//uint8_t		getIcVc20msOccur(void);
//void		clrIcVc20msOccur(void);
//uint32_t  	getADCinputValue(uint8_t ch);
//void		APP_ADC_DMA_RecoverPan(uint8_t ch);				//恢复检锅DMA
void		API_ADC_DMA_RecoverTxa(void);				//恢复检锅DMA


//void		APP_ADC_PanSwChange(uint32_t ch);			//切换检锅选择口
uint32_t 	APP_ADC_GetPowerTxa(uint8_t ch);		//得到谐振电流计算的功率值

void CopyISRToRAM(void);

int16_t*		getIcValueAdress(void);
uint16_t*		APP_ADC_GetHrtimAddress(void);

void  			APP_ADC_TimDmaEnd(void);			//得到100ms提取 DMA的结束标志进行TXA滤波求和处理
uint32_t *  		getAdcBuffAddress(uint8_t ch);
uint32_t   		getAdcBuffSize(void);
uint32_t   		getAdcBuffSum(uint8_t ch);
void			API_ADC_DMA_RecoverTxaPan(void);
void			API_ADC_DMA_TimStart(void);			//启动TXA DMA  TIM3 

/* 外部回調函數聲明 ----------------------------------------------*/
uint8_t 	APP_ADC_SetTXAstepDmaHrtimEnd(void);	//在HRTIM CMP4 第二个中断中结束DMA
void	APP_ADC_SetTXAstepDmaRest(void);
void	APP_ADC_TxaAvgReset(uint8_t ch);			//滤波初始化
void	APP_ADC_ClearCeilQAvg(uint8_t ch);		//重置CEILQ的滤波，防止倍频切换时误判

//void	APP_ADC_WaitTxaCalOver(void);		//等待TXA DMA采集完成，防止与检锅冲突

uint16_t *	APP_ADC_GetHrtimSyncBuffAdr(void);		//得到多炉头同一点的HRTIM值 
void	APP_ADC_CalculatePower(void);

uint8_t 	APP_ADC_IsTxaDmaStart(void);			//DMA TXA时不调整PPG

#ifdef __cplusplus
}
#endif

#endif 

//__APP_ADC_H__ end
