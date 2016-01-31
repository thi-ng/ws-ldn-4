#pragma once

#include "ex05/main.h"

void Error_Handler(void);
void SysTick_Handler(void);

void EXTI15_10_IRQHandler(void);
void AUDIO_OUT_SAIx_DMAx_IRQHandler(void);
void TIMx_IRQHandler(void);
