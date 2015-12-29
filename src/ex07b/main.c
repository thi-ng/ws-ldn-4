#include "ex07/main.h"

__IO DMABufferState bufferState = BUFFER_OFFSET_NONE;
__IO int32_t isPressed = 0;

uint8_t audioBuffer[AUDIO_BUFFER_SIZE];

static Synth synth;
static SeqTrack* tracks[2];
static uint8_t keyChanges[] = { 0, 5, 7, 8, 12, 19, 24 };

static void playNoteInst1(Synth* synth, SeqTrack *track, int8_t note,
		uint32_t tick);
static void playNoteInst2(Synth* synth, SeqTrack *track, int8_t note,
		uint32_t tick);
static void updateAudioBuffer(Synth *synth);

static __IO uint32_t transposeID = 0;

int main(void) {
	HAL_Init();
	SystemClock_Config();
	led_all_init();
	BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);
	HAL_Delay(1000);

	if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_AUTO, 85, SAMPLERATE) != 0) {
		Error_Handler();
	}

	synth_init(&synth);
	synth_bus_init(&(synth.bus[0]),
			(int16_t*) malloc(sizeof(int16_t) * DELAY_LENGTH),
			DELAY_LENGTH, 4);
	synth_osc_init(&(synth.lfoEnvMod), synth_osc_sin_dc, 0, 0, 0, 1.0f);
	BSP_AUDIO_OUT_Play((uint16_t*) &audioBuffer[0], AUDIO_BUFFER_SIZE);

	int8_t notes1[16] = { 36, -1, 12, 12, -1, -1, -1, -1, 48, -1, 17, 12, -1,
			-1, 24, 24 };

	int8_t notes2[32] = { 12, -1, -1, -1, 12, -1, -1, -1, 12, -1, -1, -1, 12,
			-1, -1, 7, 12, -1, -1, -1, 12, -1, -1, -1, 12, -1, -1, -1, 12,
			-1, 19, 7 };

	tracks[0] = initTrack((SeqTrack*) malloc(sizeof(SeqTrack)), playNoteInst1,
			notes1, 16, 250, 0.5f);

	tracks[1] = initTrack((SeqTrack*) malloc(sizeof(SeqTrack)), playNoteInst2,
			notes2, 32, 250, 1.0f);

	while (1) {
		uint32_t tick = HAL_GetTick();
		updateAllTracks(&synth, tracks, 2, tick);
		updateAudioBuffer(&synth);
	}
}

void playNoteInst1(Synth* synth, SeqTrack *track, int8_t note, uint32_t tick) {
	float freq = notes[note + 24 + keyChanges[transposeID]];
	SynthVoice *voice = synth_new_voice(synth);
	(&voice->filter[0])->type = IIR_BP;
	(&voice->filter[1])->type = IIR_HP;
	float fcutoff = 3000.0f + 2900.0f * sinf(tick * 0.0015f);
	float fresonance = 0.55f + 0.30f * sinf(tick * 0.0007f);
	synth_set_iir_coeff(&voice->filter[0], fcutoff, fresonance, 0.15f);
	synth_set_iir_coeff(&voice->filter[1], fcutoff, fresonance, 0.15f);

	synth_adsr_init(&(voice->env), 0.25f, 0.0005f, 0.000175f, 1.0f, 0.75f);
	synth_osc_init(&(voice->osc[0]), synth_osc_pinknoise, 0.2f, 0.0f, freq,
			0.0f);
//	synth_osc_set_wavetables(&(voice->osc[0]), wtable_noise, NULL);
	synth_osc_init(&(voice->osc[1]), synth_osc_pinknoise, 0.2f, 0.0f,
			freq * 1.01f, 0.0f);
//	synth_osc_set_wavetables(&(voice->osc[1]), wtable_noise, NULL);
	BSP_LED_Toggle(LED_GREEN);
}

void playNoteInst2(Synth* synth, SeqTrack *track, int8_t note, uint32_t tick) {
	float freq = notes[note + keyChanges[transposeID]];
	SynthVoice *voice = synth_new_voice(synth);
	(&voice->filter[0])->type = IIR_LP;
	(&voice->filter[1])->type = IIR_LP;
	float fcutoff = 524.0f;
	synth_set_iir_coeff(&voice->filter[0], fcutoff, 0.9f, 0.2f);
	synth_set_iir_coeff(&voice->filter[1], fcutoff, 0.9f, 0.2f);
	synth_adsr_init(&(voice->env), 0.05f, 0.000005f, 0.002f, 1.0f, 0.95f);
//	synth_osc_init(&(voice->lfoPitch), synth_osc_sin, FREQ_TO_RAD(5.0f), 0.0f,
//			10.0f, 0.0f);
	synth_osc_init(&(voice->lfoPitch), synth_osc_impulse,
			FREQ_TO_RAD(freq * 3.0f), 0.0f, 20.0f, 0.0f);
	synth_osc_init(&(voice->osc[0]), synth_osc_sin_math, 0.4f, 0.0f, freq,
			0.0f);
	synth_osc_init(&(voice->osc[1]), synth_osc_sin_math, 0.4f, 0.0f,
			freq * 0.99f, 0.0f);
	BSP_LED_Toggle(LED_ORANGE);
}

void updateAudioBuffer(Synth *synth) {
	if (bufferState == BUFFER_OFFSET_HALF) {
		int16_t *ptr = (int16_t*) &audioBuffer[0];
		synth_render_slice(synth, ptr, AUDIO_BUFFER_SIZE >> 3);
		bufferState = BUFFER_OFFSET_NONE;
	}

	if (bufferState == BUFFER_OFFSET_FULL) {
		int16_t *ptr = (int16_t*) &audioBuffer[AUDIO_BUFFER_SIZE >> 1];
		synth_render_slice(synth, ptr, AUDIO_BUFFER_SIZE >> 3);
		bufferState = BUFFER_OFFSET_NONE;
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t pin) {
	if (pin == KEY_BUTTON_PIN) {
		if (!isPressed) {
			BSP_LED_Toggle(LED_BLUE);
			transposeID = (transposeID + 1) % 7;
			isPressed = 1;
		} else {
			isPressed = 0;
		}
	}
}

void BSP_AUDIO_OUT_HalfTransfer_CallBack(void) {
	bufferState = BUFFER_OFFSET_HALF;
}

void BSP_AUDIO_OUT_TransferComplete_CallBack(void) {
	bufferState = BUFFER_OFFSET_FULL;
	BSP_AUDIO_OUT_ChangeBuffer((uint16_t*) &audioBuffer[0],
	AUDIO_BUFFER_SIZE >> 1);
}

void BSP_AUDIO_OUT_Error_CallBack(void) {
	BSP_LED_On(LED_RED);
	while (1) {
	}
}
