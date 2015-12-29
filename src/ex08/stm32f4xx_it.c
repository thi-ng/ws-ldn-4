#include "ex08/main.h"

extern I2S_HandleTypeDef hAudioOutI2s;
extern I2S_HandleTypeDef hAudioInI2s;
extern HCD_HandleTypeDef usbHCD;

void Error_Handler(void) {
	BSP_LED_On(LED_RED);
	while (1) {
	}
}

void SysTick_Handler(void) {
	HAL_IncTick();
}

// External line 0 interrupt request
void EXTI0_IRQHandler(void) {
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

// External line 1 interrupt request
void EXTI4_IRQHandler(void) {
	HAL_GPIO_EXTI_IRQHandler(ACCELERO_INT1_PIN);
}

// I2S interrupt (audio out DMA)
void I2S3_IRQHandler(void) {
	HAL_DMA_IRQHandler(hAudioOutI2s.hdmatx);
}

// DMA Stream interrupt request (audio in DMA)
void I2S2_IRQHandler(void) {
	HAL_DMA_IRQHandler(hAudioInI2s.hdmarx);
}

// TIM4 global interrupt request
void TIM4_IRQHandler(void) {
	//HAL_TIM_IRQHandler(&ledTimer);
}

// USB-On-The-Go FS global interrupt request.
void OTG_FS_IRQHandler(void) {
	HAL_HCD_IRQHandler(&usbHCD);
}
