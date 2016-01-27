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
#include "gui/bt_dustknob48_12.h"
#include "gui/bt_dustled48_2.h"

static __IO DMABufferState bufferState = BUFFER_OFFSET_NONE;

static CT_Synth synth;
static CT_DSPNodeHandler oscFunctions[] = { ct_synth_process_osc_spiral,
		ct_synth_process_osc_sin, ct_synth_process_osc_square,
		ct_synth_process_osc_saw, ct_synth_process_osc_sawsin,
		ct_synth_process_osc_tri };

static CT_BiquadType filterTypes[] = { LPF, HPF, BPF, PEQ };

static const uint8_t scale[] = { 36, 40, 43, 45, 55, 52, 48, 60, 52, 55, 45, 48,
		36, 43, 31, 33 };
uint32_t noteID = 0;
uint32_t voiceID = 0;
uint32_t lastNote = 0;

static uint8_t volume = VOLUME;
static float osc1Gain = 0.0f;
static float osc2Gain = 0.25f;
static uint8_t osc1Fn = 0;
static uint8_t osc2Fn = 0;
static float filterCutoff = 8000.0f;
static float filterQ = 0.9f;
static uint8_t filterType = 0;
static float feedback = 0.5f;

static uint8_t audioBuf[AUDIO_DMA_BUFFER_SIZE];

static SpriteSheet dialSheet = { .pixels = bt_dustknob48_12_rgb, .spriteWidth =
		48, .spriteHeight = 48, .numSprites = 12, .format = CM_RGB888 };

static SpriteSheet soloSheet = { .pixels = bt_dustled48_2_rgb,
		.spriteWidth = 48, .spriteHeight = 24, .numSprites = 2, .format =
		CM_RGB888 };

static GUI *gui;
extern __IO GUITouchState touchState;

static void initSynth();
static void updateAudioBuffer();

static void initAppGUI();
static void drawGUI();
static void gui_cb_setVolume(GUIElement *e);
static void gui_cb_setOsc1Gain(GUIElement *e);
static void gui_cb_setOsc2Gain(GUIElement *e);
static void gui_cb_setOsc1Fn(GUIElement *e);
static void gui_cb_setOsc2Fn(GUIElement *e);
static void gui_cb_setFilterCutOff(GUIElement *e);
static void gui_cb_setFilterQ(GUIElement *e);
static void gui_cb_setFilterType(GUIElement *e);
static void gui_cb_setFeedback(GUIElement *e);

void demoAudioPlayback(void) {
	BSP_LCD_Clear(UI_BG_COLOR);
	initAppGUI();
	initSynth();
	BSP_LCD_SetFont(&UI_FONT);
	guiUpdate(gui, &touchState);
	if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_HEADPHONE, 70, SAMPLE_RATE) != 0) {
		Error_Handler();
	}
	BSP_AUDIO_OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_02);
	BSP_AUDIO_OUT_SetVolume(volume);
	BSP_AUDIO_OUT_Play((uint16_t *) audioBuf, AUDIO_DMA_BUFFER_SIZE);

	while (1) {
		//updateAudioBuffer();
		drawGUI();
	}

	if (BSP_AUDIO_OUT_Stop(CODEC_PDWN_HW) != AUDIO_OK) {
		Error_Handler();
	}
}

static void initAppGUI() {
	gui = initGUI(14, &UI_FONT, UI_BG_COLOR, UI_TEXT_COLOR);
	gui->items[0] = guiDialButton(0, "MASTER", 15, 10, (float) volume / 80.0f,
	UI_SENSITIVITY, &dialSheet, gui_cb_setVolume);
	gui->items[1] = guiDialButton(1, "OSC1", 95, 10, (float) osc1Gain / 0.25f,
	UI_SENSITIVITY, &dialSheet, gui_cb_setOsc1Gain);
	gui->items[2] = guiDialButton(2, "OSC2", 175, 10, (float) osc2Gain / 0.25f,
	UI_SENSITIVITY, &dialSheet, gui_cb_setOsc2Gain);
	gui->items[3] = guiDialButton(3, "FREQ", 255, 10, filterCutoff / 8000.0f,
	UI_SENSITIVITY, &dialSheet, gui_cb_setFilterCutOff);
	gui->items[4] = guiDialButton(4, "RES", 335, 10, 0.9f - filterQ,
	UI_SENSITIVITY, &dialSheet, gui_cb_setFilterQ);
	gui->items[5] = guiDialButton(5, "DELAY", 415, 10, feedback / 0.9f,
	UI_SENSITIVITY, &dialSheet, gui_cb_setFeedback);
	// OSC1 types
	gui->items[6] = guiPushButton(6, NULL, 95, 90, 1.0f, &soloSheet,
			gui_cb_setOsc1Fn);
	gui->items[7] = guiPushButton(7, NULL, 95, 120, 2.0f, &soloSheet,
			gui_cb_setOsc1Fn);
	gui->items[8] = guiPushButton(8, NULL, 95, 150, 4.0f, &soloSheet,
			gui_cb_setOsc1Fn);
	// OSC2 types
	gui->items[9] = guiPushButton(9, NULL, 175, 90, 1.0f, &soloSheet,
			gui_cb_setOsc2Fn);
	gui->items[10] = guiPushButton(10, NULL, 175, 120, 2.0f, &soloSheet,
			gui_cb_setOsc2Fn);
	gui->items[11] = guiPushButton(11, NULL, 175, 150, 4.0f, &soloSheet,
			gui_cb_setOsc2Fn);
	// Filter types
	gui->items[12] = guiPushButton(6, NULL, 255, 90, 1.0f, &soloSheet,
			gui_cb_setFilterType);
	gui->items[13] = guiPushButton(7, NULL, 255, 120, 2.0f, &soloSheet,
			gui_cb_setFilterType);
	guiForceRedraw(gui);
}

