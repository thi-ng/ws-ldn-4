#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "stm32746g_discovery.h"
#include "stm32746g_discovery_lcd.h"
#include "ex04/audio_play.h"
#include "synth/synth.h"
#include "synth/adsr.h"
#include "synth/iir.h"
#include "synth/biquad.h"
#include "synth/panning.h"
#include "synth/osc.h"
#include "synth/node_ops.h"
#include "synth/delay.h"
#include "macros.h"
#include "ct_math.h"
#include "gui/gui.h"
#include "gui/bt_blackangle48_12.h"

static __IO DMABufferState bufferState = BUFFER_OFFSET_NONE;

static CT_Synth synth;
static uint8_t volume = VOLUME;

static const uint8_t scale[] = { 36, 40, 43, 45, 55, 52, 48, 60 };
uint32_t noteID = 0;
uint32_t voiceID = 0;
uint32_t lastNote = 0;
float filterCutoff = 8000.0f;

static uint8_t audioBuf[AUDIO_DMA_BUFFER_SIZE];

static SpriteSheet dialSheet = { .pixels = bt_blackangle48_12,
		.spriteWidth = 48, .spriteHeight = 48, .numSprites = 12 };

static GUI *gui;
extern __IO GUITouchState touchState;

static void initSynth();
static void updateAudioBuffer();

static void initAppGUI();
static void drawGUI();
static void gui_cb_setVolume(GUIElement *e);
static void gui_cb_setFilter(GUIElement *e);

void demoAudioPlayback(void) {
	BSP_LCD_Clear(LCD_COLOR_BLACK);
	initAppGUI();
	initSynth();
	BSP_LCD_SetFont(&UI_FONT);
	guiUpdate(gui, &touchState);
	if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_HEADPHONE, 0, SAMPLE_RATE) != 0) {
		Error_Handler();
	}
	BSP_AUDIO_OUT_SetVolume(volume);
	BSP_AUDIO_OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_02);
	BSP_AUDIO_OUT_Play((uint16_t *) audioBuf, AUDIO_DMA_BUFFER_SIZE);

	while (1) {
		updateAudioBuffer();
		drawGUI();
	}

	if (BSP_AUDIO_OUT_Stop(CODEC_PDWN_HW) != AUDIO_OK) {
		Error_Handler();
	}
}

static void initAppGUI() {
	gui = initGUI(2);
	gui->items[0] = guiDialButton(0, "Volume", 10, 10, (float)volume / 80.0f, 0.025f, &dialSheet,
			gui_cb_setVolume);
	gui->items[1] = guiDialButton(1, "Freq", 80, 10, filterCutoff / 8000.0f, 0.025f, &dialSheet,
			gui_cb_setFilter);
	guiForceRedraw(gui);
}

static void gui_cb_setVolume(GUIElement *e) {
	DialButtonState *db = (DialButtonState *) (e->userData);
	BSP_AUDIO_OUT_SetVolume((uint8_t) (db->value * 80.0f));
}

static void gui_cb_setFilter(GUIElement *e) {
	DialButtonState *db = (DialButtonState *) (e->userData);
	filterCutoff = 220.0f + db->value * 8000.0f;
}

static void drawGUI() {
	if (touchState.touchUpdate) {
		getTouchState(&touchState);
		guiUpdate(gui, &touchState);
		touchState.touchUpdate = 0;
	}
}

