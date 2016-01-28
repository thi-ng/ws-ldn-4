#pragma once

#include "ex04/main.h"

#define VOLUME 70

#define AUDIO_DMA_BUFFER_SIZE 1024
#define AUDIO_DMA_BUFFER_SIZE2 (AUDIO_DMA_BUFFER_SIZE >> 1)
#define AUDIO_DMA_BUFFER_SIZE4 (AUDIO_DMA_BUFFER_SIZE >> 2)
#define AUDIO_DMA_BUFFER_SIZE8 (AUDIO_DMA_BUFFER_SIZE >> 3)

typedef enum {
	BUFFER_OFFSET_NONE = 0, BUFFER_OFFSET_HALF, BUFFER_OFFSET_FULL
} DMABufferState;

typedef struct {
	float osc1Gain;
	float osc2Gain;
	float detune;
	float filterCutoff;
	float filterQ;
	float feedback;
	float width;
	float attack;
	float decay;
	float sustain;
	float release;
	float string;
	uint16_t tempo;
	uint8_t osc1Fn;
	uint8_t osc2Fn;
	uint8_t filterType;
	uint8_t volume;
} SynthPreset;

void demoAudioPlayback(void);
