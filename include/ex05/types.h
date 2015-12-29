#ifndef __EX05_TYPES_H
#define __EX05_TYPES_H

typedef enum {
	APP_IDLE = 0, APP_START, APP_RUNNING
} AppState;

typedef enum {
	CMD_PLAY = 0, CMD_RECORD, CMD_STOP
} AppCommand;

typedef enum {
	PLAYBACK_PAUSE = 0, PLAYBACK_RESUME, PLAYBACK_IDLE
} PlaybackState;

typedef enum {
	USBH_USER_FS_INIT = 0, USBH_USER_AUDIO
} USBAppState;

typedef enum {
	STOP_TOGGLE = 0,
	LED3_TOGGLE = 3,
	LED4_TOGGLE = 4,
	LED5_TOGGLE = 5,
	LED6_TOGGLE = 6,
	LEDS_OFF = 7
} LEDToggleState;

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

typedef enum {
	BUFFER_OFFSET_NONE = 0, BUFFER_OFFSET_HALF, BUFFER_OFFSET_FULL
} DMABufferState;

#endif
