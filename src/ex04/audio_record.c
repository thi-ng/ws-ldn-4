#include "ex04/audio_record.h"
#include <string.h>

#define PCM_SIZE2 (2 * PCM_OUT_SIZE)
#define PCM_SIZE4 (4 * PCM_OUT_SIZE)

typedef enum {
	BUFFER_OFFSET_NONE = 0, BUFFER_OFFSET_HALF, BUFFER_OFFSET_FULL
} DMABufferState;

typedef enum {
	RECORD_WAIT = 0, RECORD_BUFFER_FLIP, RECORD_DONE
} RecordState;

typedef struct {
	DMABufferState state;
	uint32_t ptr;
} AudioBufferState;

uint16_t recordBuffer[RECORD_BUFFER_SIZE];

static uint16_t bufPCM[PCM_SIZE2];
static uint16_t bufPDM[INTERNAL_BUFF_SIZE];

AudioBufferState bufferState;

__IO uint32_t numRecorded = 0;
__IO uint32_t audioBuffOffset = 0;
__IO uint32_t audioRecordState = RECORD_WAIT;

extern __IO uint32_t isPressed;
extern uint32_t useRecordBuffer;
extern uint32_t audioTotalSize;
extern uint32_t audioRemSize;
extern uint16_t *currentPos;

void demoAudioRecord(void) {
	bufferState.state = BUFFER_OFFSET_NONE;
	if (BSP_AUDIO_IN_Init(DEFAULT_AUDIO_IN_FREQ,
	DEFAULT_AUDIO_IN_BIT_RESOLUTION,
	DEFAULT_AUDIO_IN_CHANNEL_NBR) != AUDIO_OK) {
		Error_Handler();
	}

	BSP_LED_On(LED_ORANGE);

	if (BSP_AUDIO_IN_Record(bufPDM, INTERNAL_BUFF_SIZE) != AUDIO_OK) {
		Error_Handler();
	}
	bufferState.ptr = 0;
	audioRecordState = RECORD_WAIT;

	while (audioRecordState != RECORD_DONE) {
		if (bufferState.state == BUFFER_OFFSET_HALF) {
			/* PDM to PCM data convert */
			BSP_AUDIO_IN_PDMToPCM(bufPDM, bufPCM);

			/* Copy PCM data in internal buffer */
			memcpy(&recordBuffer[numRecorded * PCM_SIZE2], bufPCM,
			PCM_SIZE4);

			bufferState.state = BUFFER_OFFSET_NONE;

			if (numRecorded == (RECORD_BUFFER_SIZE / PCM_SIZE4) - 1) {
				audioRecordState = RECORD_BUFFER_FLIP;
				audioBuffOffset = 0;
				numRecorded++;
			} else if (numRecorded == (RECORD_BUFFER_SIZE / PCM_SIZE2) - 1) {
				audioRecordState = RECORD_DONE;
				audioBuffOffset = RECORD_BUFFER_SIZE / 2;
				numRecorded = 0;
			} else {
				numRecorded++;
			}

		}

		if (bufferState.state == BUFFER_OFFSET_FULL) {
			BSP_AUDIO_IN_PDMToPCM(&bufPDM[INTERNAL_BUFF_SIZE / 2], bufPCM);
			memcpy(&recordBuffer[numRecorded * PCM_SIZE2], bufPCM, PCM_SIZE4);

			bufferState.state = BUFFER_OFFSET_NONE;

			if (numRecorded == (RECORD_BUFFER_SIZE / PCM_SIZE4) - 1) {
				audioRecordState = RECORD_BUFFER_FLIP;
				audioBuffOffset = 0;
				numRecorded++;
			} else if (numRecorded == (RECORD_BUFFER_SIZE / PCM_SIZE2) - 1) {
				audioRecordState = RECORD_DONE;
				audioBuffOffset = RECORD_BUFFER_SIZE / 2;
				numRecorded = 0;
			} else {
				numRecorded++;
			}
		}
	}

	if (BSP_AUDIO_IN_Stop() != AUDIO_OK) {
		Error_Handler();
	}

	useRecordBuffer = 1;
	isPressed = 0;

	audioTotalSize = AUDIODATA_SIZE * RECORD_BUFFER_SIZE;
	audioRemSize = 0;
	currentPos = recordBuffer;

	BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_AUTO, 70, DEFAULT_AUDIO_IN_FREQ);
	BSP_AUDIO_OUT_Play(recordBuffer, audioTotalSize);
	BSP_LED_Off(LED_ORANGE);
	BSP_LED_On(LED_BLUE);

	while (!isPressed)
		;

	if (BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW) != AUDIO_OK) {
		Error_Handler();
	}
}

// Calculates the remaining file size and new position of the pointer
void BSP_AUDIO_IN_TransferComplete_CallBack(void) {
	bufferState.state = BUFFER_OFFSET_FULL;
}

// Manages the DMA Half Transfer complete interrupt
void BSP_AUDIO_IN_HalfTransfer_CallBack(void) {
	bufferState.state = BUFFER_OFFSET_HALF;
}

// Audio input error callback
void BSP_AUDIO_IN_Error_Callback(void) {
	Error_Handler();
}
