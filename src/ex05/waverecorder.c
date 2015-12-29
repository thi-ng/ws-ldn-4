#include "ex05/main.h"
#include "ex05/waverecorder.h"
#include <string.h>

typedef struct {
	DMABufferState state;
	uint32_t ptr;
} AudioBufferState;

extern __IO AppCommand appCommand;
extern __IO LEDToggleState ledState;
extern __IO uint32_t ticksRecorded;
extern AppState appState;

__IO uint32_t numRecorded = 0;

uint32_t isRecording = 0;
uint16_t recordBuffer[RECORD_BUFFER_SIZE];

static uint16_t bufPCM[PCM_SIZE2];/* PCM stereo samples are saved in RecBuf */
static uint16_t bufPDM[INTERNAL_BUFF_SIZE];

AudioBufferState bufferState;
WavHeader wav;

FIL wavFile;
__IO uint32_t isAudioDataReady = 0, audioBufOffset = 0;
__IO FRESULT res;

static void initWavHeader(uint32_t freq, WavHeader* header);
static void updateWavHeader(WavHeader* wav, uint32_t length);

/*  
 A single MEMS microphone MP45DT02 mounted on STM32F4-Discovery is connected
 to the Inter-IC Sound (I2S) peripheral. The I2S is configured in master
 receiver mode. In this mode, the I2S peripheral provides the clock to the MEMS
 microphone through CLK_IN and acquires the data (Audio samples) from the MEMS
 microphone through PDM_OUT.

 Data acquisition is performed in 16-bit PDM format and using I2S IT mode.

 In order to avoid data-loss, two buffers are used:
 - When there are enough data to be transmitted to USB, there is a buffer reception
 switch.

 To avoid data-loss:
 - IT ISR priority must be set at a higher priority than USB, this priority
 order must be respected when managing other interrupts;
 */
uint8_t startWaveRecorder(uint16_t* buf, uint32_t size) {
	bufferState.state = BUFFER_OFFSET_NONE;
	BSP_AUDIO_IN_Init(DEFAULT_AUDIO_IN_FREQ, DEFAULT_AUDIO_IN_BIT_RESOLUTION,
	DEFAULT_AUDIO_IN_CHANNEL_NBR);
	return BSP_AUDIO_IN_Record(buf, size);
}

uint32_t stopWaveRecorder(void) {
	return BSP_AUDIO_IN_Stop();
}

void recordWaveFile(void) {
	uint32_t byteswritten = 0;
	ledState = LEDS_OFF;

	// Remove file if it exists on USB disk
	f_unlink(REC_WAVE_NAME);

	// Open file to write
	if ((appState == APP_IDLE)
			|| (f_open(&wavFile, REC_WAVE_NAME, FA_CREATE_ALWAYS | FA_WRITE)
					!= FR_OK)) {
		while (1) {
			// USB file error
			BSP_LED_Toggle(LED_RED);
			HAL_Delay(50);
		}
	} else {
		isRecording = 1;
	}
	initWavHeader(DEFAULT_AUDIO_IN_FREQ, &wav);
	f_write(&wavFile, &wav, 44, (void*) &byteswritten);
	bufferState.ptr = byteswritten;
	startWaveRecorder(bufPDM, INTERNAL_BUFF_SIZE);

	ticksRecorded = 0;
	numRecorded = 0;
	ledState = LED3_TOGGLE;

	while (appState != APP_IDLE) {
		if (ticksRecorded <= DEFAULT_TIME_REC) {
			/* Check if there are Data to write in Usb Key */
			if (isAudioDataReady == 1) {
				/* write buffer in file */
				res = f_write(&wavFile, recordBuffer + audioBufOffset,
				RECORD_BUFFER_SIZE, (void*) &byteswritten);
				if (res != FR_OK) {
					Error_Handler();
				}
				bufferState.ptr += byteswritten;
				isAudioDataReady = 0;
			}

			if (appCommand != CMD_RECORD) {
				stopWaveRecorder();
				appCommand = CMD_PLAY;
				ledState = LED6_TOGGLE;
				break;
			}
		} else {
			stopWaveRecorder();
			appCommand = CMD_STOP;
			ledState = LED4_TOGGLE;
			isAudioDataReady = 0;
			break;
		}
	}

	f_lseek(&wavFile, 0);
	updateWavHeader(&wav, bufferState.ptr);
	f_write(&wavFile, &wav, sizeof(WavHeader), (void*) &byteswritten);
	f_close(&wavFile);
	f_mount(NULL, 0, 1);

	appCommand = CMD_PLAY;
}

/**
 * @brief  Encoder initialization.
 * @param  freq: Sampling frequency.
 * @param  pHeader: Pointer to the WAV file header to be written.
 * @retval 0 if success, !0 else.
 */
static void initWavHeader(uint32_t freq, WavHeader* wav) {
	wav->riffTag = 'R' | 'I' << 8 | 'F' << 16 | 'F' << 24;
	wav->waveTag = 'W' | 'A' << 8 | 'V' << 16 | 'E' << 24;
	wav->formatTag = 'f' | 'm' << 8 | 't' << 16 | ' ' << 24;
	wav->dataTag = 'd' | 'a' << 8 | 't' << 16 | 'a' << 24;
	wav->riffLength = 0;
	wav->formatLength = 16;
	wav->audioFormat = 1;
	wav->numChannels = 2;
	wav->sampleRate = freq;
	wav->byteRate = (freq * (16 / 8) * 2); // freq * bits / 8 * channels
	wav->blockAlign = 2 * (16 / 8); // channels * (wav.bits / 8);
	wav->bits = 16;
	wav->dataLength = 0;
}

static void updateWavHeader(WavHeader* wav, uint32_t length) {
	wav->riffLength = length - 8;
	wav->dataLength = length - sizeof(WavHeader);
}

void BSP_AUDIO_IN_TransferComplete_CallBack(void) {
	BSP_AUDIO_IN_PDMToPCM(&bufPDM[INTERNAL_BUFF_SIZE / 2], bufPCM);
	memcpy(&recordBuffer[numRecorded * PCM_SIZE2], bufPCM, PCM_SIZE4);

	bufferState.state = BUFFER_OFFSET_NONE;

	if (numRecorded == (RECORD_BUFFER_SIZE / PCM_SIZE4) - 1) {
		isAudioDataReady = 1;
		audioBufOffset = 0;
		numRecorded++;
	} else if (numRecorded == (RECORD_BUFFER_SIZE / PCM_SIZE2) - 1) {
		isAudioDataReady = 1;
		audioBufOffset = RECORD_BUFFER_SIZE / 2;
		numRecorded = 0;
	} else {
		numRecorded++;
	}
}

/**
 * @brief  Manages the DMA Half Transfer complete interrupt.
 * @param  None
 * @retval None
 */
void BSP_AUDIO_IN_HalfTransfer_CallBack(void) {
	BSP_AUDIO_IN_PDMToPCM(bufPDM, bufPCM);
	memcpy(&recordBuffer[numRecorded * PCM_SIZE2], bufPCM, PCM_SIZE4);

	bufferState.state = BUFFER_OFFSET_NONE;

	if (numRecorded == (RECORD_BUFFER_SIZE / PCM_SIZE4) - 1) {
		isAudioDataReady = 1;
		audioBufOffset = 0;
		numRecorded++;
	} else if (numRecorded == (RECORD_BUFFER_SIZE / PCM_SIZE2) - 1) {
		isAudioDataReady = 1;
		audioBufOffset = RECORD_BUFFER_SIZE / 2;
		numRecorded = 0;
	} else {
		numRecorded++;
	}
}
