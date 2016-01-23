#include "ex02/main.h"
#include "ex02/stm32f7xx_it.h"

extern TIM_HandleTypeDef TimHandle;

void Error_Handler(void) {
	BSP_LED_On(LED_GREEN);
	while (1) {
	}
}

void SysTick_Handler(void) {
	HAL_IncTick();
}

/******************************************************************************/
/*                 STM32F7xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f7xx.S).                                               */
/******************************************************************************/

// External line 0 interrupt request.
void EXTI15_10_IRQHandler(void) {
	HAL_GPIO_EXTI_IRQHandler(KEY_BUTTON_PIN);
}

// Timer interrupt request.
void TIMx_IRQHandler(void) {
	HAL_TIM_IRQHandler(&TimHandle);
}
