#include "ex03/main.h"

DemoFn demos[] = { demoAccelerometer, demoGyro };

uint32_t demoID = 0;
__IO uint32_t isPressed = 0;

int main(void) {
	HAL_Init();
	led_all_init();
	SystemClock_Config();
	BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);

	while (1) {
		isPressed = 0;
		demos[demoID]();
		demoID = (demoID + 1) % 2;
		isPressed = 0;
		led_all_on();
		while (!isPressed);
		led_all_off();
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (KEY_BUTTON_PIN == GPIO_Pin) {
		while (BSP_PB_GetState(BUTTON_KEY) != RESET);
		isPressed = 1;
	}
}
