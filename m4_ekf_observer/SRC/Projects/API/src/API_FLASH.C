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
* File         : main.c
* By           : RX_DV_Team
*********************************************************************************************************
*/


/* Includes ------------------------------------------------------------------*/

#include "rx32g4xx_config_def.h"
#include "rx32g4xx_hal.h"
#include	"API_FLASH.H"
//#include "system_init.h"
//#include "system_bsp.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
//#define FLASH_USER_START_ADDR   ADDR_FLASH_PAGE_11                           /* Start @ of user Flash area */
//#define FLASH_USER_END_ADDR     (ADDR_FLASH_PAGE_11 + FLASH_PAGE_SIZE - 1)   /* End @ of user Flash area */






//一次16字节=4个uint32_t
static uint32_t Program_Data_128[128] = 
{
    0x00000000, 0x11111111, 0x22222222, 0x33333333, 0x44444444, 0x55555555, 0x66666666, 0x77777777,
    0x88888888, 0x99999999, 0xAAAAAAAA, 0xBBBBBBBB, 0xCCCCCCCC, 0xDDDDDDDD, 0xEEEEEEEE, 0x0F0F0F0F,
    0x00000000, 0x11111111, 0x22222222, 0x33333333, 0x44444444, 0x55555555, 0x66666666, 0x77777777,
    0x88888888, 0x99999999, 0xAAAAAAAA, 0xBBBBBBBB, 0xCCCCCCCC, 0xDDDDDDDD, 0xEEEEEEEE, 0x0F0F0F0F,
    0x00000000, 0x11111111, 0x22222222, 0x33333333, 0x44444444, 0x55555555, 0x66666666, 0x77777777,
    0x88888888, 0x99999999, 0xAAAAAAAA, 0xBBBBBBBB, 0xCCCCCCCC, 0xDDDDDDDD, 0xEEEEEEEE, 0x0F0F0F0F,
    0x00000000, 0x11111111, 0x22222222, 0x33333333, 0x44444444, 0x55555555, 0x66666666, 0x77777777,
    0x88888888, 0x99999999, 0xAAAAAAAA, 0xBBBBBBBB, 0xCCCCCCCC, 0xDDDDDDDD, 0xEEEEEEEE, 0x0F0F0F0F,
    0x00000000, 0x11111111, 0x22222222, 0x33333333, 0x44444444, 0x55555555, 0x66666666, 0x77777777,
    0x88888888, 0x99999999, 0xAAAAAAAA, 0xBBBBBBBB, 0xCCCCCCCC, 0xDDDDDDDD, 0xEEEEEEEE, 0x0F0F0F0F,
    0x00000000, 0x11111111, 0x22222222, 0x33333333, 0x44444444, 0x55555555, 0x66666666, 0x77777777,
    0x88888888, 0x99999999, 0xAAAAAAAA, 0xBBBBBBBB, 0xCCCCCCCC, 0xDDDDDDDD, 0xEEEEEEEE, 0x0F0F0F0F,
    0x00000000, 0x11111111, 0x22222222, 0x33333333, 0x44444444, 0x55555555, 0x66666666, 0x77777777,
    0x88888888, 0x99999999, 0xAAAAAAAA, 0xBBBBBBBB, 0xCCCCCCCC, 0xDDDDDDDD, 0xEEEEEEEE, 0x0F0F0F0F,
    0x00000000, 0x11111111, 0x22222222, 0x33333333, 0x44444444, 0x55555555, 0x66666666, 0x77777777,
    0x88888888, 0x99999999, 0xAAAAAAAA, 0xBBBBBBBB, 0xCCCCCCCC, 0xDDDDDDDD, 0xEEEEEEEE, 0x0F0F0F0F
};

static uint32_t readData[128] = {0};



/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
//uint32_t FirstPage = 0, NbOfPages = 0, BankNumber = 0;
//uint32_t Address = 0, PageError = 0;
//__IO uint32_t MemoryProgramStatus = 0;
//__IO uint32_t data32 = 0;

///*Variable used for Erase procedure*/
//static FLASH_EraseInitTypeDef EraseInitStruct;

/* Private function prototypes -----------------------------------------------*/
//static uint32_t GetPage(uint32_t Address);
//static uint32_t GetBank(uint32_t Address);

__weak		void Error_Handler(void)
{}	


/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/*
 * main: initialize and start the system
 */
