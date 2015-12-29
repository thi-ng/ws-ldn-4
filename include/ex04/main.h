#ifndef __EX04_MAIN_H
#define __EX04_MAIN_H

#include "stm32f401_discovery.h"
#include "stm32f401_discovery_audio.h"
#include "stm32f401_discovery_accelerometer.h"
#include "stm32f4xx_it.h"
#include "clockconfig.h"
#include "led.h"
#include "ex04/audio_play.h"
#include "ex04/audio_record.h"

// Size of the recorder buffer (multiple of 4096)
#define RECORD_BUFFER_SIZE 0x7000

typedef void (*DemoFn)(void);

typedef enum {
	PAUSE_STATUS = 0, RESUME_STATUS, IDLE_STATUS
} PlaybackState;

#endif
