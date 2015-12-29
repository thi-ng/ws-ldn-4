#include "ex05/main.h"

#define AUDIO_BUFFER_SIZE 4096
#define VOLUME 80

extern __IO uint32_t isLooping;
extern __IO uint32_t numPressed;
extern __IO AppCommand appCommand;
extern __IO PlaybackState playbackState;
extern AppState appState;
extern uint32_t isRecording;

static uint32_t waveDataLength = 0;
static __IO uint32_t samplesRemaining = 0;

__IO uint32_t ledState;
__IO uint32_t isPlaying = 0;
__IO uint32_t bufferState = BUFFER_OFFSET_NONE;
uint8_t audioBuffer[AUDIO_BUFFER_SIZE];

FIL audioFile;
DIR Directory;

void playWaveFile(uint32_t freq) {
	UINT bytesread = 0;

	/* Start playing */
	isPlaying = 1;
	isLooping = 1;

	/* Initialize Wave player (Codec, DMA, I2C) */
	if (initPlayback(freq) != 0) {
		Error_Handler();
	}

	/* Get Data from USB Flash Disk */
	f_lseek(&audioFile, 0);
	f_read(&audioFile, audioBuffer, AUDIO_BUFFER_SIZE, &bytesread);
	samplesRemaining = waveDataLength - bytesread;

	/* Start playing Wave */
	BSP_AUDIO_OUT_Play((uint16_t*) &audioBuffer[0], AUDIO_BUFFER_SIZE);
	ledState = LED6_TOGGLE;
	playbackState = PLAYBACK_RESUME;
	numPressed = 0;

	while ((samplesRemaining != 0) && (appState != APP_IDLE)) {
		if (appCommand == CMD_PLAY) {
			if (playbackState == PLAYBACK_PAUSE) {
				ledState = STOP_TOGGLE;
				togglePlaybackResume(playbackState);
				playbackState = PLAYBACK_IDLE;
			} else if (playbackState == PLAYBACK_RESUME) {
				ledState = LED6_TOGGLE;
				togglePlaybackResume(playbackState);
				playbackState = PLAYBACK_IDLE;
			}

			bytesread = 0;

			if (bufferState == BUFFER_OFFSET_HALF) {
				f_read(&audioFile, &audioBuffer[0],
				AUDIO_BUFFER_SIZE / 2, (void *) &bytesread);

				bufferState = BUFFER_OFFSET_NONE;
			}

			if (bufferState == BUFFER_OFFSET_FULL) {
				f_read(&audioFile, &audioBuffer[AUDIO_BUFFER_SIZE / 2],
				AUDIO_BUFFER_SIZE / 2, (void *) &bytesread);

				bufferState = BUFFER_OFFSET_NONE;
			}
			if (samplesRemaining > (AUDIO_BUFFER_SIZE / 2)) {
				samplesRemaining -= bytesread;
			} else {
				samplesRemaining = 0;
			}
		} else {
			stopPlayback();
			f_close(&audioFile);
			samplesRemaining = 0;
			isLooping = 1;
			break;
		}
	}
	ledState = LEDS_OFF;
	isLooping = 1;
	isPlaying = 0;
	stopPlayback();
	f_close(&audioFile);
}

int initPlayback(uint32_t freq) {
	/* MEMS Accelerometer configure to manage PAUSE, RESUME operations */
	BSP_ACCELERO_Click_ITConfig();
	return BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_AUTO, VOLUME, freq);
}

void startPlayback(void) {
	UINT bytesread = 0;
	char path[] = "0:/";
	char* wavefilename = NULL;
	WavHeader waveformat;

	/* Get the read out protection status */
	if (f_opendir(&Directory, path) == FR_OK) {
		if (isRecording == 1) {
			wavefilename = REC_WAVE_NAME;
		} else {
			wavefilename = WAVE_NAME;
		}
		/* Open the Wave file to be played */
		if (f_open(&audioFile, wavefilename, FA_READ) != FR_OK) {
			BSP_LED_On(LED5);
			appCommand = CMD_RECORD;
		} else {
			/* Read sizeof(WaveFormat) from the selected file */
			f_read(&audioFile, &waveformat, sizeof(waveformat), &bytesread);

			/* Set WaveDataLenght to the Speech Wave length */
			waveDataLength = waveformat.riffLength;

			/* Play the Wave */
			playWaveFile(waveformat.sampleRate);
		}
	}
}

void stopPlayback(void) {
	BSP_AUDIO_OUT_Stop(CODEC_PDWN_HW);
}

void togglePlaybackResume(PlaybackState state) {
	if (state == PLAYBACK_PAUSE) {
		BSP_AUDIO_OUT_Pause();
	} else {
		BSP_AUDIO_OUT_Resume();
	}
}

void BSP_AUDIO_OUT_HalfTransfer_CallBack(void) {
	bufferState = BUFFER_OFFSET_HALF;
}

// Calculates the remaining file size and new position of the pointer.
void BSP_AUDIO_OUT_TransferComplete_CallBack(void) {
	bufferState = BUFFER_OFFSET_FULL;
	BSP_AUDIO_OUT_ChangeBuffer((uint16_t*) &audioBuffer[0],
	AUDIO_BUFFER_SIZE / 2);
}

// Manages the DMA FIFO error interrupt.
void BSP_AUDIO_OUT_Error_CallBack(void) {
	while (1) {
	}
}

void WavePlayer_CallBack(void) {
	if (appState != APP_IDLE) {
		/* Reset the Wave player variables */
		isLooping = 1;
		isPlaying = 0;
		ledState = LEDS_OFF;
		playbackState = PLAYBACK_RESUME;
		waveDataLength = 0;
		numPressed = 0;

		if (BSP_AUDIO_OUT_Stop(CODEC_PDWN_HW) != AUDIO_OK) {
			while (1) {
			};
		}

		/* Turn OFF LED3, LED4 and LED6 */
		BSP_LED_Off(LED3);
		BSP_LED_Off(LED4);
		BSP_LED_Off(LED6);
	}
}
