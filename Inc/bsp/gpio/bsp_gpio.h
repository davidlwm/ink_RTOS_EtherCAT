#ifndef __BSP_GPIO_H__
#define __BSP_GPIO_H__

/* ����ͷ�ļ� ----------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* ���Ͷ��� --------------------------------------------------------------*/

/* �궨�� --------------------------------------------------------------------*/
#define RST_ESC     HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, GPIO_PIN_RESET);
#define RST_ESCEND  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, GPIO_PIN_SET);


/* ��չ���� ------------------------------------------------------------------*/
/* �������� ------------------------------------------------------------------*/
void EXTI0_Configuration(void);
void EXTI1_Configuration(void);
void EXTI3_Configuration(void);
void RST_Configuration(void);

#endif  // __BSP_GPIO_H__


