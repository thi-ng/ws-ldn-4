#include "ex04/main.h"

DemoFn demos[] = { demoAudioPlayback };

uint32_t demoID = 0;

__IO uint32_t isPressed = 0;
__IO uint32_t numPressed = 0;
__IO PlaybackState playbackState = IDLE_STATUS;

int main(void) {
	HAL_Init();

	BSP_LED_Init(LED_GREEN);
	SystemClock_Config();
	BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);

	while (1) {
		isPressed = 0;
		demos[demoID]();
		BSP_LED_On(LED_GREEN);
		while (!isPressed);
		BSP_LED_Off(LED_GREEN);
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (KEY_BUTTON_PIN == GPIO_Pin) {
		while (BSP_PB_GetState(BUTTON_KEY) != RESET)
			;
		isPressed = 1;
	}
}
