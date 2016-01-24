#include "ex04/audio_play.h"
#include <string.h>
#include <math.h>

#define VOLUME 70
#define AUDIO_BUFFER_SIZE 256
#define AUDIO_BUFFER_SIZE2 (AUDIO_BUFFER_SIZE / 2)
#define AUDIO_BUFFER_SIZE4 (AUDIO_BUFFER_SIZE / 4)
#define AUDIO_BUFFER_SIZE8 (AUDIO_BUFFER_SIZE / 8)

#define SAMPLE_RATE AUDIO_FREQUENCY_44K
#define TAU_RATE (M_TWOPI / (float)SAMPLE_RATE)
#define INV_TAU_RATE (1.0f / TAU_RATE)
#define HZ_TO_RAD(freq) ((freq)*TAU_RATE)
#define RAD_TO_HZ(freq) ((freq)*INV_TAU_RATE)

typedef enum {
	BUFFER_OFFSET_NONE = 0, BUFFER_OFFSET_HALF, BUFFER_OFFSET_FULL
} DMABufferState;

__IO DMABufferState bufferState = BUFFER_OFFSET_NONE;

typedef struct {
	float freq;
	float phase;
	float amp;
} Osc;

static uint8_t audioBuf[AUDIO_BUFFER_SIZE];

static Osc osc = { .freq = HZ_TO_RAD(110.0f), .phase = 0.0f, .amp = 1.0f };

static void updateAudioBuffer();
static void updateOscillator(int16_t *ptr);

void demoAudioPlayback(void) {
	BSP_LED_On(LED_GREEN);
	memset(audioBuf, 0, AUDIO_BUFFER_SIZE);
	if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_AUTO, VOLUME, SAMPLE_RATE) != 0) {
		Error_Handler();
	}

	BSP_AUDIO_OUT_Play((uint16_t *)audioBuf, AUDIO_BUFFER_SIZE);

	while (1) {
		updateAudioBuffer();
	}

//	if (BSP_AUDIO_OUT_Stop(CODEC_PDWN_HW) != AUDIO_OK) {
//		Error_Handler();
//	}
}

static void updateAudioBuffer() {
	if (bufferState == BUFFER_OFFSET_HALF) {
		int16_t *ptr = (int16_t*) &audioBuf;
		updateOscillator(ptr);
		bufferState = BUFFER_OFFSET_NONE;
	}

	if (bufferState == BUFFER_OFFSET_FULL) {
		int16_t *ptr = (int16_t*) &audioBuf[AUDIO_BUFFER_SIZE >> 1];
		updateOscillator(ptr);
		bufferState = BUFFER_OFFSET_NONE;
	}
}

static void updateOscillator(int16_t *ptr) {
	for (uint16_t i = 0; i < AUDIO_BUFFER_SIZE8; i++) {
		float y = sinf(osc.phase) * osc.amp;
		osc.phase += osc.freq;
		if (osc.phase >= M_TWOPI) {
			osc.phase -= M_TWOPI;
		}
		int16_t yint = (int16_t) (y * 32767.0f);
		*ptr++ = yint;
		*ptr++ = yint;
	}
}

void BSP_AUDIO_OUT_HalfTransfer_CallBack(void) {
	bufferState = BUFFER_OFFSET_HALF;
}

void BSP_AUDIO_OUT_TransferComplete_CallBack(void) {
	bufferState = BUFFER_OFFSET_FULL;
	BSP_AUDIO_OUT_ChangeBuffer((uint16_t*) &audioBuf,
	AUDIO_BUFFER_SIZE >> 1);
}

void BSP_AUDIO_OUT_Error_CallBack(void) {
	Error_Handler();
}
