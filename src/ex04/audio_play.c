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
#include "synth/pluck.h"
#include "synth/node_ops.h"
#include "synth/delay.h"
#include "macros.h"
#include "ct_math.h"
#include "gui/gui.h"
#include "gui/bt_dustknob48_12.h"
#include "gui/bt_dustled48_2.h"
#include "ex04/thing64_16.h"

static __IO DMABufferState bufferState = BUFFER_OFFSET_NONE;

static CT_Synth synth;
static CT_DSPNodeHandler oscFunctions[] = { ct_synth_process_osc_spiral,
		ct_synth_process_osc_sin, ct_synth_process_osc_square,
		ct_synth_process_osc_saw, ct_synth_process_osc_sawsin,
		ct_synth_process_osc_tri, ct_synth_process_pluck,
		ct_synth_process_pluck };

static CT_BiquadType filterTypes[] = { LPF, HPF, BPF, PEQ };

static const uint8_t scale[] = { 36, 40, 43, 45, 55, 52, 48, 60, 52, 55, 45, 48,
		36, 43, 31, 33 };

uint32_t noteID = 0;
uint32_t voiceID = 0;
uint32_t lastNote = 0;

static SynthPreset synthPresets[] = { { .osc1Gain = 0, .osc2Gain = 0.2f,
		.detune = 0.5066f, .filterCutoff = 8000.0f, .filterQ = 0.9f, .feedback =
				0.5f, .width = 0.66f, .osc1Fn = 0, .osc2Fn = 0, .filterType = 0,
		.tempo = 150, .attack = 0.01f, .decay = 0.05f, .sustain = 0.25f,
		.release = 0.66f, .string = 0.02f, .volume = VOLUME } };

static SynthPreset *preset = &synthPresets[0];

static uint8_t audioBuf[AUDIO_DMA_BUFFER_SIZE];

static SpriteSheet dialSheet = { .pixels = bt_dustknob48_12_rgb888, .spriteWidth =
		48, .spriteHeight = 48, .numSprites = 12, .format = CM_RGB888 };

static SpriteSheet soloSheet = { .pixels = bt_dustled48_2_rgb888,
		.spriteWidth = 48, .spriteHeight = 24, .numSprites = 2, .format =
		CM_RGB888 };

static GUI *gui;
extern __IO GUITouchState touchState;

static void initSynth();
static void updateAudioBuffer();
static void patchStackPluckOsc(CT_DSPStack *stack, uint8_t oscID);
static void stackUseCommonOsc(CT_DSPStack *stack, uint8_t oscID);

static void initAppGUI();
static void drawGUI();
static void gui_cb_setVolume(GUIElement *e);
static void gui_cb_setOsc1Gain(GUIElement *e);
static void gui_cb_setOsc2Gain(GUIElement *e);
static void gui_cb_setOsc1Fn(GUIElement *e);
static void gui_cb_setOsc2Fn(GUIElement *e);
static void gui_cb_setDetune(GUIElement *e);
static void gui_cb_setFilterCutOff(GUIElement *e);
static void gui_cb_setFilterQ(GUIElement *e);
static void gui_cb_setFilterType(GUIElement *e);
static void gui_cb_setFeedback(GUIElement *e);
static void gui_cb_setTempo(GUIElement *e);
static void gui_cb_setWidth(GUIElement *e);
static void gui_cb_setAttack(GUIElement *e);
static void gui_cb_setDecay(GUIElement *e);
static void gui_cb_setSustain(GUIElement *e);
static void gui_cb_setRelease(GUIElement *e);
static void gui_cb_setString(GUIElement *e);

void demoAudioPlayback(void) {
	BSP_LCD_Clear(UI_BG_COLOR);
	drawBitmapRaw(15, 180, 64, 64, thing64_16, CM_ARGB4444);
	initAppGUI();
	initSynth();
	BSP_LCD_SetFont(&UI_FONT);
	guiUpdate(gui, &touchState);
	if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_HEADPHONE, 70, SAMPLE_RATE) != 0) {
		Error_Handler();
	}
	BSP_AUDIO_OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_02);
	BSP_AUDIO_OUT_SetVolume(preset->volume);
	BSP_AUDIO_OUT_Play((uint16_t *) audioBuf, AUDIO_DMA_BUFFER_SIZE);

	while (1) {
		drawGUI();
		HAL_Delay(16);
	}

	if (BSP_AUDIO_OUT_Stop(CODEC_PDWN_HW) != AUDIO_OK) {
		Error_Handler();
	}
}

