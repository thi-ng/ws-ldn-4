#ifndef __STM32F4xx_IT_H
#define __STM32F4xx_IT_H

#include "ex02/main.h"

void Error_Handler(void);
void SysTick_Handler(void);
void EXTI0_IRQHandler(void);
void TIM5_IRQHandler(void);

#endif
