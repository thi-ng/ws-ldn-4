#ifndef __STM32F4xx_IT_H
#define __STM32F4xx_IT_H

#include "ex07/main.h"
#include "stm32f4xx.h"

void Error_Handler(void);
void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);

void EXTI0_IRQHandler(void);
void EXTI4_IRQHandler(void);
void I2S2_IRQHandler(void);
void I2S3_IRQHandler(void);
void TIM4_IRQHandler(void);

#endif
