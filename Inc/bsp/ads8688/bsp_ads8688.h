#ifndef __BSP_ADS8688_H__
#define __BSP_ADS8688_H__

/* 包含头文件 ----------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "ads8688/ADS8688.h"

/* 类型定义 ------------------------------------------------------------------*/
/* 宏定义 --------------------------------------------------------------------*/
/* ADS8688使用SPI3接口 - 根据接线图ADC-spi3.png */
#define ADS8688_SPIx                                   SPI3

#define ADS8688_SPIx_RCC_CLK_ENABLE()                  __HAL_RCC_SPI3_CLK_ENABLE()

#define GPIO_AFx_ADS8688_SPIx                          GPIO_AF6_SPI3

/* SPI3_SCK -> PC10 */
#define ADS8688_SPI_SCK_CLK_ENABLE()                   __HAL_RCC_GPIOC_CLK_ENABLE()
#define ADS8688_SPI_SCK_PORT                           GPIOC
#define ADS8688_SPI_SCK_PIN                            GPIO_PIN_10

/* SPI3_MISO -> PC11 */
#define ADS8688_SPI_MISO_CLK_ENABLE()                  __HAL_RCC_GPIOC_CLK_ENABLE()
#define ADS8688_SPI_MISO_PORT                          GPIOC
#define ADS8688_SPI_MISO_PIN                           GPIO_PIN_11

/* SPI3_MOSI -> PC12 */
#define ADS8688_SPI_MOSI_CLK_ENABLE()                  __HAL_RCC_GPIOC_CLK_ENABLE()
#define ADS8688_SPI_MOSI_PORT                          GPIOC
#define ADS8688_SPI_MOSI_PIN                           GPIO_PIN_12

/* ADS8688_CS -> PA15 - 根据接线图stm32F407ZGT6.png */
#define ADS8688_SPI_CS_CLK_ENABLE()                    __HAL_RCC_GPIOA_CLK_ENABLE()
#define ADS8688_SPI_CS_PORT                            GPIOA
#define ADS8688_SPI_CS_PIN                             GPIO_PIN_15

/* ADS8688 RESET引脚定义 (可选,如果硬件连接了RST引脚) */
#define ADS8688_RST_CLK_ENABLE()                       __HAL_RCC_GPIOB_CLK_ENABLE()
#define ADS8688_RST_PORT                               GPIOB
#define ADS8688_RST_PIN                                GPIO_PIN_8

/* 扩展变量 ------------------------------------------------------------------*/
extern SPI_HandleTypeDef hads8688_spi;
extern ADS8688 ads8688_device;

/* 函数声明 ------------------------------------------------------------------*/
void BSP_ADS8688_Init(void);
HAL_StatusTypeDef BSP_ADS8688_ReadAllChannels(uint16_t *data);
void BSP_ADS8688_ConvertToVoltage(uint16_t *raw_data, float *voltage_data, uint8_t channel_count);

#endif /* __BSP_ADS8688_H__ */
