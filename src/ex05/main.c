#include "ex05/main.h"

#define NUM_USER_FNS 1

static AppState appState = APP_IDLE;
static __IO PlaybackState playbackState = PLAYBACK_IDLE;

USBH_HandleTypeDef hUSBHost;

static uint8_t midiReceiveBuffer[MIDI_BUF_SIZE];
static uint8_t audioBuffer[AUDIO_DMA_BUFFER_SIZE];

static __IO DMABufferState bufferState = BUFFER_OFFSET_NONE;
static __IO uint32_t canReceive = 0;
static tinymt32_t rng;

TIM_HandleTypeDef TimHandle;

static SynthPreset synthPresets[] = { { .osc1Gain = 0.3f, .osc2Gain = 0.3f,
		.detune = 0.5066f, .filterCutoff = 8000.0f, .filterQ = 0.9f, .feedback =
				0.5f, .width = 0.66f, .osc1Fn = 4, .osc2Fn = 5, .filterType = 0,
		.tempo = 150, .attack = 0.01f, .decay = 0.05f, .sustain = 0.25f,
		.release = 0.66f, .string = 0.02f, .volume = VOLUME } };

static SynthPreset *preset = &synthPresets[0];

static CT_DSPNodeHandler oscFunctions[] = { ct_synth_process_osc_spiral,
		ct_synth_process_osc_sin, ct_synth_process_osc_square,
		ct_synth_process_osc_saw, ct_synth_process_osc_sawsin,
		ct_synth_process_osc_tri };

static CT_BiquadType filterTypes[] = { LPF, HPF, BPF, PEQ };

static CT_Synth synth;
static SeqTrack* tracks[2];

static const uint8_t scale[] = { 36, 40, 43, 45, 55, 52, 48, 60, 52, 55, 45, 48,
		36, 43, 31, 33 };
static int8_t transpose[] = { -12, -5, 0, 7, 12 };
static int8_t notes1[] = { -1, -1, -1, -1, -1, -1, -1, -1 };
static int8_t notes2[] = { -1, -1, -1, -1, -1, -1, -1, -1 };

static __IO uint32_t voiceID = 0;
static __IO uint32_t transposeID = 2;

static void usbUserProcess(USBH_HandleTypeDef *pHost, uint8_t vId);
static void midiApplication();
static void processMidiPackets();
static void initTimer(uint16_t period);
static void initSynth();
static void initAudio();
static void initSequencer();
static void resumePlayback();
static void rewindPlayback();
static void pausePlayback();
static void stopPlayback();
static void playNote(CT_Synth* synth, SeqTrack *track, int8_t note,
		uint32_t tick);

static void updateAudioBuffer();
static void renderAudio(int16_t *ptr, uint32_t frames);

int main(void) {
	CPU_CACHE_Enable();
	HAL_Init();
	SystemClock_Config();
	BSP_LED_Init(LED_GREEN);
	initTimer(20);

	tinymt32_init(&rng, 0xdecafbad);

	USBH_Init(&hUSBHost, usbUserProcess, 0);
	USBH_RegisterClass(&hUSBHost, USBH_MIDI_CLASS);
	USBH_Start(&hUSBHost);

	while (1) {
		midiApplication();
		USBH_Process(&hUSBHost);
	}
}

void usbUserProcess(USBH_HandleTypeDef *usbHost, uint8_t eventID) {
	UNUSED(usbHost);
	switch (eventID) {
	case HOST_USER_SELECT_CONFIGURATION:
		break;
	case HOST_USER_DISCONNECTION:
		appState = APP_DISCONNECT;
		BSP_LED_Off(LED_GREEN);
		break;
	case HOST_USER_CLASS_ACTIVE:
		appState = APP_READY;
		BSP_LED_On(LED_GREEN);
		break;
	case HOST_USER_CONNECTION:
		appState = APP_START;
		break;
	default:
		break;
	}
}

void midiApplication(void) {
	switch (appState) {
	case APP_READY:
		USBH_MIDI_Receive(&hUSBHost, midiReceiveBuffer, MIDI_BUF_SIZE);
		initAudio();
		pausePlayback();
		initSequencer();
		appState = APP_RUNNING;
		break;
	case APP_RUNNING:
		if (canReceive) {
			canReceive = 0;
			USBH_MIDI_Receive(&hUSBHost, midiReceiveBuffer, MIDI_BUF_SIZE);
		}
		break;
	case APP_DISCONNECT:
		appState = APP_IDLE;
		playbackState = PLAYBACK_IDLE;
		stopPlayback();
		USBH_MIDI_Stop(&hUSBHost);
		break;
	default:
		break;
	}
}

