#include "ex03/main.h"
#include "ex03/stm32f4xx_it.h"

void Error_Handler(void) {
	BSP_LED_On(LED_RED);
	while (1) {
	}
}

void SysTick_Handler(void) {
	HAL_IncTick();
}

/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (BSP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/

void EXTI0_IRQHandler(void) {
	HAL_GPIO_EXTI_IRQHandler(KEY_BUTTON_PIN);
}

void EXTI4_IRQHandler(void) {
	HAL_GPIO_EXTI_IRQHandler(ACCELERO_INT1_PIN);
}
