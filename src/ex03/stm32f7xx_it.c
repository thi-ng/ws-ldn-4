#include "ex03/main.h"
#include "ex03/stm32f7xx_it.h"
//#include "stm32746g_discovery_lcd.h"

void Error_Handler(void) {
	BSP_LED_On(LED_GREEN);
	while (1) {
	}
}

void SysTick_Handler(void) {
	HAL_IncTick();
}

void EXTI15_10_IRQHandler(void) {
	// Interrupt handler shared between SD_DETECT pin, USER_KEY button and touch screen interrupt
	if (__HAL_GPIO_EXTI_GET_IT(SD_DETECT_PIN) != RESET) {
		HAL_GPIO_EXTI_IRQHandler(
		SD_DETECT_PIN | TS_INT_PIN);
	} else {
		// User button event or Touch screen interrupt
		HAL_GPIO_EXTI_IRQHandler(KEY_BUTTON_PIN);
	}
}

//void DMA2D_IRQHandler(void) {
//	BSP_LCD_DMA2D_IRQHandler();
//}
