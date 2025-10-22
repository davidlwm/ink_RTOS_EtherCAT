
#ifndef __GENERAL_TIM_H__
#define __GENERAL_TIM_H__

/* ����ͷ�ļ� ----------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* ���Ͷ��� ------------------------------------------------------------------*/

/* �궨�� --------------------------------------------------------------------*/
/********************ͨ�ö�ʱ��TIM�������壬TIM2************/

#define GENERAL_TIMx                     TIM2
#define GENERAL_TIM_RCC_CLK_ENABLE()     __HAL_RCC_TIM2_CLK_ENABLE()
#define GENERAL_TIM_RCC_CLK_DISABLE()    __HAL_RCC_TIM2_CLK_DISABLE()
#define GENERAL_TIM_IRQ                  TIM2_IRQn
#define GENERAL_TIM_INT_FUN              TIM2_IRQHandler


/* ��չ���� ------------------------------------------------------------------*/
extern TIM_HandleTypeDef htimx;

/* �������� ------------------------------------------------------------------*/
void TIM_Configuration(uint8_t period);	


#endif	/* __GENERAL_TIM_H__ */

