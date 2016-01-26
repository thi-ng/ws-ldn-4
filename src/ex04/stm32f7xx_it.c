#include "ex04/main.h"
#include "ex04/stm32f7xx_it.h"

extern SAI_HandleTypeDef haudio_out_sai;

void Error_Handler(void) {
	BSP_LED_On(LED_GREEN);
	while (1) {
	}
}

void SysTick_Handler(void) {
	HAL_IncTick();
}

/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (BSP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/

void EXTI15_10_IRQHandler(void) {
	if (__HAL_GPIO_EXTI_GET_IT(TS_INT_PIN) != RESET) {
		HAL_GPIO_EXTI_IRQHandler(TS_INT_PIN);
	} else {
		HAL_GPIO_EXTI_IRQHandler(KEY_BUTTON_PIN);
	}
}

void AUDIO_OUT_SAIx_DMAx_IRQHandler(void) {
	HAL_DMA_IRQHandler(haudio_out_sai.hdmatx);
}
