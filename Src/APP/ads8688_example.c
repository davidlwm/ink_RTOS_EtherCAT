/**
  ******************************************************************************
  * @file    ads8688_example.c
  * @brief   ADS8688驱动使用示例
  * @note    这是一个ADS8688 ADC驱动的使用示例，展示如何初始化和读取数据
  ******************************************************************************
  */

#include "ads8688/bsp_ads8688.h"
#include "usart/bsp_debug_usart.h"
#include "FreeRTOS.h"
#include "task.h"

/**
  * @brief  ADS8688使用示例 - 初始化
  * @note   在main函数或初始化任务中调用
  */
void ADS8688_Example_Init(void)
{
    printf("ADS8688 Initialization...\r\n");

    /* 初始化ADS8688驱动 */
    BSP_ADS8688_Init();

    printf("ADS8688 Initialized successfully\r\n");
}

/**
  * @brief  ADS8688使用示例 - 读取所有通道数据
  * @note   可以在任务中周期性调用此函数
  */
void ADS8688_Example_ReadChannels(void)
{
    uint16_t raw_data[8] = {0};    // 存储原始ADC数据
    float voltage_data[8] = {0.0}; // 存储转换后的电压值
    HAL_StatusTypeDef status;

    /* 读取所有8个通道的ADC数据 */
    status = BSP_ADS8688_ReadAllChannels(raw_data);

    if (status == HAL_OK) {
        /* 转换为电压值 */
        BSP_ADS8688_ConvertToVoltage(raw_data, voltage_data, 8);

        /* 打印所有通道的数据 */
        printf("ADS8688 Channel Data:\r\n");
        for (uint8_t i = 0; i < 8; i++) {
            printf("CH%d: Raw=0x%04X (%.3fV)\r\n",
                   i, raw_data[i], voltage_data[i]);
        }
        printf("\r\n");
    } else {
        printf("Error reading ADS8688 channels\r\n");
    }
}

/**
  * @brief  ADS8688采集任务示例
  * @note   这是一个FreeRTOS任务示例，周期性读取ADC数据
  */
void ADS8688_Task_Example(void *pvParameters)
{
    uint16_t raw_data[8] = {0};
    float voltage_data[8] = {0.0};

    printf("ADS8688 Task Started\r\n");

    while (1) {
        /* 读取ADC数据 */
        if (BSP_ADS8688_ReadAllChannels(raw_data) == HAL_OK) {
            /* 转换为电压值 */
            BSP_ADS8688_ConvertToVoltage(raw_data, voltage_data, 8);

            /* 处理数据 - 这里只是打印 */
            for (uint8_t i = 0; i < 8; i++) {
                // 可以在这里添加数据处理逻辑
                // 例如：发送到EtherCAT，存储到缓冲区等
            }
        }

        /* 延时100ms后再次采样 */
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/**
  * @brief  创建ADS8688采集任务
  * @note   在main函数中调用此函数创建任务
  */
void ADS8688_CreateTask(void)
{
    BaseType_t xReturn = pdPASS;

    xReturn = xTaskCreate(ADS8688_Task_Example,    /* 任务入口函数 */
                         "ADS8688_Task",           /* 任务名称 */
                         512,                      /* 任务栈大小 */
                         NULL,                     /* 任务参数 */
                         5,                        /* 任务优先级 */
                         NULL);                    /* 任务句柄 */

    if (xReturn != pdPASS) {
        printf("Failed to create ADS8688 task\r\n");
    }
}

/* 使用方法:
 *
 * 1. 在main函数中调用初始化:
 *    ADS8688_Example_Init();
 *
 * 2. 简单读取示例:
 *    ADS8688_Example_ReadChannels();
 *
 * 3. 创建采集任务:
 *    ADS8688_CreateTask();
 *
 * 4. 或者在现有任务中周期性读取:
 *    uint16_t raw_data[8];
 *    BSP_ADS8688_ReadAllChannels(raw_data);
 */
