#include "ex04/audio_play.h"
#include <string.h>
#include <math.h>

#define VOLUME 70
#define AUDIO_BUFFER_SIZE 512
#define AUDIO_BUFFER_SIZE2 (AUDIO_BUFFER_SIZE / 2)
#define AUDIO_BUFFER_SIZE4 (AUDIO_BUFFER_SIZE / 4)

#define SAMPLE_RATE AUDIO_FREQUENCY_44K
#define TAU_RATE (M_TWOPI / (float)SAMPLE_RATE)
#define INV_TAU_RATE (1.0f / TAU_RATE)
#define HZ_TO_RAD(freq) ((freq)*TAU_RATE)
#define RAD_TO_HZ(freq) ((freq)*INV_TAU_RATE)

extern __IO PlaybackState playbackState;

typedef struct {
	float freq;
	float phase;
	float amp;
} Osc;

uint16_t audioBuf[AUDIO_BUFFER_SIZE];
uint16_t *currentPos;

Osc osc = { .freq = HZ_TO_RAD(440.0f), .phase = 0.0f, .amp = 1.0f };

void demoAudioPlayback(void) {
	BSP_LED_On(LED_GREEN);
	memset(audioBuf, 0, AUDIO_BUFFER_SIZE * 2);
	if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_AUTO, VOLUME, SAMPLE_RATE) != 0) {
		Error_Handler();
	}

	isPressed = 0;
	currentPos = audioBuf;
	BSP_AUDIO_OUT_Play(currentPos, AUDIO_BUFFER_SIZE2);

	while (1) {
	}

//	if (BSP_AUDIO_OUT_Stop(CODEC_PDWN_HW) != AUDIO_OK) {
//		Error_Handler();
//	}
}

void updateOscillator() {
	for (uint16_t i = 0; i < AUDIO_BUFFER_SIZE4; i++) {
		float y = sinf(osc->phase) * osc->amp;
		osc->phase += osc->freq;
		if (osc->phase >= M_TWOPI) {
			osc->phase -= M_TWOPI;
		}
		int16_t yint = (int16_t) (y * 32767.0f);
		*currentPos++ = yint;
		*currentPos++ = yint;
	}
}

void BSP_AUDIO_OUT_HalfTransfer_CallBack(void) {
	updateOscillator();
}

// DMA callback: Calculates remaining file size & new position of the pointer
void BSP_AUDIO_OUT_TransferComplete_CallBack() {
	BSP_AUDIO_OUT_ChangeBuffer(currentPos, AUDIO_BUFFER_SIZE);
	updateOscillator();
	currentPos = audioBuf;
}

void BSP_AUDIO_OUT_Error_CallBack(void) {
	Error_Handler();
}
