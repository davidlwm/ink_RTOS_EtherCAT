/**
  ******************************************************************************
  * @file    ads8688_example.h
  * @brief   ADS8688驱动使用示例头文件
  ******************************************************************************
  */

#ifndef __ADS8688_EXAMPLE_H__
#define __ADS8688_EXAMPLE_H__

#include "stm32f4xx_hal.h"

/* 函数声明 ------------------------------------------------------------------*/
void ADS8688_Example_Init(void);
void ADS8688_Example_ReadChannels(void);
void ADS8688_Task_Example(void *pvParameters);
void ADS8688_CreateTask(void);

#endif /* __ADS8688_EXAMPLE_H__ */