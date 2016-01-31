#include "ex05/main.h"
#include "ex05/stm32f7xx_it.h"

extern TIM_HandleTypeDef TimHandle;
extern HCD_HandleTypeDef hhcd;
extern SAI_HandleTypeDef haudio_out_sai;

void Error_Handler(void) {
	BSP_LED_On(LED_GREEN);
	while (1) {
	}
}

void SysTick_Handler(void) {
	HAL_IncTick();
}

void OTG_FS_IRQHandler(void) {
	HAL_HCD_IRQHandler(&hhcd);
}

void AUDIO_OUT_SAIx_DMAx_IRQHandler(void) {
	HAL_DMA_IRQHandler(haudio_out_sai.hdmatx);
}

void TIMx_IRQHandler(void) {
	HAL_TIM_IRQHandler(&TimHandle);
}
