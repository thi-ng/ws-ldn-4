#include "ex02/main.h"
#include "ex02/stm32f4xx_it.h"

extern TIM_HandleTypeDef TimHandle;

void Error_Handler(void) {
	BSP_LED_On(LED_RED);
	while (1) {
	}
}

void SysTick_Handler(void) {
	// DON'T DO ANYTHING HERE!
	// usually this handler calls HAL_IncTick()
}

/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/

/**
 * @brief  This function handles External line 0 interrupt request.
 */
void EXTI0_IRQHandler(void) {
	HAL_GPIO_EXTI_IRQHandler(KEY_BUTTON_PIN);
}

/**
 * @brief  This function handles TIM interrupt request.
 */
void TIM5_IRQHandler(void) {
	HAL_TIM_IRQHandler(&TimHandle);
}
