#ifndef __EX06_MAIN_H
#define __EX06_MAIN_H

#include <stdlib.h>
#include <math.h>
#include "stm32f4xx_hal.h"
#include "stm32f401_discovery.h"
#include "stm32f401_discovery_audio.h"
#include "ex06/stm32f4xx_it.h"
#include "clockconfig.h"
#include "led.h"
#include "synth/synth.h"
#include "synth/scales.h"
#include "synth/sequencer.h"

typedef enum {
	BUFFER_OFFSET_NONE = 0, BUFFER_OFFSET_HALF, BUFFER_OFFSET_FULL
} DMABufferState;

#endif
