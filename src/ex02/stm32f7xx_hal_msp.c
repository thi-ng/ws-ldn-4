#include "ex02/main.h"

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim) {
	// TIMx Peripheral clock enable
	TIMx_CLK_ENABLE();
	// Set the TIMx priority
	HAL_NVIC_SetPriority(TIMx_IRQn, 3, 0);
	// Enable the TIMx global Interrupt
	HAL_NVIC_EnableIRQ(TIMx_IRQn);
}
