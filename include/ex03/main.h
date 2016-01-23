#pragma once
#include "stm32746g_discovery.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_ts.h"
#include "ex03/stm32f7xx_it.h"
#include "clockconfig.h"
#include <stdint.h>

#define RGB565_BYTE_PER_PIXEL     2
#define ARBG8888_BYTE_PER_PIXEL   4
#define LCD_FRAME_BUFFER          SDRAM_DEVICE_ADDR

#define MIN(x,y) ((x) < (y) ? (x) : (y))
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#define CLAMP(x,lo,hi) (MAX(MIN((x),(hi)),(lo)))

typedef void (*DemoFn)(void);

typedef struct Button Button;

typedef void (*ButtonHandler)(Button *button, TS_StateTypeDef *touchState);
typedef void (*ButtonRenderer)(Button *button);

enum {
	BS_OFF = 1,
	BS_ON = 2,
	BS_HOVER = 4,
	BS_DIRTY = 8,
	BS_ONOFF_MASK = BS_ON | BS_OFF
};

struct Button {
	uint16_t x;
	uint16_t y;
	uint16_t width;
	uint16_t height;
	uint16_t state;
	ButtonHandler handler;
	ButtonRenderer render;
};
