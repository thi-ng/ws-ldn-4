#include "ex02/main.h"

extern void Error_Handler(void);

static void CPU_CACHE_Enable(void);

TIM_HandleTypeDef TimHandle;
uint32_t isSuspended = 0;

int main(void) {
	CPU_CACHE_Enable();
	/* This sample code shows how to configure The HAL time base source base with a
	 dedicated  Tick interrupt priority.
	 A general purpose timer(TIM5) is used instead of Systick as source of time base.
	 Time base duration is fixed to 1ms since PPP_TIMEOUT_VALUEs are defined and
	 handled in milliseconds basis.
	 */
	HAL_Init();

	SystemClock_Config();
	BSP_LED_Init(LED_GREEN);
	BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);

	while (1) {
		BSP_LED_Toggle(LED_GREEN);
		HAL_Delay(1000);
	}
}

/**
 * @brief  This function configures the TIM5 as a time base source.
 *         The time source is configured to have 1ms time base with a dedicated
 *         Tick interrupt priority.
 * @note   This function is called  automatically at the beginning of program after
 *         reset by HAL_Init() or at any time when clock is configured, by HAL_RCC_ClockConfig().
 * @param  TickPriority: Tick interrupt priority.
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority) {
	RCC_ClkInitTypeDef sClokConfig;
	uint32_t uwTimclock;
	uint32_t uwPrescalerValue;
	uint32_t pFLatency;

	/* Configure the timer IRQ priority */
	HAL_NVIC_SetPriority(TIMx_IRQn, TickPriority, 0);

	/* Get clock configuration */
	HAL_RCC_GetClockConfig(&sClokConfig, &pFLatency);

	/* Compute timer clock */
	uwTimclock = HAL_RCC_GetPCLK1Freq();
	if (sClokConfig.APB1CLKDivider != 0) {
		uwTimclock = 2 * HAL_RCC_GetPCLK1Freq();
	}

	/* Compute the prescaler value to have TIMx counter clock equal to 1MHz */
	uwPrescalerValue = (uint32_t) ((uwTimclock / 1000000) - 1);

	/* Initialize TIMx peripheral as follow:
	 + Period = [(TIMxCLK/1000) - 1]. to have a (1/1000) s time base.
	 + Prescaler = (uwTimclock/1000000 - 1) to have a 1MHz counter clock.
	 + ClockDivision = 0
	 + Counter direction = Up
	 */
	TimHandle.Instance = TIMx;
	TimHandle.Init.Period = (1000000 / 1000) - 1;
	TimHandle.Init.Prescaler = uwPrescalerValue;
	TimHandle.Init.ClockDivision = 0;
	TimHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
	if (HAL_TIM_Base_Init(&TimHandle) != HAL_OK) {
		/* Initialization Error */
		Error_Handler();
	}

	/* Start the TIM time Base generation in interrupt mode */
	if (HAL_TIM_Base_Start_IT(&TimHandle) != HAL_OK) {
		/* Starting Error */
		Error_Handler();
	}

	return HAL_OK;
}

// Disables the tick increment by disabling TIM5 update interrupt.
void HAL_SuspendTick(void) {
	__HAL_TIM_DISABLE_IT(&TimHandle, TIM_IT_UPDATE);
}

// Enables the tick increment by Enabling TIM5 update interrupt.
void HAL_ResumeTick(void) {
	__HAL_TIM_ENABLE_IT(&TimHandle, TIM_IT_UPDATE);
}

/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM5 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	HAL_IncTick();
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == KEY_BUTTON_PIN) {
		if (isSuspended == 0) {
			HAL_SuspendTick();
			isSuspended = 1;
		} else {
			HAL_ResumeTick();
			isSuspended = 0;
		}
	}
}

static void CPU_CACHE_Enable(void) {
	SCB_EnableICache();
	SCB_EnableDCache();
}
