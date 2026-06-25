


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
* File         : main.h
* By           : RX_DV_Team
*********************************************************************************************************
*/

#ifndef __API_FMAC__
#define __API_FMAC__


#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
//#include "rx32g4xx.h"


/* Exported typedef ----------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Configuration: uncomment in order to use the clipping feature */
#define CLIP_ENABLED

/* Not called/called status */
#define DATA_NOT_READY          0
#define DATA_READY              1

/* Size of the data arrays */
#define ARRAY_SIZE              508

/* Size of the preloaded input data array (size <= INPUT_BUFFER_SIZE) */
#define INPUT_ARRAY_1_SIZE      120

/* Size of the input data arrays written during calculation */
#define INPUT_ARRAY_2_SIZE      120
#define INPUT_ARRAY_3_SIZE      110
#define INPUT_ARRAY_4_SIZE      182

/* Expected number of calls to HAL_FMAC_GetDataCallback */
#define GET_DATA_CALLBACK_COUNT  1

/* Size of the output data arrays read during calculation
   Total expected size: input array size - number of feed-forward taps */
#define OUTPUT_ARRAY_1_SIZE     520
#define OUTPUT_ARRAY_2_SIZE     249

/* Expected number of calls to HAL_FMAC_OutputDataReadyCallback */
#define DATA_RDY_CALLBACK_COUNT 1

#ifdef	Fmac5
#define		FmacLeve		5
#endif

#ifdef	Fmac9
#define		FmacLeve		9
#endif

#ifdef	Fmac21
#define		FmacLeve		21
#endif

/* Filter parameter P: number of feed-forward taps or coefficients in the range [2:127] */
#define COEFF_VECTOR_B_SIZE     FmacLeve

/* Filter parameter Q: not used */
#define FILTER_PARAM_Q_NOT_USED 0

/* Filter parameter R: gain in the range [0:7] */
#define GAIN                   0 



/* Throughput parameter: extra space in the input buffer (minimum: 0) */
#define MEMORY_PARAMETER_D1    49			// 122

/* Throughput parameter: extra space in the output buffer (minimum: 1) */
#define MEMORY_PARAMETER_D2     100

/* Throughput parameter: watermark threshold for the input buffer */
#define INPUT_THRESHOLD         FMAC_THRESHOLD_1

/* Throughput parameter: watermark threshold for the output buffer (inferior or equal to MEMORY_PARAMETER_D1) */
#define OUTPUT_THRESHOLD        FMAC_THRESHOLD_1



/* FMAC internal memory configuration: base address of the coefficient buffer */
#define COEFFICIENT_BUFFER_BASE 0

/* FMAC internal memory configuration: size of the coefficient buffer */
#define COEFFICIENT_BUFFER_SIZE COEFF_VECTOR_B_SIZE

/* FMAC internal memory configuration: base address of the input buffer */
#define INPUT_BUFFER_BASE       COEFFICIENT_BUFFER_SIZE

/* FMAC internal memory configuration: size of the input buffer */
#define INPUT_BUFFER_SIZE       COEFF_VECTOR_B_SIZE + MEMORY_PARAMETER_D1

/* FMAC internal memory configuration: base address of the input buffer */
#define OUTPUT_BUFFER_BASE      COEFFICIENT_BUFFER_SIZE + INPUT_BUFFER_SIZE

/* FMAC internal memory configuration: size of the input buffer */
#define OUTPUT_BUFFER_SIZE      MEMORY_PARAMETER_D2


enum			//FMAC运算的内容
{
	FmacStop=0,		//停止
	FmacPan,			//检锅
	FmacTxa,			//谐振电流计算
};	


typedef	struct
{
		int16_t 	* X1;		//输入数据地址
		int16_t 	* Y;		//输出数据地址
		int16_t		outSize;		//输出数据大小
		int8_t 		ch;
		int8_t 		type;			//运算内容
//-----以下内部使用--------------------------
		int16_t 	* X2;		//滤波器数据地址
		int16_t 	coeffBsize;	//滤波器数据地址
		int16_t 	res16;
}API_FMAC_MEMDEF;

typedef	struct
{
		int16_t 	* X1;		//输入数据地址
		int16_t 	* Y;		//输出数据地址
		int16_t		outSize;		//输出数据大小
		int8_t 		ch;
		int8_t 		res;
		
}API_FMAC_PublicMemDEF;

/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported function prototypes ----------------------------------------------*/
//void	API_FMAC_GetMemAddress(API_FMAC_MEMDEF* FMAC_MEM);
//void	 API_FMAC_INIT(void);

void	API_FMAC_Rest(void);

API_FMAC_MEMDEF*	API_FMAC_GetMemAddress(void);		//返回FMAC 初始设置地址

int16_t*   API_FMAC_InputValueAdress(void);
uint32_t  API_FMAC_GetWdateAddress(void);

#ifdef __cplusplus
}
#endif

#endif /* __API_FMAC__*/