void	 API_FLASH_INIT(void)
{
 
    
    /* Unlock the Flash to enable the flash control register access *************/
    HAL_FLASH_Unlock();

    /* Clear OPTVERR bit set on virgin samples */
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR_RX);


    HAL_FLASH_Lock();
 

}




// 擦除最后一个页
void Flash_Erase_Last_Page(void) 
{
    FLASH_EraseInitTypeDef eraseConfig;
    uint32_t pageError = 0;
    
    eraseConfig.TypeErase = FLASH_TYPEERASE_PAGES;
    eraseConfig.Banks = FLASH_BANK_2;  
    eraseConfig.Page = 100;  // 直接指定页号
    eraseConfig.NbPages = 1; // 擦除1页
    
    if (HAL_FLASHEx_Erase(&eraseConfig, &pageError) != HAL_OK) 
    {
        Error_Handler();
    }
}

// 写入16字节数据到指定地址（128位对齐）
void Flash_Write(uint32_t address, uint32_t *data,uint32_t size) 
{
    // 确保地址对齐到16字节边界
    if (address % 16 != 0) 
    {
        Error_Handler();
    }
    
    // Program
    for (uint32_t i=0; i<size; i+=4)
    {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEDOUBLEWORD, (address+i*4), &(data[i])) != HAL_OK)
        {
            Error_Handler();
        }
        
    }  
}

// 从指定地址读取16字节数据
void Flash_Read(uint32_t address, uint32_t *buffer,uint32_t size) 
{
    // 确保地址对齐到16字节边界
    if (address % 16 != 0) 
    {
        Error_Handler();
    }
        
    for (uint32_t i=0 ; i<size ; i++)
    {
        (buffer[i]) = *(__IO uint32_t *)(address + i*4);        
    }  
    
}

#if 0
void update_metadata(uint32_t tail) {
    // 1. 切换写入扇区
    uint32_t target_addr = (active_sector == 0) ? 
                          META_SECTOR_1_ADDR : META_SECTOR_0_ADDR;
    
    // 2. 准备数据
    QueueMetaData meta;
    meta.tail = tail;
//    meta.checksum = calculate_crc32(&tail, sizeof(tail));
    
    // 3. 擦除目标扇区
//    flash_erase_sector(target_addr);
    
    // 4. 写入新元数据
//    flash_write(target_addr, &meta, sizeof(meta));
    
    // 5. 更新激活扇区标识
    active_sector = !active_sector;
}


#define FLASH_BASE_ADDR   0x8000000
#define BLOCK_SIZE        (512 * 1024)    // 512KB
#define UNIT_SIZE         16              // 16字节/单元
#define MAX_UNITS         (BLOCK_SIZE / UNIT_SIZE)  // 32768单元

uint32_t queue_tail = 0;  // 当前队尾指针

void init_queue() {
    // 尝试从两个元数据扇区加载有效数据
    QueueMetaData meta;
    if(load_metadata(META_SECTOR_0_ADDR, &meta)) {
        queue_tail = meta.tail;
    } else if(load_metadata(META_SECTOR_1_ADDR, &meta)) {
        queue_tail = meta.tail;
    } else {
        queue_tail = 0;  // 默认初始化
    }
    
    // 检查队列满状态
    if(queue_tail >= MAX_UNITS) {
        flash_erase_block(FLASH_BASE_ADDR);
        queue_tail = 0;
        update_metadata(0);
    }
}

void write_data(uint8_t* data) {
    // 1. 计算写入地址
    uint32_t addr = FLASH_BASE_ADDR + (queue_tail * UNIT_SIZE);
    
    // 2. 写入FLASH（需确保16字节原子写入）
    flash_write(addr, data, UNIT_SIZE);
    
    // 3. 更新队尾指针
    queue_tail++;
    
    // 4. 检查块是否写满
    if(queue_tail >= MAX_UNITS) {
        flash_erase_block(FLASH_BASE_ADDR);
        queue_tail = 0;
    }
    
    // 5. 持久化元数据
    update_metadata(queue_tail);
}

void read_latest(uint8_t* buffer, uint32_t n) {
    // 从队尾向前读取n个单元
    for(int i=0; i<n; i++) {
        // 计算读取位置（处理循环）
        uint32_t read_idx = (queue_tail - 1 - i + MAX_UNITS) % MAX_UNITS;
        uint32_t addr = FLASH_BASE_ADDR + (read_idx * UNIT_SIZE);
        
        // 从FLASH读取数据
        flash_read(addr, &buffer[i*UNIT_SIZE], UNIT_SIZE);
    }
}
#endif