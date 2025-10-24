/* 包含头文件 ----------------------------------------------------------------*/
#include "ads8688/bsp_ads8688.h"
#include "usart/bsp_debug_usart.h"

/* 私有类型定义 --------------------------------------------------------------*/
/* 私有宏定义 ----------------------------------------------------------------*/
/* 私有变量 ------------------------------------------------------------------*/
SPI_HandleTypeDef hads8688_spi;
ADS8688 ads8688_device;

/* 扩展变量 ------------------------------------------------------------------*/
/* 私有函数原型 --------------------------------------------------------------*/
static void ADS8688_SPI_GPIO_Config(void);
static void ADS8688_SPI_Config(void);

/* 函数体 --------------------------------------------------------------------*/

/**
  * 函数功能: ADS8688 SPI引脚配置
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 配置SPI3引脚和CS引脚
*/
static void ADS8688_SPI_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* 使能GPIO时钟 */
    ADS8688_SPI_SCK_CLK_ENABLE();
    ADS8688_SPI_MISO_CLK_ENABLE();
    ADS8688_SPI_MOSI_CLK_ENABLE();
    ADS8688_SPI_CS_CLK_ENABLE();
    ADS8688_RST_CLK_ENABLE();

    /* 配置SPI引脚 */
    GPIO_InitStruct.Pin = ADS8688_SPI_SCK_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AFx_ADS8688_SPIx;
    HAL_GPIO_Init(ADS8688_SPI_SCK_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = ADS8688_SPI_MISO_PIN;
    HAL_GPIO_Init(ADS8688_SPI_MISO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = ADS8688_SPI_MOSI_PIN;
    HAL_GPIO_Init(ADS8688_SPI_MOSI_PORT, &GPIO_InitStruct);

    /* 配置CS引脚为推挽输出 */
    GPIO_InitStruct.Pin = ADS8688_SPI_CS_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(ADS8688_SPI_CS_PORT, &GPIO_InitStruct);

    /* 配置RST引脚为推挽输出 */
    GPIO_InitStruct.Pin = ADS8688_RST_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(ADS8688_RST_PORT, &GPIO_InitStruct);

    /* 设置CS和RST为高电平 */
    HAL_GPIO_WritePin(ADS8688_SPI_CS_PORT, ADS8688_SPI_CS_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(ADS8688_RST_PORT, ADS8688_RST_PIN, GPIO_PIN_SET);
}

/**
  * 函数功能: ADS8688 SPI配置
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 配置SPI3参数
*/
static void ADS8688_SPI_Config(void)
{
    /* 使能SPI时钟 */
    ADS8688_SPIx_RCC_CLK_ENABLE();

    hads8688_spi.Instance = ADS8688_SPIx;
    hads8688_spi.Init.Mode = SPI_MODE_MASTER;                    // 主模式
    hads8688_spi.Init.Direction = SPI_DIRECTION_2LINES;          // 双线双向全双工
    hads8688_spi.Init.DataSize = SPI_DATASIZE_8BIT;             // 8位数据长度
    hads8688_spi.Init.CLKPolarity = SPI_POLARITY_LOW;           // 时钟极性:空闲状态时CLK保持低电平
    hads8688_spi.Init.CLKPhase = SPI_PHASE_2EDGE;               // 时钟相位:数据在SCK的第二个跳变沿被采样
    hads8688_spi.Init.NSS = SPI_NSS_SOFT;                       // NSS软件管理
    hads8688_spi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32; // SPI波特率预分频值32
    hads8688_spi.Init.FirstBit = SPI_FIRSTBIT_MSB;              // MSB先行
    hads8688_spi.Init.TIMode = SPI_TIMODE_DISABLE;              // 关闭TI模式
    hads8688_spi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE; // 关闭硬件CRC校验
    hads8688_spi.Init.CRCPolynomial = 10;                       // CRC值计算的多项式

    HAL_SPI_Init(&hads8688_spi);
}

/**
  * 函数功能: 初始化ADS8688
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 该函数应由HAL库内部调用
*/
void BSP_ADS8688_Init(void)
{
    /* 配置SPI引脚 */
    ADS8688_SPI_GPIO_Config();

    /* 配置SPI */
    ADS8688_SPI_Config();

    /* 硬件复位ADS8688 */
    HAL_GPIO_WritePin(ADS8688_RST_PORT, ADS8688_RST_PIN, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(ADS8688_RST_PORT, ADS8688_RST_PIN, GPIO_PIN_SET);
    HAL_Delay(100);

    /* 初始化ADS8688设备 */
    uint8_t init_status = ADS8688_Init(&ads8688_device,
                                       &hads8688_spi,
                                       ADS8688_SPI_CS_PORT,
                                       ADS8688_SPI_CS_PIN);

    if (init_status == 0) {
        printf("ADS8688 initialization successful\r\n");
    } else {
        printf("ADS8688 initialization failed with status: %d\r\n", init_status);
    }
}

/**
  * 函数功能: 读取ADS8688所有通道数据
  * 输入参数: data - 存储读取数据的数组指针
  * 返 回 值: HAL状态
  * 说    明: 读取8个通道的16位ADC值
*/
HAL_StatusTypeDef BSP_ADS8688_ReadAllChannels(uint16_t *data)
{
    return ADS_Read_All_Raw(&ads8688_device, data);
}

/**
  * 函数功能: 将ADC原始数据转换为电压值
  * 输入参数: raw_data - 原始ADC数据
  *          voltage_data - 转换后的电压值数组
  *          channel_count - 通道数量
  * 返 回 值: 无
  * 说    明: 根据配置的量程转换为实际电压值
*/
void BSP_ADS8688_ConvertToVoltage(uint16_t *raw_data, float *voltage_data, uint8_t channel_count)
{
    for (uint8_t i = 0; i < channel_count && i < 8; i++) {
        /* 根据ADS8688配置的量程计算电压值 */
        /* Channel 0,1,6,7: 0-5V range (0x06) */
        /* Channel 2,3,4,5: 0-10V range (0x05) */
        if (i == 0 || i == 1 || i == 6 || i == 7) {
            // 0-5V range: raw_data / 65536 * 5.0V
            voltage_data[i] = ((float)raw_data[i] / 65536.0f) * 5.0f;
        } else {
            // 0-10V range: raw_data / 65536 * 10.0V
            voltage_data[i] = ((float)raw_data[i] / 65536.0f) * 10.0f;
        }
    }
}