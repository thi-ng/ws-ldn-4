#pragma once
#include "stm32f7xx_hal.h"
#include "stm32f746g_discovery.h"
#include "stm32f746g_discovery_audio.h"
#include "stm32f7xx_it.h"
#include "clockconfig.h"
#include "ex04/audio_play.h"
#include "ex04/audio_record.h"

// Size of the recorder buffer (multiple of 4096)
#define RECORD_BUFFER_SIZE 0x7000

typedef void (*DemoFn)(void);

typedef enum {
	PAUSE_STATUS = 0, RESUME_STATUS, IDLE_STATUS
} PlaybackState;