void processMidiPackets() {
	uint8_t *ptr = midiReceiveBuffer;
	uint16_t numPackets = USBH_MIDI_GetLastReceivedDataSize(&hUSBHost) >> 2;
	if (numPackets != 0) {
		while (numPackets--) {
			ptr++;
			uint32_t type = *ptr++;
			uint32_t subtype = *ptr++;
			uint32_t val = *ptr++;

			if ((type & 0xf0) == 0xb0) {
				switch (subtype) {
				case MIDI_CC_BT_STOP:
					BSP_LED_Off(LED_GREEN);
					pausePlayback();
					break;
				case MIDI_CC_BT_PLAY:
					BSP_LED_On(LED_GREEN);
					resumePlayback();
					break;
				case MIDI_CC_BT_REWIND:
					rewindPlayback();
					break;
				case MIDI_CC_BT_LEFT:
					if (preset->osc1Fn) {
						preset->osc1Fn--;
					} else {
						preset->osc1Fn = 5;
					}
					break;
				case MIDI_CC_BT_RIGHT:
					preset->osc1Fn = (preset->osc1Fn + 1) % 6;
					break;
				case MIDI_CC_BT_TRACK_LEFT:
					if (transposeID == 0) {
						transposeID = 4;
					} else {
						transposeID--;
					}
					break;
				case MIDI_CC_BT_TRACK_RIGHT:
					transposeID = (transposeID + 1) % 5;
					break;
				case MIDI_CC_SLIDER1:
					tracks[0]->cutoff = 220.0f
							+ expf(4.5f * val / 127.0f - 3.5f) / 2.7f * 8000.0f;
					break;
				case MIDI_CC_SLIDER2:
					tracks[1]->cutoff = 220.0f
							+ expf(4.5f * val / 127.0f - 3.5f) / 2.7f * 8000.0f;
					break;
				case MIDI_CC_KNOB1:
					tracks[0]->resonance = 1.0f - 0.9f * (float) val / 127.0f;
					break;
				case MIDI_CC_KNOB2:
					tracks[1]->resonance = 1.0f - 0.9f * (float) val / 127.0f;
					break;
				case MIDI_CC_KNOB3:
					tracks[0]->damping = 0.9f * (float) val / 127.0f;
					tracks[1]->damping = tracks[0]->damping;
					break;
				case MIDI_CC_KNOB4:
					tracks[0]->attack = 0.005f + 0.5f * (float) val / 127.0f;
					break;
				case MIDI_CC_KNOB5:
					tracks[1]->attack = 0.005f + 0.5f * (float) val / 127.0f;
					break;

				default:
					if (subtype >= MIDI_CC_BT_S1 && subtype <= MIDI_CC_BT_S8) {
						int8_t note = tracks[0]->notes[subtype - MIDI_CC_BT_S1];
						tracks[0]->notes[subtype - MIDI_CC_BT_S1] = (
								(note == -1) ?
										scale[tinymt32_generate_uint32(&rng)
												% 16] :
										-1);
					}
					if (subtype >= MIDI_CC_BT_M1 && subtype <= MIDI_CC_BT_M8) {
						int8_t note = tracks[1]->notes[subtype - MIDI_CC_BT_M1];
						tracks[1]->notes[subtype - MIDI_CC_BT_M1] = (
								(note == -1) ?
										scale[tinymt32_generate_uint32(&rng)
												% 16] :
										-1);
					}
					break;
				}
			}
		}
	}
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
	CT_DSPNode *sum = ct_synth_op4("s", osc1, env, osc2, env,
			ct_synth_process_madd);
	CT_DSPNode *filter = ct_synth_filter_biquad("f",
			filterTypes[preset->filterType], sum, preset->filterCutoff, 12.0f,
			preset->filterQ);
	CT_DSPNode *delay = ct_synth_delay("d", filter,
			(int) (SAMPLE_RATE * 0.375f), preset->feedback, 1);
	CT_DSPNode *pan = ct_synth_panning("p", delay, NULL, 0.5f);
	CT_DSPNode *nodes[] = { env, osc1, osc2, sum, filter, delay, pan };
	ct_synth_init_stack(stack);
	ct_synth_build_stack(stack, nodes, 7);
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

void initAudio(void) {
	initSynth();
	if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_AUTO, 85, SAMPLE_RATE) != 0) {
		Error_Handler();
	}
	BSP_AUDIO_OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_02);
	BSP_AUDIO_OUT_Play((uint16_t*) &audioBuffer[0], AUDIO_DMA_BUFFER_SIZE);
}

void initSequencer(void) {
	tracks[0] = initTrack(0, (SeqTrack*) malloc(sizeof(SeqTrack)), playNote,
			notes1, 8, 250, 1.0f);
	tracks[1] = initTrack(1, (SeqTrack*) malloc(sizeof(SeqTrack)), playNote,
			notes2, 8, 250, 1.0f);
	tracks[0]->attack = 0.005f;
	tracks[0]->decay = 0.1f;
	tracks[1]->attack = 0.005f;
	tracks[1]->decay = 0.1f;
}

