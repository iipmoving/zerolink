#ifndef __API_FLASH_H__
#define __API_FLASH_H__


#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
//#include "rx32g4xx.h"
//#include "rx32g4xx_hal_flash.h"
//#include "rx32g4xx_hal_flash_ex.h"

/* Exported typedef ----------------------------------------------------------*/
typedef struct {
    uint32_t tail;      // 队尾指针（0~32768）
    uint32_t checksum;  // CRC32校验值（保护tail数据）
} QueueMetaData;

	
/* Exported define -----------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
#define ADDR_FLASH_PAGE_0     ((uint32_t) 0x08000000)     /* Base @ of Page 0,   1 Kbytes */
#define ADDR_FLASH_PAGE_11    ((uint32_t) 0x08002C00)     /* Base @ of Page 11,  1 Kbytes */
#define ADDR_FLASH_PAGE_511   ((uint32_t) 0x0807FC00)     /* Base @ of Page 511, 1 Kbytes */

#define		FLASH_START					0x55AABBCC			//页面初始化引导符	
	
#define META_SECTOR_0_ADDR 0x1000  // 元数据扇区1
#define META_SECTOR_1_ADDR 0x2000  // 元数据扇区2
uint8_t active_sector = 0;         // 当前激活扇区


/* Exported function prototypes ----------------------------------------------*/
void Flash_Erase_Last_Page(void);// 擦除最后一个页 
void Flash_Write(uint32_t address, uint32_t *data,uint32_t size); // 写入n字节数据到指定地址（128位对齐）
void Flash_Read(uint32_t address, uint32_t *buffer,uint32_t size);// 从指定地址读取n字节数据
void	 API_FLASH_INIT(void);

#ifdef __cplusplus
}
#endif


#endif		//__API_FLASH_H__
