#include "ex01/main.h"

uint8_t isPaused = 0;

int main(void) {
	HAL_Init();

	BSP_LED_Init(LED_GREEN);
	SystemClock_Config();
	// option #1
	//BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_GPIO);
	// option #2
	BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);

	while (1) {
		// option #1: synchronous button read via GPIO pin polling
//		if (BSP_PB_GetState(BUTTON_KEY) == SET) {
//			while (BSP_PB_GetState(BUTTON_KEY) != RESET);
//			isPaused ^= 0x1;
//		}
		if (!isPaused) {
			BSP_LED_On(LED_GREEN);
			HAL_Delay(LED_SPEED);
			BSP_LED_Off(LED_GREEN);
			HAL_Delay(LED_SPEED);
		}
	}
}

// option #2: async button handling via EXTI interrupt

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == KEY_BUTTON_PIN) {
		while (BSP_PB_GetState(BUTTON_KEY) != RESET);
		isPaused ^= 0x1;
	}
}