static void initStack(CT_DSPStack *stack, float freq) {
	CT_DSPNode *env = ct_synth_adsr("e", synth.lfo[0], 0.005f, 0.1f, 0.3f,
			1.0f, 0.5f);
	CT_DSPNode *osc1 = ct_synth_osc("a", ct_synth_process_osc_spiral, 0.0f,
			HZ_TO_RAD(freq), 0.25f, 0.0f);
	CT_DSPNode *osc2 = ct_synth_osc("b", ct_synth_process_osc_spiral, 0.0f,
			HZ_TO_RAD(freq * 1.01f), 0.25f, 0.0f);
	CT_DSPNode *sum = ct_synth_op4("s", osc1, env, osc2, env,
			ct_synth_process_madd);
	CT_DSPNode *filter = ct_synth_filter_biquad("f", LPF, sum, filterCutoff,
			12.0f, 0.25f);
	CT_DSPNode *delay = ct_synth_delay("d", filter,
			(int) (SAMPLE_RATE * 0.15f), 0.7f, 1);
	CT_DSPNode *pan = ct_synth_panning("p", delay, NULL, 0.5f);
	//CT_DSPNode *nodes[] = { env, osc1, osc2, sum, filter, delay, pan };
	CT_DSPNode *nodes[] = { env, osc1, osc2, sum, filter, delay, pan };
	ct_synth_init_stack(stack);
	ct_synth_build_stack(stack, nodes, 7);
}

static void initSynth() {
	ct_synth_init(&synth, 2);
	synth.lfo[0] = ct_synth_osc("lfo1", ct_synth_process_osc_sin, 0.0f,
			HZ_TO_RAD(1 / 24.0f), 0.6f, 1.0f);
	synth.numLFO = 1;
	for (uint8_t i = 0; i < synth.numStacks; i++) {
		initStack(&synth.stacks[i], 110.0f);
	}
	ct_synth_collect_stacks(&synth);
}

void updateOscillator(int16_t *ptr, uint32_t frames) {
	if (HAL_GetTick() - lastNote >= 250) {
		lastNote = HAL_GetTick();
		CT_DSPStack *s = &synth.stacks[voiceID];
		ct_synth_reset_adsr(NODE_ID(s, "e"));
		CT_OscState *osc1 = NODE_ID_STATE(CT_OscState, s, "a");
		CT_OscState *osc2 = NODE_ID_STATE(CT_OscState, s, "b");
		osc1->freq = HZ_TO_RAD(ct_synth_notes[scale[noteID]]);
		osc2->freq = HZ_TO_RAD(ct_synth_notes[scale[noteID]] * 0.51f);
		ct_synth_calculate_biquad_coeff(NODE_ID(s, "f"), LPF,
				//(float) ((lastNote * 4) % 11000) + 110.0f, 12.0f, 0.25f);
				filterCutoff, 12.0f, 0.5f);
//		NODE_ID_STATE(CT_PanningState, s, "p")->pos =
//				(voiceID % 2) ? 0.1f : 0.9f;
		ct_synth_activate_stack(s);
		noteID = (noteID + 1) % 8;
		voiceID = (voiceID + 1) % synth.numStacks;
	}
	ct_synth_update_mix_stereo_i16(&synth, frames, ptr);
}

static void updateAudioBuffer() {
	if (bufferState == BUFFER_OFFSET_HALF) {
		int16_t *ptr = (int16_t*) &audioBuf[0];
		//ct_synth_update_mix_stereo_i16(&synth, AUDIO_DMA_BUFFER_SIZE2, ptr);
		updateOscillator(ptr, AUDIO_DMA_BUFFER_SIZE8);
		bufferState = BUFFER_OFFSET_NONE;
	} else if (bufferState == BUFFER_OFFSET_FULL) {
		int16_t *ptr = (int16_t*) &audioBuf[0] + AUDIO_DMA_BUFFER_SIZE4;
		//ct_synth_update_mix_stereo_i16(&synth, AUDIO_DMA_BUFFER_SIZE2, ptr);
		updateOscillator(ptr, AUDIO_DMA_BUFFER_SIZE8);
		bufferState = BUFFER_OFFSET_NONE;
	}
}

void BSP_AUDIO_OUT_HalfTransfer_CallBack(void) {
	bufferState = BUFFER_OFFSET_HALF;
}

void BSP_AUDIO_OUT_TransferComplete_CallBack(void) {
	bufferState = BUFFER_OFFSET_FULL;
//	BSP_AUDIO_OUT_ChangeBuffer((uint16_t*) audioBuf,
//	AUDIO_DMA_BUFFER_SIZE * 2);
}

void BSP_AUDIO_OUT_Error_CallBack(void) {
	Error_Handler();
}
