#include <math.h>
#include "ex03/main.h"
#include "ex03/buttons.h"

#define NUM_DEMOS 3

extern LTDC_HandleTypeDef hLtdcHandler;
extern void LL_ConvertLineToARGB8888(void * pSrc, void *pDst, uint32_t xSize,
		uint32_t ColorMode);

static void demoWelcome();
static void demoGraph();
static void demoScribble();

DemoFn demos[] = { demoWelcome, demoGraph, demoScribble };
uint32_t demoID = 0;

static void touchScreenError();
static TS_StateTypeDef touchState;

__IO uint32_t isPressed = 0;

static void handleButton(Button *bt, TS_StateTypeDef *touchState);
static void renderButton(Button *bt);

static void customDrawBitmap(uint32_t x, uint32_t y, uint32_t width,
		uint32_t height, uint8_t *pbmp);

static Button bt = { .x = 10, .y = 10, .width = 100, .height = 64, .state =
		BS_OFF, .handler = handleButton, .render = renderButton };

int main() {
	CPU_CACHE_Enable();
	HAL_Init();
	SystemClock_Config();

	BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);
	BSP_LCD_Init();
	if (BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize()) == TS_OK) {
		BSP_LCD_LayerDefaultInit(LTDC_ACTIVE_LAYER, LCD_FRAME_BUFFER);

		BSP_LCD_SelectLayer(LTDC_ACTIVE_LAYER);

		while (1) {
			isPressed = 0;
			demos[demoID]();
			demoID = (demoID + 1) % NUM_DEMOS;
			while (!isPressed) {
				BSP_TS_GetState(&touchState);
				if (touchState.touchDetected) {
					do {
						BSP_TS_GetState(&touchState);
						HAL_Delay(10);
					} while (touchState.touchDetected);
					break;
				}
				HAL_Delay(10);
			}
		}
	} else {
		touchScreenError();
	}
	return 0;
}

static void demoWelcome() {
	BSP_LCD_SetFont(&LCD_DEFAULT_FONT);
	BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
	BSP_LCD_Clear(LCD_COLOR_WHITE);
	BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
	BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() / 2 - 24,
			(uint8_t *) "STM32F746G", CENTER_MODE);
	BSP_LCD_SetTextColor(LCD_COLOR_DARKBLUE);
	BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() / 2 + 6,
			(uint8_t *) "Welcome!", CENTER_MODE);
	// Cupertino stylee
	while (!isPressed) {
		BSP_TS_GetState(&touchState);
		bt.handler(&bt, &touchState);
		bt.render(&bt);
		HAL_Delay(30);
	}
}

static void demoGraph() {
	BSP_LCD_Clear(LCD_COLOR_BLACK);
	uint16_t w = BSP_LCD_GetXSize();
	uint16_t h = BSP_LCD_GetYSize() / 2;
	for (uint16_t x = 0; x < w; x++) {
		float theta = (float) x / w * 4.0f * M_PI;
		float y = sinf(theta);
		y += 0.5f * sinf(2.0f * theta);
		y += 0.3333f * sinf(3.0f * theta);
		y += 0.25f * sinf(4.0f * theta);
		BSP_LCD_DrawPixel(x, (uint16_t) (h + h * y * 0.6f), LCD_COLOR_CYAN);
	}
}

static void demoScribble() {
	static uint32_t cols[] = { LCD_COLOR_RED, LCD_COLOR_GREEN, LCD_COLOR_BLUE,
	LCD_COLOR_YELLOW, LCD_COLOR_MAGENTA };
	uint16_t width = BSP_LCD_GetXSize();
	uint16_t height = BSP_LCD_GetYSize();
	BSP_LCD_Clear(LCD_COLOR_WHITE);
	while (!isPressed) {
		BSP_TS_GetState(&touchState);
		if (touchState.touchDetected) {
			for (uint8_t i = 0; i < MIN(touchState.touchDetected, 5); i++) {
				BSP_LCD_SetTextColor(cols[i]);
				BSP_LCD_FillCircle(CLAMP(touchState.touchX[i], 6, width - 6),
						CLAMP(touchState.touchY[i], 6, height - 6), 5);
			}
		}
		HAL_Delay(10);
	}
}

static void handleButton(Button *bt, TS_StateTypeDef *touch) {
	if (touch->touchDetected) {
		// touch detected...
		uint16_t x = touch->touchX[0];
		uint16_t y = touch->touchY[0];
		if (x >= bt->x && x < bt->x + bt->width && y >= bt->y
				&& y < bt->y + bt->height) {
			switch (bt->state) {
			case BS_OFF:
				bt->state = BS_HOVER_TO_ON;
				break;
			case BS_ON:
				bt->state = BS_HOVER_TO_OFF;
				break;
			default:
				break;
			}
		}
	} else {
		switch (bt->state) {
		case BS_HOVER_TO_ON:
			bt->state = BS_ON;
			break;
		case BS_HOVER_TO_OFF:
			bt->state = BS_OFF;
			break;
		default:
			break;
		}
	}
}

static uint32_t buttonColors[] = { 0xff000000, 0xffcccccc, 0xffcc0000,
		0xffcccccc };

static void renderButton(Button *bt) {
	BSP_LCD_SetTextColor(buttonColors[bt->state]);
	BSP_LCD_FillRect(bt->x, bt->y, bt->width, bt->height);
	customDrawBitmap(bt->x, bt->y, 64, 64, i_am_robot);
}

static void touchScreenError() {
	BSP_LCD_SetFont(&LCD_DEFAULT_FONT);
	BSP_LCD_SetBackColor(LCD_COLOR_RED);
	BSP_LCD_Clear(LCD_COLOR_RED);
	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() / 2 - 24,
			(uint8_t *) "Touchscreen error!", CENTER_MODE);
	while (1) {
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (KEY_BUTTON_PIN == GPIO_Pin) {
		while (BSP_PB_GetState(BUTTON_KEY) != RESET) {
		}
		isPressed = 1;
	}
}

static void customDrawBitmap(uint32_t x, uint32_t y, uint32_t width,
		uint32_t height, uint8_t *pbmp) {
	uint32_t lcdWidth = BSP_LCD_GetXSize();
	uint32_t address = hLtdcHandler.LayerCfg[LTDC_ACTIVE_LAYER].FBStartAdress
			+ (((lcdWidth * y) + x) << 2);
	while (--height) {
		LL_ConvertLineToARGB8888((uint32_t *) pbmp, (uint32_t *) address, width,
				CM_ARGB8888);
		address += lcdWidth << 2;
		pbmp += width << 2;
	}
}
