#include "ex04/audio_play.h"

typedef struct {
	uint32_t riffTag;
	uint32_t riffLength;
	uint32_t waveTag;
	uint32_t formatTag;
	uint32_t formatLength;
	uint16_t audioFormat;
	uint16_t numChannels;
	uint32_t sampleRate;
	uint32_t byteRate;
	uint16_t blockAlign;
	uint16_t bits;
	uint32_t dataTag;
	uint32_t dataLength;
} WavHeader;

#define AUDIO_FILE_SIZE    0x20000
#define AUDIO_FILE_ADDRESS 0x08020000
#define VOLUME 70

extern __IO uint32_t isPressed;
extern __IO PlaybackState playbackState;
__IO uint32_t useRecordBuffer = 0;

WavHeader *wav = NULL;

uint32_t audioTotalSize = 0xffff; // total size of the audio file
uint32_t audioRemSize = 0xffff; // remaining data in audio file
uint16_t *currentPos; // current position of audio pointer

extern uint16_t recordBuffer[RECORD_BUFFER_SIZE];

void demoAudioPlayback(void) {

	if (BSP_ACCELERO_Init() != ACCELERO_OK) {
		Error_Handler();
	}

	BSP_ACCELERO_Click_ITConfig();
	BSP_LED_On(LED_BLUE);
	BSP_LED_On(LED_GREEN);
	wav = (WavHeader*) AUDIO_FILE_ADDRESS;

	if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_AUTO, VOLUME, wav->sampleRate) != 0) {
		Error_Handler();
	}

	isPressed = 0;
	useRecordBuffer = 0;

	audioTotalSize = AUDIO_FILE_SIZE;
	currentPos = (uint16_t*) AUDIO_FILE_ADDRESS;
	BSP_AUDIO_OUT_Play(currentPos, audioTotalSize);
	audioRemSize = audioTotalSize - AUDIODATA_SIZE * DMA_MAX(audioTotalSize);
	currentPos += DMA_MAX(audioTotalSize);

	while (!isPressed) {
		if (playbackState == PAUSE_STATUS) {
			BSP_LED_Off(LED_GREEN);
			BSP_LED_On(LED_RED);
			BSP_AUDIO_OUT_Pause();
			playbackState = IDLE_STATUS;
		} else if (playbackState == RESUME_STATUS) {
			BSP_LED_Off(LED_RED);
			BSP_LED_On(LED_GREEN);
			BSP_AUDIO_OUT_Resume();
			playbackState = IDLE_STATUS;
		}
	}

	if (BSP_AUDIO_OUT_Stop(CODEC_PDWN_HW) != AUDIO_OK) {
		Error_Handler();
	}
}

// DMA callback: Calculates remaining file size & new position of the pointer
void BSP_AUDIO_OUT_TransferComplete_CallBack() {
	uint32_t isLooping = 0;

	if (audioRemSize > 0) {
		BSP_AUDIO_OUT_ChangeBuffer(currentPos,
				DMA_MAX(audioRemSize/AUDIODATA_SIZE));
		currentPos += DMA_MAX(audioRemSize);
		audioRemSize -= AUDIODATA_SIZE * DMA_MAX(audioRemSize / AUDIODATA_SIZE);
	} else {
		isLooping = 1;
	}

	if (isLooping) {
		if (useRecordBuffer) {
			BSP_AUDIO_OUT_Play(recordBuffer, audioTotalSize);
		} else {
			currentPos = (uint16_t*) AUDIO_FILE_ADDRESS;
			BSP_AUDIO_OUT_Play(currentPos, audioTotalSize);
			audioRemSize = audioTotalSize
					- AUDIODATA_SIZE * DMA_MAX(audioTotalSize);
			currentPos += DMA_MAX(audioTotalSize);
		}
	}
}

void BSP_AUDIO_OUT_Error_CallBack(void) {
	Error_Handler();
}
