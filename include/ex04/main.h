#pragma once

#include "stm32f7xx_hal.h"
#include "stm32746g_discovery.h"
#include "stm32746g_discovery_audio.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_ts.h"
#include "ex04/stm32f7xx_it.h"
#include "ex04/audio_play.h"
#include "gui/gui.h"
#include "clockconfig.h"

#define LCD_FRAME_BUFFER SDRAM_DEVICE_ADDR

#define TIMx              TIM3
#define TIMx_CLK_ENABLE() __HAL_RCC_TIM3_CLK_ENABLE()
#define TIMx_IRQn         TIM3_IRQn
#define TIMx_IRQHandler   TIM3_IRQHandler

typedef void (*DemoFn)(void);

typedef enum {
	PAUSE_STATUS = 0, RESUME_STATUS, IDLE_STATUS
} PlaybackState;

void getTouchState(volatile GUITouchState *state);
