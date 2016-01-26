#pragma once
#include "stm32f7xx_hal.h"
#include "stm32746g_discovery.h"
#include "stm32746g_discovery_audio.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_ts.h"
#include "stm32f7xx_it.h"
#include "clockconfig.h"
#include "ex04/audio_play.h"
#include "gui/gui.h"

#define LCD_FRAME_BUFFER SDRAM_DEVICE_ADDR

typedef void (*DemoFn)(void);

typedef enum {
	PAUSE_STATUS = 0, RESUME_STATUS, IDLE_STATUS
} PlaybackState;

void getTouchState(volatile GUITouchState *state);
