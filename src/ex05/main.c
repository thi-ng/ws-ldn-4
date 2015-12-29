#include "ex05/main.h"

TIM_HandleTypeDef ledTimer;
TIM_OC_InitTypeDef ledConfig;

__IO uint32_t isPressed = 0;
__IO uint32_t numPressed = 1;
__IO uint32_t isLooping = 1;

__IO PlaybackState playbackState = PLAYBACK_IDLE;
AppState appState = APP_IDLE;
__IO AppCommand appCommand = CMD_PLAY;

/* Capture Compare Register Value.
 Defined as external in stm32f4xx_it.c file */
__IO uint16_t CCR1Val = 16826;

extern __IO LEDToggleState ledState;

FATFS usbFileSys; /* File system object for USB disk logical drive */
char usbDrivePath[4]; /* USB Host logical drive path */

USBH_HandleTypeDef hUSBHost; /* USB Host handle - DON'T change name*/
USBAppState usbAppState = USBH_USER_FS_INIT;

static void TIM_LED_Config(void);
static void usbUserProcess(USBH_HandleTypeDef *pHost, uint8_t vId);
static void startApp(void);
static void executeAppCommand(void);

int main(void) {
	HAL_Init();

	led_all_init();
	SystemClock_Config();

	if (BSP_ACCELERO_Init() != ACCELERO_OK) {
		Error_Handler();
	}
	BSP_ACCELERO_Click_ITConfig();
	TIM_LED_Config();

	isLooping = 1;
	ledState = LEDS_OFF;

	BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);

	// Link the USB Host disk I/O driver
	if (FATFS_LinkDriver(&USBH_Driver, usbDrivePath) == 0) {
		USBH_Init(&hUSBHost, usbUserProcess, 0);
		USBH_RegisterClass(&hUSBHost, USBH_MSC_CLASS);
		USBH_Start(&hUSBHost);

		while (1) {
			switch (appState) {
			case APP_START:
				startApp();
				break;
			case APP_IDLE:
			default:
				break;
			}
			USBH_Process(&hUSBHost);
		}
	}

	while (1) {
	}
}

/**
 * @brief  User Process
 * @param  phost: Host Handle
 * @param  id: Host Library user message ID
 * @retval None
 */
static void usbUserProcess(USBH_HandleTypeDef *pHost, uint8_t msgID) {
	switch (msgID) {
	case HOST_USER_SELECT_CONFIGURATION:
		break;

	case HOST_USER_DISCONNECTION:
		WavePlayer_CallBack();
		appState = APP_IDLE;
		// unmount file
		f_mount(NULL, (TCHAR const*) "", 0);
		break;

	case HOST_USER_CLASS_ACTIVE:
		appState = APP_START;
		break;

	case HOST_USER_CONNECTION:
		break;

	default:
		break;
	}
}

static void startApp(void) {
	switch (usbAppState) {
	case USBH_USER_AUDIO:
		executeAppCommand();
		usbAppState = USBH_USER_FS_INIT;
		break;

	case USBH_USER_FS_INIT:
		if (f_mount(&usbFileSys, (TCHAR const*) usbDrivePath, 0) != FR_OK) {
			Error_Handler();
		}
		usbAppState = USBH_USER_AUDIO;
		break;

	default:
		break;
	}
}

static void executeAppCommand(void) {
	switch (appCommand) {
	case CMD_PLAY:
		if (isLooping) {
			startPlayback();
		}
		break;

	case CMD_RECORD:
		isLooping = 1;
		recordWaveFile();
		break;

	default:
		break;
	}
}

