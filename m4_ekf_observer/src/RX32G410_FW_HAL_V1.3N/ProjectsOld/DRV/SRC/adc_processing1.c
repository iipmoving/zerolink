// adc_processing.c
#include "adc_processing.h"

// 确保32字节对齐 (缓存行对齐)
__attribute__((aligned(32)))
//ADC_Buffer_t TxA_ADC_TimDmaBuff;

// 定义在汇编中使用的block_idx
__attribute__((section(".data")))
uint8_t block_idx = 0;