static void gui_cb_setVolume(GUIElement *e) {
	DialButtonState *db = (DialButtonState *) (e->userData);
	BSP_AUDIO_OUT_SetVolume((uint8_t) (db->value * 80.0f));
}

static void gui_cb_setOsc1Gain(GUIElement *e) {
	DialButtonState *db = (DialButtonState *) (e->userData);
	osc1Gain = expf(4.5f * db->value - 3.5f) / 2.7f * 0.25f;
}

static void gui_cb_setOsc2Gain(GUIElement *e) {
	DialButtonState *db = (DialButtonState *) (e->userData);
	osc2Gain = expf(4.5f * db->value - 3.5f) / 2.7f * 0.25f;
}

static void gui_cb_setOsc1Fn(GUIElement *e) {
	uint8_t id = (uint8_t) ((PushButtonState *) (e->userData))->value;
	if (e->state & GUI_ON) {
		osc1Fn |= id;
	} else {
		osc1Fn &= ~id;
	}
}

static void gui_cb_setOsc2Fn(GUIElement *e) {
	uint8_t id = (uint8_t) ((PushButtonState *) (e->userData))->value;
	if (e->state & GUI_ON) {
		osc2Fn |= id;
	} else {
		osc2Fn &= ~id;
	}
}

static void gui_cb_setFilterCutOff(GUIElement *e) {
	DialButtonState *db = (DialButtonState *) (e->userData);
	filterCutoff = 220.0f + expf(4.5f * db->value - 3.5f) / 2.7f * 8000.0f;
}

static void gui_cb_setFilterQ(GUIElement *e) {
	DialButtonState *db = (DialButtonState *) (e->userData);
	filterQ = 1.0f - db->value * 0.9f;
}

static void gui_cb_setFilterType(GUIElement *e) {
	uint8_t id = (uint8_t) ((PushButtonState *) (e->userData))->value;
	if (e->state & GUI_ON) {
		filterType |= id;
	} else {
		filterType &= ~id;
	}
}

static void gui_cb_setFeedback(GUIElement *e) {
	DialButtonState *db = (DialButtonState *) (e->userData);
	feedback = db->value * 0.9f;
}

static void drawGUI() {
	if (touchState.touchUpdate) {
		getTouchState(&touchState);
		guiUpdate(gui, &touchState);
		touchState.touchUpdate = 0;
	}
}

static void initStack(CT_DSPStack *stack, float freq) {
	CT_DSPNode *env = ct_synth_adsr("e", synth.lfo[0], 0.01f, 0.05f, 0.2f, 1.0f,
			0.5f);
	CT_DSPNode *osc1 = ct_synth_osc("a", oscFunctions[osc1Fn], 0.0f,
			HZ_TO_RAD(freq), osc1Gain, 0.0f);
	CT_DSPNode *osc2 = ct_synth_osc("b", oscFunctions[osc2Fn], 0.0f,
			HZ_TO_RAD(freq * 1.01f), osc2Gain, 0.0f);
	CT_DSPNode *sum = ct_synth_op4("s", osc1, env, osc2, env,
			ct_synth_process_madd);
	CT_DSPNode *filter = ct_synth_filter_biquad("f", filterTypes[filterType],
			sum, filterCutoff, 12.0f, filterQ);
	CT_DSPNode *delay = ct_synth_delay("d", filter,
			(int) (SAMPLE_RATE * 0.375f), feedback, 1);
	CT_DSPNode *pan = ct_synth_panning("p", delay, NULL, 0.5f);
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
	if (HAL_GetTick() - lastNote >= 150) {
		lastNote = HAL_GetTick();
		CT_DSPStack *s = &synth.stacks[voiceID];
		ct_synth_reset_adsr(NODE_ID(s, "e"));
		CT_OscState *osc1 = NODE_ID_STATE(CT_OscState, s, "a");
		CT_OscState *osc2 = NODE_ID_STATE(CT_OscState, s, "b");
		osc1->freq = HZ_TO_RAD(ct_synth_notes[scale[noteID]]);
		osc2->freq = HZ_TO_RAD(ct_synth_notes[scale[noteID]] * 0.51f);
		osc1->phase = 0;
		osc2->phase = 0;
		osc1->gain = osc1Gain;
		osc2->gain = osc2Gain;
		NODE_ID(s, "a")->handler = oscFunctions[osc1Fn];
		NODE_ID(s, "b")->handler = oscFunctions[osc2Fn];
		ct_synth_calculate_biquad_coeff(NODE_ID(s, "f"),
				filterTypes[filterType], filterCutoff, 12.0f, filterQ);
		NODE_ID_STATE(CT_DelayState, s, "d")->feedback = feedback;
		NODE_ID_STATE(CT_PanningState, s, "p")->pos =
				(voiceID % 2) ? 0.1f : 0.9f;
		ct_synth_activate_stack(s);
		noteID = (noteID + 1) % 16;
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
}

void BSP_AUDIO_OUT_Error_CallBack(void) {
	Error_Handler();
}

// Callback function run whenever timer caused interrupt
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	UNUSED(htim);
	//__disable_irq();
	updateAudioBuffer();
	//__enable_irq();
}