// Configures TIM4 Peripheral for LEDs lighting.
static void TIM_LED_Config(void) {
	__HAL_RCC_TIM4_CLK_ENABLE();
	HAL_NVIC_SetPriority(TIM4_IRQn, 7, 0);
	HAL_NVIC_EnableIRQ(TIM4_IRQn);

	/* -----------------------------------------------------------------------
	 TIM4 Configuration: Output Compare Timing Mode:
	 To get TIM4 counter clock at 250 KHz, the prescaler is computed as follows:
	 Prescaler = (TIM4CLK / TIM4 counter clock) - 1
	 Prescaler = ((f(APB1) * 2) / 250 KHz) - 1

	 CC update rate = TIM4 counter clock / CCR_Val = 32.687 Hz
	 ==> Toggling frequency = 16.343 Hz
	 ----------------------------------------------------------------------- */

	/* Compute the prescaler value */
	uint16_t prescalervalue = (uint16_t) ((HAL_RCC_GetPCLK1Freq() * 2) / 250000)
			- 1;

	/* Time base configuration */
	ledTimer.Instance = TIM4;
	ledTimer.Init.Period = 65535;
	ledTimer.Init.Prescaler = prescalervalue;
	ledTimer.Init.ClockDivision = 0;
	ledTimer.Init.CounterMode = TIM_COUNTERMODE_UP;
	if (HAL_TIM_OC_Init(&ledTimer) != HAL_OK) {
		Error_Handler();
	}

	/* Output Compare Timing Mode configuration: Channel1 */
	ledConfig.OCMode = TIM_OCMODE_TIMING;
	ledConfig.OCIdleState = TIM_OCIDLESTATE_SET;
	ledConfig.Pulse = CCR1Val;
	ledConfig.OCPolarity = TIM_OCPOLARITY_HIGH;
	ledConfig.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	ledConfig.OCFastMode = TIM_OCFAST_ENABLE;
	ledConfig.OCNIdleState = TIM_OCNIDLESTATE_SET;

	/* Initialize the TIM4 Channel1 with the structure above */
	if (HAL_TIM_OC_ConfigChannel(&ledTimer, &ledConfig, TIM_CHANNEL_1)
			!= HAL_OK) {
		Error_Handler();
	}

	/* Start the Output Compare */
	if (HAL_TIM_OC_Start_IT(&ledTimer, TIM_CHANNEL_1) != HAL_OK) {
		Error_Handler();
	}
}

// Output Compare callback in non blocking mode
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim) {
	if (ledState == LED3_TOGGLE) {
		BSP_LED_Toggle(LED3);
		BSP_LED_Off(LED6);
		BSP_LED_Off(LED4);
	} else if (ledState == LED4_TOGGLE) {
		BSP_LED_Toggle(LED4);
		BSP_LED_Off(LED6);
		BSP_LED_Off(LED3);
	} else if (ledState == LED6_TOGGLE) {
		BSP_LED_Off(LED3);
		BSP_LED_Off(LED4);
		BSP_LED_Toggle(LED6);
	} else if (ledState == STOP_TOGGLE) {
		BSP_LED_On(LED6);
	} else if (ledState == LEDS_OFF) {
		led_all_off();
	}
	/* Get the TIM4 Input Capture 1 value */
	uint32_t capture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);

	/* Set the TIM4 Capture Compare1 Register value */
	__HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, (CCR1Val + capture));
}

// EXTI line detection callback
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == GPIO_PIN_0) {
		if (!isPressed) {
			HAL_Delay(10);
			switch (appCommand) {
			case CMD_RECORD:
				isLooping = 1;
				appCommand = CMD_PLAY;
				break;
			case CMD_PLAY:
				appCommand = CMD_RECORD;
				break;
			default:
				isLooping = 1;
				appCommand = CMD_PLAY;
				break;
			}
			isPressed = 1;
		} else {
			isPressed = 0;
		}
	}

	if (GPIO_Pin == ACCELERO_INT1_PIN) {
		if (numPressed == 1) {
			playbackState = PLAYBACK_RESUME;
			numPressed = 0;
		} else {
			playbackState = PLAYBACK_PAUSE;
			numPressed = 1;
		}
	}
}
