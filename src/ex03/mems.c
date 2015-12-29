#include "ex03/mems.h"

extern __IO uint32_t isPressed;

int16_t thresholdAcc = 1000;
float thresholdGyro = 5000.0;

static void readAccel(void);
static void readGyro(void);

void demoAccelerometer(void) {
	if (BSP_ACCELERO_Init() != HAL_OK) {
		Error_Handler();
	}
	isPressed = 0;
	while (!isPressed) {
		readAccel();
	}
}

static void readAccel(void) {
	int16_t buf[3];
	int16_t x, y;

	BSP_ACCELERO_GetXYZ(buf);

	x = buf[0];
	y = buf[1];

	if (ABS(x) > ABS(y)) {
		if (x > thresholdAcc) {
			BSP_LED_On(LED_RED);
		} else if (x < -thresholdAcc) {
			BSP_LED_On(LED_GREEN);
		}
	} else {
		if (y > thresholdAcc) {
			BSP_LED_On(LED_ORANGE);
		} else if (y < -thresholdAcc) {
			BSP_LED_On(LED_BLUE);
		}
	}
	HAL_Delay(10);
	led_all_off();
}

void demoGyro(void) {
	if (BSP_GYRO_Init() != HAL_OK) {
		Error_Handler();
	}
	isPressed = 0;
	while (!isPressed) {
		readGyro();
	}
}

static void readGyro(void) {
	float buf[3];
	float x, y;

	BSP_GYRO_GetXYZ(buf);

	x = ABS(buf[0]);
	y = ABS(buf[1]);

	if (x > y) {
		if (buf[0] > thresholdGyro) {
			BSP_LED_On(LED_RED);
		} else if (buf[0] < -thresholdGyro) {
			BSP_LED_On(LED_GREEN);
		}
	} else {
		if (buf[1] > thresholdGyro) {
			BSP_LED_On(LED_ORANGE);
		} else if (buf[1] < -thresholdGyro) {
			BSP_LED_On(LED_BLUE);
		}
	}
	HAL_Delay(10);
	led_all_off();
}