static void initAppGUI() {
	gui = initGUI(22, &UI_FONT, UI_BG_COLOR, UI_TEXT_COLOR);
	gui->items[0] = guiDialButton(0, "MASTER", 15, 10,
			(float) (preset->volume) / 80.0f,
			UI_SENSITIVITY, &dialSheet, gui_cb_setVolume);
	gui->items[1] = guiDialButton(1, "OSC1", 95, 10, preset->osc1Gain / 0.2f,
	UI_SENSITIVITY, &dialSheet, gui_cb_setOsc1Gain);
	gui->items[2] = guiDialButton(2, "OSC2", 175, 10, preset->osc2Gain / 0.2f,
	UI_SENSITIVITY, &dialSheet, gui_cb_setOsc2Gain);
	gui->items[3] = guiDialButton(3, "FREQ", 255, 10,
			preset->filterCutoff / 8000.0f,
			UI_SENSITIVITY, &dialSheet, gui_cb_setFilterCutOff);
	gui->items[4] = guiDialButton(4, "RES", 335, 10, 0.9f - preset->filterQ,
	UI_SENSITIVITY, &dialSheet, gui_cb_setFilterQ);
	gui->items[5] = guiDialButton(5, "DELAY", 415, 10, preset->feedback / 0.9f,
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
	gui->items[12] = guiPushButton(12, NULL, 255, 90, 1.0f, &soloSheet,
			gui_cb_setFilterType);
	gui->items[13] = guiPushButton(13, NULL, 255, 120, 2.0f, &soloSheet,
			gui_cb_setFilterType);
	// Tempo
	gui->items[14] = guiDialButton(14, "TEMPO", 15, 90,
			1.0f - preset->tempo / 900.0f,
			UI_SENSITIVITY, &dialSheet, gui_cb_setTempo);
	// Panning width
	gui->items[15] = guiDialButton(15, "WIDTH", 415, 90, preset->width,
	UI_SENSITIVITY, &dialSheet, gui_cb_setWidth);
	// Envelope
	gui->items[16] = guiDialButton(16, "ATTACK", 95, 180, preset->attack,
	UI_SENSITIVITY, &dialSheet, gui_cb_setAttack);
	gui->items[17] = guiDialButton(17, "DECAY", 175, 180, preset->decay,
	UI_SENSITIVITY, &dialSheet, gui_cb_setDecay);
	gui->items[18] = guiDialButton(18, "SUSTAIN", 255, 180, preset->sustain,
	UI_SENSITIVITY, &dialSheet, gui_cb_setSustain);
	gui->items[19] = guiDialButton(19, "RELEASE", 335, 180, preset->release,
	UI_SENSITIVITY, &dialSheet, gui_cb_setRelease);
	// OSC2 detune
	gui->items[20] = guiDialButton(20, "DETUNE", 335, 90,
			(preset->detune - 0.5f) * 50.0f,
			UI_SENSITIVITY, &dialSheet, gui_cb_setDetune);
	// Karplus-Strong param
	gui->items[21] = guiDialButton(21, "STRING", 415, 180, preset->string * 33.0f,
	UI_SENSITIVITY, &dialSheet, gui_cb_setString);
	guiForceRedraw(gui);
}

static void gui_cb_setVolume(GUIElement *e) {
	DialButtonState *db = (DialButtonState *) (e->userData);
	preset->volume = (uint8_t) (db->value * 90.0f);
	BSP_AUDIO_OUT_SetVolume(preset->volume);
}

static void gui_cb_setTempo(GUIElement *e) {
	DialButtonState *db = (DialButtonState *) (e->userData);
	preset->tempo = 150 + (uint16_t) ((1.0f - db->value) * 5.0f) * 150;
}

static void gui_cb_setOsc1Gain(GUIElement *e) {
	DialButtonState *db = (DialButtonState *) (e->userData);
	preset->osc1Gain = expf(4.5f * db->value - 3.5f) / 2.7f * 0.2f;
}

static void gui_cb_setOsc2Gain(GUIElement *e) {
	DialButtonState *db = (DialButtonState *) (e->userData);
	preset->osc2Gain = expf(4.5f * db->value - 3.5f) / 2.7f * 0.2f;
}

static void gui_cb_setDetune(GUIElement *e) {
	DialButtonState *db = (DialButtonState *) (e->userData);
	preset->detune = 0.5f + db->value * 0.02f;
}

static void gui_cb_setOsc1Fn(GUIElement *e) {
	uint8_t id = (uint8_t) ((PushButtonState *) (e->userData))->value;
	if (e->state & GUI_ON) {
		preset->osc1Fn |= id;
	} else {
		preset->osc1Fn &= ~id;
	}
}

static void gui_cb_setOsc2Fn(GUIElement *e) {
	uint8_t id = (uint8_t) ((PushButtonState *) (e->userData))->value;
	if (e->state & GUI_ON) {
		preset->osc2Fn |= id;
	} else {
		preset->osc2Fn &= ~id;
	}
}

static void gui_cb_setFilterCutOff(GUIElement *e) {
	DialButtonState *db = (DialButtonState *) (e->userData);
	preset->filterCutoff = 220.0f
			+ expf(4.5f * db->value - 3.5f) / 2.7f * 8000.0f;
}

static void gui_cb_setFilterQ(GUIElement *e) {
	DialButtonState *db = (DialButtonState *) (e->userData);
	preset->filterQ = 1.0f - db->value * 0.9f;
}

static void gui_cb_setFilterType(GUIElement *e) {
	uint8_t id = (uint8_t) ((PushButtonState *) (e->userData))->value;
	if (e->state & GUI_ON) {
		preset->filterType |= id;
	} else {
		preset->filterType &= ~id;
	}
}

static void gui_cb_setFeedback(GUIElement *e) {
	DialButtonState *db = (DialButtonState *) (e->userData);
	preset->feedback = db->value * 0.95f;
}

static void gui_cb_setWidth(GUIElement *e) {
	DialButtonState *db = (DialButtonState *) (e->userData);
	preset->width = db->value;
}

static void gui_cb_setAttack(GUIElement *e) {
	DialButtonState *db = (DialButtonState *) (e->userData);
	preset->attack = 0.002f + db->value * 0.99f;
}

static void gui_cb_setDecay(GUIElement *e) {
	DialButtonState *db = (DialButtonState *) (e->userData);
	preset->decay = 0.005f + db->value * 0.5f;
}

static void gui_cb_setSustain(GUIElement *e) {
	DialButtonState *db = (DialButtonState *) (e->userData);
	preset->sustain = db->value * 0.99f;
}

static void gui_cb_setRelease(GUIElement *e) {
	DialButtonState *db = (DialButtonState *) (e->userData);
	preset->release = 0.002f + db->value * 0.99f;
}

static void gui_cb_setString(GUIElement *e) {
	DialButtonState *db = (DialButtonState *) (e->userData);
	preset->string = 0.001f + db->value * 0.03f;
}
static void drawGUI() {
	if (touchState.touchUpdate) {
		getTouchState(&touchState);
		guiUpdate(gui, &touchState);
		touchState.touchUpdate = 0;
	}
}

static void patchStackPluckOsc(CT_DSPStack *stack, uint8_t oscID) {
	CT_DSPNode *pluck = NODE_ID(stack, oscID ? "bp" : "ap");
	CT_NodeOp4State *sum = NODE_ID_STATE(CT_NodeOp4State, stack, "s");
	if (oscID) {
		sum->bufC = pluck->buf;
	} else {
		sum->bufA = pluck->buf;
	}
	NODE_ID(stack, oscID ? "b" : "a")->flags = 0;
	pluck->flags = NODE_ACTIVE;
}

static void stackUseCommonOsc(CT_DSPStack *stack, uint8_t oscID) {
	CT_DSPNode *osc = NODE_ID(stack, oscID ? "b" : "a");
	CT_NodeOp4State *sum = NODE_ID_STATE(CT_NodeOp4State, stack, "s");
	if (oscID) {
		sum->bufC = osc->buf;
	} else {
		sum->bufA = osc->buf;
	}
	NODE_ID(stack, oscID ? "bp" : "ap")->flags = 0;
	osc->flags = NODE_ACTIVE;
}

static void initStack(CT_DSPStack *stack, float freq) {
	float f1 = HZ_TO_RAD(freq);
	float f2 = HZ_TO_RAD(freq * preset->detune);
	CT_DSPNode *env = ct_synth_adsr("e", synth.lfo[0], preset->attack,
			preset->decay, preset->release, 1.0f, preset->sustain);
	CT_DSPNode *osc1 = ct_synth_osc("a", oscFunctions[preset->osc1Fn], 0.0f, f1,
			preset->osc1Gain, 0.0f);
	CT_DSPNode *osc2 = ct_synth_osc("b", oscFunctions[preset->osc2Fn], 0.0f, f2,
			preset->osc2Gain, 0.0f);
	CT_DSPNode *pluck1 = ct_synth_osc_pluck("ap", f1, 0.005f, preset->osc1Gain,
			0.0f);
	pluck1->flags = 0;
	CT_DSPNode *pluck2 = ct_synth_osc_pluck("bp", f2, 0.005f, preset->osc2Gain,
			0.0f);
	pluck2->flags = 0;
	CT_DSPNode *sum = ct_synth_op4("s", osc1, env, osc2, env,
			ct_synth_process_madd);
	CT_DSPNode *filter = ct_synth_filter_biquad("f",
			filterTypes[preset->filterType], sum, preset->filterCutoff, 12.0f,
			preset->filterQ);
	CT_DSPNode *delay = ct_synth_delay("d", filter,
			(int) (SAMPLE_RATE * 0.375f), preset->feedback, 1);
	CT_DSPNode *pan = ct_synth_panning("p", delay, NULL, 0.5f);
	CT_DSPNode *nodes[] = { env, osc1, osc2, pluck1, pluck2, sum, filter, delay,
			pan };
	ct_synth_init_stack(stack);
	ct_synth_build_stack(stack, nodes, 9);
}

static void initSynth() {
	ct_math_init();
	ct_synth_init(&synth, 3);
	synth.lfo[0] = ct_synth_osc("lfo1", ct_synth_process_osc_sin, 0.0f,
			HZ_TO_RAD(1 / 24.0f), 0.6f, 1.0f);
	synth.numLFO = 1;
	for (uint8_t i = 0; i < synth.numStacks; i++) {
		initStack(&synth.stacks[i], 110.0f);
	}
	ct_synth_collect_stacks(&synth);
}

void renderAudio(int16_t *ptr, uint32_t frames) {
	if (HAL_GetTick() - lastNote >= preset->tempo) {
		lastNote = HAL_GetTick();
		CT_DSPStack *s = &synth.stacks[voiceID];
		CT_DSPNode *env = NODE_ID(s, "e");
		float f1 = ct_synth_notes[scale[noteID]];
		float f2 = ct_synth_notes[scale[noteID]] * preset->detune;
		if (preset->osc1Fn >= 6) {
			patchStackPluckOsc(s, 0);
			CT_DSPNode *a = NODE_ID(s, "ap");
			ct_synth_reset_pluck(a, f1, preset->string, 0.95f);
			((CT_PluckOsc *)a->state)->gain = preset->osc1Gain;
		} else {
			stackUseCommonOsc(s, 0);
			CT_DSPNode *a = NODE_ID(s, "a");
			CT_OscState *osc1 = (CT_OscState *) a->state;
			osc1->freq = HZ_TO_RAD(f1);
			osc1->phase = 0;
			osc1->gain = preset->osc1Gain;
			a->handler = oscFunctions[MIN(preset->osc1Fn, 5)];
		}
		if (preset->osc2Fn >= 6) {
			patchStackPluckOsc(s, 1);
			CT_DSPNode *b = NODE_ID(s, "bp");
			ct_synth_reset_pluck(b, f2, preset->string, 0.95f);
			((CT_PluckOsc *)b->state)->gain = preset->osc2Gain;
		} else {
			stackUseCommonOsc(s, 1);
			CT_DSPNode *b = NODE_ID(s, "b");
			CT_OscState *osc2 = (CT_OscState *) b->state;
			osc2->freq = HZ_TO_RAD(f2);
			osc2->phase = 0;
			osc2->gain = preset->osc2Gain;
			b->handler = oscFunctions[MIN(preset->osc2Fn, 5)];
		}
		ct_synth_configure_adsr(env, preset->attack, preset->decay,
				preset->release, 1.0f, preset->sustain);
		ct_synth_reset_adsr(env);
		ct_synth_calculate_biquad_coeff(NODE_ID(s, "f"),
				filterTypes[preset->filterType], preset->filterCutoff, 12.0f,
				preset->filterQ);
		NODE_ID_STATE(CT_DelayState, s, "d")->feedback = preset->feedback;
		NODE_ID_STATE(CT_PanningState, s, "p")->pos = 0.5f
				+ 0.49f * ((voiceID % 2) ? -preset->width : preset->width);
		ct_synth_activate_stack(s);
		noteID = (noteID + 1) % 16;
		voiceID = (voiceID + 1) % synth.numStacks;
	}
	ct_synth_update_mix_stereo_i16(&synth, frames, ptr);
}

static void updateAudioBuffer() {
	if (bufferState == BUFFER_OFFSET_HALF) {
		int16_t *ptr = (int16_t*) &audioBuf[0];
		renderAudio(ptr, AUDIO_DMA_BUFFER_SIZE8);
		bufferState = BUFFER_OFFSET_NONE;
	} else if (bufferState == BUFFER_OFFSET_FULL) {
		int16_t *ptr = (int16_t*) &audioBuf[0] + AUDIO_DMA_BUFFER_SIZE4;
		renderAudio(ptr, AUDIO_DMA_BUFFER_SIZE8);
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
