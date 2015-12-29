#include "ex02/main.h"

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim) {
	__HAL_RCC_TIM5_CLK_ENABLE();
	HAL_NVIC_SetPriority(TIM5_IRQn, TICK_INT_PRIORITY, 0);
	HAL_NVIC_EnableIRQ(TIM5_IRQn);
}

void HAL_MspInit(void) {
}

void HAL_MspDeInit(void) {
}
