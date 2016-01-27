#include "ex04/main.h"

DemoFn demos[] = { demoAudioPlayback };

uint32_t demoID = 0;

__IO uint32_t isPressed = 0;
__IO uint32_t numPressed = 0;
__IO PlaybackState playbackState = IDLE_STATUS;
__IO GUITouchState touchState;
TIM_HandleTypeDef TimHandle;

extern TS_DrvTypeDef *tsDriver;
extern uint8_t TS_I2cAddress;

static void initTimer(uint16_t period);

int main(void) {
	CPU_CACHE_Enable();
	HAL_Init();
	HAL_NVIC_SetPriority(SysTick_IRQn, 0 ,0);
	HAL_MPU_Disable();
	BSP_LED_Init(LED_GREEN);
	SystemClock_Config();
	BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);

	BSP_LCD_Init();
	BSP_LCD_LayerDefaultInit(LTDC_ACTIVE_LAYER, LCD_FRAME_BUFFER);
	BSP_LCD_SelectLayer(LTDC_ACTIVE_LAYER);
	BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
	BSP_TS_ITConfig();

	initTimer(20);

	touchState.touchDetected = 0;
	touchState.lastTouch = 0;
	demos[demoID]();
}

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

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	switch (GPIO_Pin) {
	case KEY_BUTTON_PIN:
		while (BSP_PB_GetState(BUTTON_KEY) != RESET) {
		}
		isPressed = 1;
		break;
	case TS_INT_PIN:
		BSP_LED_Toggle(LED_GREEN);
		uint32_t tick = HAL_GetTick();
		if (tick - touchState.lastTouch > 1) {
			touchState.touchUpdate = 1;
			touchState.lastTouch = tick;
		}
		BSP_TS_ITClear();
		break;
	default:
		break;
	}
}

void getTouchState(volatile GUITouchState *state) {
	state->touchDetected = tsDriver->DetectTouch(TS_I2cAddress);
	if (state->touchDetected) {
		tsDriver->GetXY(TS_I2cAddress, &(state->touchY[0]),
				&(state->touchX[0]));
	}
}
