#include "API_i2c_slave.h"

I2C_HandleTypeDef hi2c1;

void MX_I2C1_Init(void)
{  
  hi2c1.Instance             = I2Cx;
  hi2c1.Init.ClockSpeed      = I2C_SPEEDCLOCK;
  hi2c1.Init.DutyCycle       = I2C_DUTYCYCLE;     
  hi2c1.Init.OwnAddress1     = I2C_ADDRESS;
  hi2c1.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;      
}

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
    GPIO_InitTypeDef  GPIO_InitStruct;
    
    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* Enable GPIO TX/RX clock */
    I2Cx_SCL_GPIO_CLK_ENABLE();
    I2Cx_SDA_GPIO_CLK_ENABLE();
    /* Enable I2Cx clock */
    I2Cx_CLK_ENABLE();
        I2Cx_FORCE_RESET();
        I2Cx_RELEASE_RESET();
    /*##-2- Configure peripheral GPIO ##########################################*/
    /* I2C TX GPIO pin configuration  */
    GPIO_InitStruct.Pin       = I2Cx_SCL_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = I2Cx_GPIO_AF;
    HAL_GPIO_Init(I2Cx_SCL_GPIO_PORT, &GPIO_InitStruct);
    
    /* I2C RX GPIO pin configuration  */
    GPIO_InitStruct.Pin       = I2Cx_SDA_PIN;
    HAL_GPIO_Init(I2Cx_SDA_GPIO_PORT, &GPIO_InitStruct);
    
    /*##-3- Configure the NVIC for I2C ########################################*/
    /* NVIC for I2Cx */
    HAL_NVIC_SetPriority(I2Cx_ER_IRQn, 0, 1);
    HAL_NVIC_EnableIRQ(I2Cx_ER_IRQn);
    HAL_NVIC_SetPriority(I2Cx_EV_IRQn, 0, 2);
    HAL_NVIC_EnableIRQ(I2Cx_EV_IRQn);
}

