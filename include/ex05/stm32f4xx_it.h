#ifndef __STM32F4xx_IT_H
#define __STM32F4xx_IT_H

#include "main.h"

void Error_Handler(void);
void SysTick_Handler(void);
void EXTI0_IRQHandler(void);
void EXTI4_IRQHandler(void);
void I2S2_IRQHandler(void);
void I2S3_IRQHandler(void);
void OTG_FS_IRQHandler(void);
void TIM4_IRQHandler(void);

#endif
