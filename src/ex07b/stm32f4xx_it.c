#include "ex07/main.h"

extern I2S_HandleTypeDef hAudioOutI2s;
extern I2S_HandleTypeDef hAudioInI2s;

void Error_Handler(void) {
	BSP_LED_On(LED_RED);
	while (1) {
	}
}

void NMI_Handler(void) {
}

void HardFault_Handler(void) {
	while (1) {
	}
}

void MemManage_Handler(void) {
	while (1) {
	}
}

void BusFault_Handler(void) {
	while (1) {
	}
}

void UsageFault_Handler(void) {
	while (1) {
	}
}

void SVC_Handler(void) {
}

void DebugMon_Handler(void) {
}

void PendSV_Handler(void) {
}

void SysTick_Handler(void) {
	HAL_IncTick();
}

/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f40xx.s).                                              */
/******************************************************************************/

void EXTI0_IRQHandler(void) {
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

void EXTI4_IRQHandler(void) {
	HAL_GPIO_EXTI_IRQHandler(ACCELERO_INT1_PIN);
}

void I2S3_IRQHandler(void) {
	HAL_DMA_IRQHandler(hAudioOutI2s.hdmatx);
}

void I2S2_IRQHandler(void) {
	HAL_DMA_IRQHandler(hAudioInI2s.hdmarx);
}

void TIM4_IRQHandler(void) {
	//HAL_TIM_IRQHandler(&hTimLed);
}
