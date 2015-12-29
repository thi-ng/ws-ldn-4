#include "ex06/main.h"

__IO DMABufferState bufferState = BUFFER_OFFSET_NONE;
__IO int32_t isPressed = 0;

uint8_t audioBuffer[AUDIO_BUFFER_SIZE];

static tinymt32_t rng;
static __IO uint32_t transposeID = 0;

static Synth synth;
static SeqTrack* tracks[2];

static uint8_t keyChanges[] = { 0, 5, 7, 9 };

static void playNote(Synth* synth, SeqTrack *track, int8_t note, uint32_t tick);
static void updateAudioBuffer(Synth *synth);

int main(void) {
	HAL_Init();
	SystemClock_Config();
	led_all_init();
	BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);
	HAL_Delay(1000);

	if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_AUTO, 85, SAMPLERATE) != 0) {
		Error_Handler();
	}

	tinymt32_init(&rng, 0x12345678);

	synth_init(&synth);
	synth_bus_init(&(synth.bus[0]),
			(int16_t*) malloc(sizeof(int16_t) * DELAY_LENGTH),
			DELAY_LENGTH, 2);
	synth_osc_init(&(synth.lfoEnvMod), synth_osc_sin_dc, 0, 0, 0, 1.0f);
	BSP_AUDIO_OUT_Play((uint16_t*) &audioBuffer[0], AUDIO_BUFFER_SIZE);

	int8_t notes[] = { 36, -1, 12, 12, -1, -1, -1, -1, 48, -1, 17, 12, -1, -1,
			-1, 24 };
	int8_t notes2[] = { 12, 17, -1, 24, 36, -1 };

	tracks[0] = initTrack((SeqTrack*) malloc(sizeof(SeqTrack)), playNote, notes,
			16, 250, 1.0f);
	tracks[1] = initTrack((SeqTrack*) malloc(sizeof(SeqTrack)), playNote,
			notes2, 6, 250, 1.0f);

	while (1) {
		uint32_t tick = HAL_GetTick();
		updateAllTracks(&synth, tracks, 2, tick);
		updateAudioBuffer(&synth);
	}
}

void playNote(Synth* synth, SeqTrack *track, int8_t note, uint32_t tick) {
	float freq = notes[note + 12 + keyChanges[transposeID]];
	SynthVoice *voice = synth_new_voice(synth);

	(&voice->filter[0])->type=IIR_LP;
	(&voice->filter[1])->type=IIR_HP;
	float fcutoff = 2000.0f + 1900.0f * sinf(tick * 0.0015f);
	float fresonance = 0.55f + 0.30f * sinf(tick * 0.0007f);
	synth_set_iir_coeff(&voice->filter[0], fcutoff, fresonance, 0.25f);
	synth_set_iir_coeff(&voice->filter[1], fcutoff, fresonance, 0.25f);

	synth_adsr_init(&(voice->env), tinymt32_generate_float01(&rng) * 0.025f,
			0.00015f, 0.00005f, 1.0f, 0.5f);
	synth_osc_init(&(voice->lfoPitch), synth_osc_sin, FREQ_TO_RAD(2.0f), 0.0f,
			1.0f, 0.0f);
	synth_osc_init(&(voice->lfoMorph), synth_osc_nop, 0.0f, 0.0f, 0.0f, 0.5f + 0.5f * sinf(PI * 1.5f + tick * 0.00015f));
//	synth_osc_init(&(voice->lfoMorph), synth_osc_sin_dc, 0.499f, 0.0f, 3520.0f + 3520.0f * sinf(PI * 1.5f + tick * 0.000075f), 0.5f);
	synth_osc_init(&(voice->osc[0]), synth_osc_wtable_morph, 0.15f, 0.0f, freq,
			0.0f);
	synth_osc_set_wavetables(&(voice->osc[0]), wtable_sin_exp2, wtable_sin_pow2);
	synth_osc_init(&(voice->osc[1]), synth_osc_wtable_morph, 0.15f, 0.0f,
			freq * 0.501f, 0.0f);
	synth_osc_set_wavetables(&(voice->osc[1]), wtable_sin_exp2,
			wtable_super_saw);
	BSP_LED_Toggle(LED_GREEN);
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
			transposeID = (transposeID + 1) % 4;
			tracks[0]->direction *= -1;
			tracks[1]->direction *= -1;
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
