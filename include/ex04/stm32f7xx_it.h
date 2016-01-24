#pragma once
#include "ex04/main.h"

void Error_Handler(void);
void SysTick_Handler(void);

//void EXTI9_5_IRQHandler(void);
//void EXTI2_IRQHandler(void);
void EXTI15_10_IRQHandler(void);
//void DMA2_Stream6_IRQHandler(void);
//void DMA2_Stream5_IRQHandler(void);
//void DMA1_Stream2_IRQHandler(void);
void AUDIO_IN_SAIx_DMAx_IRQHandler(void);
void AUDIO_OUT_SAIx_DMAx_IRQHandler(void);
