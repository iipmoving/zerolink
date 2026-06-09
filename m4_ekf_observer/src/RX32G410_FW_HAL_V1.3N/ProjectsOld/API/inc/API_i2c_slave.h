#ifndef __I2C_SALVE_H__
#define __I2C_SALVE_H__

//#include "rx32g4xx_config_def.h"
//#include "rx32g4xx_hal.h"
//#include "system_init.h"
//#include "system_bsp.h"

/* Exported define -----------------------------------------------------------*/
#define I2Cx                            I2C2

#define I2Cx_CLK_ENABLE()               __HAL_RCC_I2C2_CLK_ENABLE()
#define I2Cx_SDA_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define I2Cx_SCL_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE() 

#define I2Cx_FORCE_RESET()              __HAL_RCC_I2C2_FORCE_RESET()
#define I2Cx_RELEASE_RESET()            __HAL_RCC_I2C2_RELEASE_RESET()

/* Definition for I2Cx Pins*/
#define I2Cx_SCL_PIN                    GPIO_PIN_8
#define I2Cx_SCL_GPIO_PORT              GPIOB
#define I2Cx_SDA_PIN                    GPIO_PIN_9
#define I2Cx_SDA_GPIO_PORT              GPIOB
#define	I2Cx_GPIO_AF					GPIO_AF5
/* Definition for I2Cx's NVIC */
#define I2Cx_EV_IRQn                    I2C2_EV_IRQn
#define I2Cx_ER_IRQn                    I2C2_ER_IRQn
#define I2Cx_EV_IRQHandler              I2C2_EV_IRQHandler
#define I2Cx_ER_IRQHandler              I2C2_ER_IRQHandler

#define I2C_ADDRESS      0x50
#define I2C_SPEEDCLOCK   400000
#define I2C_DUTYCYCLE    I2C_DUTYCYCLE_2

#define MASTER_REQ_READ    0x01
#define MASTER_REQ_WRITE   0x10

/* Size of Transmission buffer */
#define TXBUFFERSIZE                    (COUNTOF(SlaveTxBuffer))
/* Size of Reception buffer */
#define RXBUFFERSIZE                    16  //主机发送过来的数据最大为10个，加上命令CM：0x10，最多为11个 、
/* Size of Reception buffer */
#define READSIZE                        3   //主机回读指令。CM和长度，所以选择为2
/* Exported macro ------------------------------------------------------------*/
#define COUNTOF(__BUFFER__)             (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))





void API_I2C1_Init(void);

#endif
