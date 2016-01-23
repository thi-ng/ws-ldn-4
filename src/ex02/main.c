#include "ex02/main.h"

extern void Error_Handler();
static void initTimer(uint16_t period);

TIM_HandleTypeDef TimHandle;
uint8_t isBlinking = 1;

int main(void) {
	CPU_CACHE_Enable();
	HAL_Init();

	SystemClock_Config();

	BSP_LED_Init(LED_GREEN);
	BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);

	// Start timer w/ 2sec interval
	initTimer(20000);

	// Infinite Loop
	while (1) {
	}
}

// Initialize timer w/ 10kHz resolution and given period
static void initTimer(uint16_t period) {
	uint32_t prescaler = (uint32_t) ((SystemCoreClock / 2) / 10000) - 1;
	TimHandle.Instance = TIMx;

	/* Initialize TIMx peripheral as follows:
	 * Period = 10000 - 1
	 * Prescaler = ((SystemCoreClock / 2)/10000) - 1
	 * ClockDivision = 0
	 * Counter direction = Up
	 */
	TimHandle.Init.Period = period - 1;
	TimHandle.Init.Prescaler = prescaler;
	TimHandle.Init.ClockDivision = 0;
	TimHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
	TimHandle.Init.RepetitionCounter = 0;

	if (HAL_TIM_Base_Init(&TimHandle) != HAL_OK) {
		Error_Handler();
	}

	if (HAL_TIM_Base_Start_IT(&TimHandle) != HAL_OK) {
		Error_Handler();
	}
}

// Callback function run whenever timer caused interrupt
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (isBlinking) {
		BSP_LED_Toggle(LED_GREEN);
	}
}

// Callback function run whenever user button has been pressed
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == KEY_BUTTON_PIN) {
		isBlinking = !isBlinking;
	}
}

