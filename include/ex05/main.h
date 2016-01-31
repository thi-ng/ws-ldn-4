#pragma once

#include <stdio.h>
#include "stm32f7xx_hal.h"
#include "stm32746g_discovery.h"
#include "stm32746g_discovery_audio.h"
#include "usbh_MIDI.h"
#include "synth/synth.h"
#include "synth/adsr.h"
#include "synth/biquad.h"
#include "synth/delay.h"
#include "synth/node_ops.h"
#include "synth/osc.h"
#include "synth/panning.h"
#include "ex05/stm32f7xx_it.h"
#include "ex05/sequencer.h"
#include "clockconfig.h"
#include "macros.h"

#define TIMx              TIM3
#define TIMx_CLK_ENABLE() __HAL_RCC_TIM3_CLK_ENABLE()
#define TIMx_IRQn         TIM3_IRQn
#define TIMx_IRQHandler   TIM3_IRQHandler

#define VOLUME 70

#define AUDIO_DMA_BUFFER_SIZE 1024
#define AUDIO_DMA_BUFFER_SIZE2 (AUDIO_DMA_BUFFER_SIZE >> 1)
#define AUDIO_DMA_BUFFER_SIZE4 (AUDIO_DMA_BUFFER_SIZE >> 2)
#define AUDIO_DMA_BUFFER_SIZE8 (AUDIO_DMA_BUFFER_SIZE >> 3)

#define MIDI_CC_BT_PLAY 41
#define MIDI_CC_BT_STOP 42
#define MIDI_CC_BT_REWIND 43
#define MIDI_CC_BT_LEFT 61
#define MIDI_CC_BT_RIGHT 62
#define MIDI_CC_BT_TRACK_LEFT 58
#define MIDI_CC_BT_TRACK_RIGHT 59

#define MIDI_CC_SLIDER1 0
#define MIDI_CC_SLIDER2 1

#define MIDI_CC_KNOB1 16
#define MIDI_CC_KNOB2 17
#define MIDI_CC_KNOB3 18
#define MIDI_CC_KNOB4 19
#define MIDI_CC_KNOB5 20

#define MIDI_CC_BT_S1 32
#define MIDI_CC_BT_S2 33
#define MIDI_CC_BT_S3 34
#define MIDI_CC_BT_S4 35
#define MIDI_CC_BT_S5 36
#define MIDI_CC_BT_S6 37
#define MIDI_CC_BT_S7 38
#define MIDI_CC_BT_S8 39

#define MIDI_CC_BT_M1 48
#define MIDI_CC_BT_M2 49
#define MIDI_CC_BT_M3 50
#define MIDI_CC_BT_M4 51
#define MIDI_CC_BT_M5 52
#define MIDI_CC_BT_M6 53
#define MIDI_CC_BT_M7 54
#define MIDI_CC_BT_M8 55

#define MIDI_BUF_SIZE 64

typedef enum {
	APP_IDLE = 0, APP_START, APP_READY, APP_RUNNING, APP_DISCONNECT
} AppState;

typedef enum {
	PLAYBACK_PAUSE = 0, PLAYBACK_PLAY, PLAYBACK_RESUME, PLAYBACK_IDLE
} PlaybackState;

typedef enum {
	USBH_USER_FS_INIT = 0, USBH_USER_AUDIO
} USBAppState;

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