void resumePlayback(void) {
	rewindPlayback();
	playbackState = PLAYBACK_RESUME;
	BSP_AUDIO_OUT_Resume();
}

void rewindPlayback(void) {
	tracks[0]->currNote = 0;
	tracks[1]->currNote = 0;
}

void pausePlayback(void) {
	playbackState = PLAYBACK_PAUSE;
	BSP_AUDIO_OUT_Pause();
}

void stopPlayback(void) {
	playbackState = PLAYBACK_IDLE;
	BSP_AUDIO_OUT_Stop(CODEC_PDWN_HW);
}

static void updateAudioBuffer() {
	if (bufferState == BUFFER_OFFSET_HALF) {
		int16_t *ptr = (int16_t*) &audioBuffer[0];
		renderAudio(ptr, AUDIO_DMA_BUFFER_SIZE8);
		bufferState = BUFFER_OFFSET_NONE;
	} else if (bufferState == BUFFER_OFFSET_FULL) {
		int16_t *ptr = (int16_t*) &audioBuffer[0] + AUDIO_DMA_BUFFER_SIZE4;
		renderAudio(ptr, AUDIO_DMA_BUFFER_SIZE8);
		bufferState = BUFFER_OFFSET_NONE;
	}
}

static void renderAudio(int16_t *ptr, uint32_t frames) {
	ct_synth_update_mix_stereo_i16(&synth, frames, ptr);
}

void playNote(CT_Synth* synth, SeqTrack *track, int8_t note, uint32_t tick) {
	CT_DSPStack *s = &synth->stacks[voiceID];
	CT_DSPNode *env = NODE_ID(s, "e");
	float f1 = ct_synth_notes[note + transpose[transposeID]];
	float f2 = f1 * preset->detune;
	CT_DSPNode *a = NODE_ID(s, "a");
	CT_OscState *osc1 = (CT_OscState *) a->state;
	osc1->freq = HZ_TO_RAD(f1);
	osc1->phase = 0;
	osc1->gain = preset->osc1Gain;
	a->handler = oscFunctions[MIN(preset->osc1Fn, 5)];
	CT_DSPNode *b = NODE_ID(s, "b");
	CT_OscState *osc2 = (CT_OscState *) b->state;
	osc2->freq = HZ_TO_RAD(f2);
	osc2->phase = 0;
	osc2->gain = preset->osc2Gain;
	b->handler = oscFunctions[MIN(preset->osc2Fn, 5)];
	ct_synth_configure_adsr(env, track->attack, preset->decay, preset->release,
			1.0f, preset->sustain);
	ct_synth_reset_adsr(env);
	ct_synth_calculate_biquad_coeff(NODE_ID(s, "f"),
			filterTypes[preset->filterType], track->cutoff, 12.0f,
			track->resonance);
	NODE_ID_STATE(CT_DelayState, s, "d")->feedback = track->damping;
	NODE_ID_STATE(CT_PanningState, s, "p")->pos = 0.5f
			+ 0.49f * ((track->id % 2) ? -preset->width : preset->width);
	ct_synth_activate_stack(s);
	voiceID = (voiceID + 1) % synth->numStacks;
}

void USBH_MIDI_ReceiveCallback(USBH_HandleTypeDef *phost) {
	BSP_LED_Toggle(LED_GREEN);
	processMidiPackets();
	canReceive = 1;
	//USBH_MIDI_Receive(&hUSBHost, midiReceiveBuffer, MIDI_BUF_SIZE);
}

void BSP_AUDIO_OUT_HalfTransfer_CallBack() {
	bufferState = BUFFER_OFFSET_HALF;
}

void BSP_AUDIO_OUT_TransferComplete_CallBack() {
	bufferState = BUFFER_OFFSET_FULL;
}

void BSP_AUDIO_OUT_Error_CallBack() {
	BSP_LED_On(LED_GREEN);
	while (1) {
	}
}

static void initTimer(uint16_t period) {
	uint32_t prescaler = (uint32_t) ((SystemCoreClock / 2) / 10000) - 1;
	TimHandle.Instance = TIMx;
	TimHandle.Init.Period = period - 1;
	TimHandle.Init.Prescaler = prescaler;
	TimHandle.Init.ClockDivision = 0;
	TimHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
	TimHandle.Init.RepetitionCounter = 0;
	if (HAL_TIM_Base_Init(&TimHandle) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_Base_Start_IT(&TimHandle) != HAL_OK) {
		Error_Handler();
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	UNUSED(htim);
	if (appState == APP_RUNNING) {
		if ((playbackState == PLAYBACK_PLAY)
				|| (playbackState == PLAYBACK_RESUME)) {
			updateAllTracks(&synth, tracks, 2, HAL_GetTick());
			updateAudioBuffer();
		}
	}
}
