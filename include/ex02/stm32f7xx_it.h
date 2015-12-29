#ifndef __STM32F7xx_IT_H
#define __STM32F7xx_IT_H

#include "ex02/main.h"

void Error_Handler(void);
void SysTick_Handler(void);
void EXTI15_10_IRQHandler(void);
void TIMx_IRQHandler(void);

#endif
