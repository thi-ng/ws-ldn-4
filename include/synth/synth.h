#ifndef __SYNTH_H
#define __SYNTH_H

#include <stdint.h>
#include "arm_math.h"
#include "synth/wavetable.h"
#include "tinymt32.h"

#define TAU						(6.283185307f)
//#define PI						(3.14159265f)
#define HALF_PI					(1.570796326794897f)
#define INV_TAU					(1.0f / TAU)
#define INV_PI					(1.0f / PI)
#define INV_HALF_PI				(1.0f / HALF_PI)

#define SAMPLERATE				(44100)
#define INV_SAMPLERATE			(1.0f / SAMPLERATE)
#define INV_NYQUIST_FREQ		(2.0f / (float)SAMPLERATE)

#ifndef SYNTH_POLYPHONY
#define SYNTH_POLYPHONY			(6)
#endif

#define SYNTH_SATURATE_BITS		16
#define SYNTH_OVERDRIVE_BITS	0

#ifndef AUDIO_BUFFER_SIZE
#define AUDIO_BUFFER_SIZE		(256)
#endif

#ifndef SYNTH_RNG_SEED
#define SYNTH_RNG_SEED			(0xcafebad)
#endif

#define ADSR_SCALE				(32767.0f)
#define INV_ADSR_SCALE			(1.0f / ADSR_SCALE)

//#define SYNTH_USE_DELAY
#define DELAY_LENGTH			(uint32_t)(SAMPLERATE * 0.375f)

#define FREQ_TO_RAD(freq)		((TAU * (freq)) / (float)SAMPLERATE)

#define RANDF(rnd)				tinymt32_generate_float(rnd)
#define NORM_RANDF(rnd)			maddf(RANDF(rnd), 2.0f, -1.0f)

inline float truncPhase(float phase) {
	while (phase >= TAU) {
		phase -= TAU;
	}
	return phase;
}

inline float clampf(float x, float min, float max) {
	return (x < min) ? min : (x > max ? max : x);
}

inline int16_t clamp16(int32_t x) {
	return (int16_t) ((x < -0x7fff) ? -0x8000 : (x > 0x7fff ? 0x7fff : x));
}

inline float stepf(float x, float edge, float y1, float y2) {
	return (x < edge ? y1 : y2);
}

inline float maddf(float a, float b, float c) {
	return a * b + c;
}

inline float mixf(float a, float b, float t) {
	return maddf(b - a, t, a);
}

typedef struct SynthOsc SynthOsc;

typedef float (*OscFn)(SynthOsc*, float lfo, float lfo2);

struct SynthOsc {
	OscFn fn;
	float phase;
	float freq;
	float amp;
	float dcOffset;
	const float *wtable1;
	const float *wtable2;
};

typedef enum {
	IDLE = 0, ATTACK = 1, DECAY = 2, SUSTAIN = 3, RELEASE = 4
} ADSRPhase;

typedef struct ADSR ADSR;

typedef float (*ADSRFn)(ADSR*, float envMod);

struct ADSR {
	float currGain;
	float attackGain;
	float sustainGain;
	float attackRate;
	float decayRate;
	float releaseRate;
	ADSRFn fn;
	ADSRPhase phase;
};

typedef enum {
	IIR_LP = 0, IIR_HP, IIR_BP, IIR_BR
} FilterType;

typedef struct SynthFilter SynthFilter;

typedef float (*FilterFn)(SynthFilter*, float input);

struct SynthFilter {
	float* src;
	float* lfo;
	float f[4];
	float g[4];
	float cutoff;
	float resonance;
	float freq;
	float damp;
	FilterType type;
	FilterFn fn;
};

typedef struct {
	SynthOsc osc[2];
	SynthOsc lfoPitch;
	SynthOsc lfoMorph;
	ADSR env;
	SynthFilter filter[2];
	uint32_t flags;
	uint32_t age;
} SynthVoice;

typedef struct {
	int16_t *buf;
	size_t len;
	uint32_t readPos;
	uint32_t writePos;
	int16_t *readPtr;
	int16_t *writePtr;
	int16_t inL;
	int16_t inR;
	uint8_t decay;
} SynthFXBus;

typedef struct {
	SynthVoice voices[SYNTH_POLYPHONY];
	SynthOsc lfoFilter;
	SynthOsc lfoEnvMod;
	SynthFXBus bus[1];
	uint8_t nextVoice;
} Synth;

float synth_pinknoise_buf[16];

void synth_osc_init(SynthOsc *osc, OscFn fn, float gain, float phase,
		float freq, float dc);
void synth_osc_set_wavetables(SynthOsc *osc, const float *tbl1,
		const float *tbl2);
float synth_osc_sin(SynthOsc *osc, float lfo, float lfo2);
float synth_osc_sin_math(SynthOsc *osc, float lfo, float lfo2);
float synth_osc_sin_dc(SynthOsc *osc, float lfo, float lfo2);
float synth_osc_sin2(SynthOsc *osc, float lfo, float lfo2);
float synth_osc_rect(SynthOsc *osc, float lfo, float lfo2);
float synth_osc_rect_phase(SynthOsc *osc, float lfo, float lfo2);
float synth_osc_rect_dc(SynthOsc *osc, float lfo, float lfo2);
float synth_osc_saw(SynthOsc *osc, float lfo, float lfo2);
float synth_osc_saw_dc(SynthOsc *osc, float lfo, float lfo2);
float synth_osc_tri(SynthOsc *osc, float lfo, float lfo2);
float synth_osc_tri_dc(SynthOsc *osc, float lfo, float lfo2);
float synth_osc_whitenoise(SynthOsc *osc, float lfo, float lfo2);
float synth_osc_whitenoise_dc(SynthOsc *osc, float lfo, float lfo2);
float synth_osc_brownnoise(SynthOsc *osc, float lfo, float lfo2);
float synth_osc_pinknoise(SynthOsc *osc, float lfo, float lfo2);
float synth_osc_nop(SynthOsc *osc, float lfo, float lfo2);
float synth_osc_impulse(SynthOsc *osc, float lfo, float lfo2);
float synth_osc_wtable_simple(SynthOsc *osc, float lfo, float lfo2);
float synth_osc_wtable_morph(SynthOsc *osc, float lfo, float lfo2);

void synth_adsr_init(ADSR *env, float attRate, float decayRate,
		float releaseRate, float attGain, float sustainGain);
float synth_adsr_update_attack(ADSR *env, float envMod);
float synth_adsr_update_decay(ADSR *env, float envMod);
float synth_adsr_update_release(ADSR *env, float envMod);
float synth_adsr_update_idle(ADSR *env, float envMod);

void synth_bus_init(SynthFXBus *bus, int16_t *buf, size_t len, uint8_t decay);

void synth_voice_init(SynthVoice *voice, uint32_t flags);
void synth_init(Synth *synth);

SynthVoice* synth_new_voice(Synth *synth);
void synth_render_slice(Synth *synth, int16_t *ptr, size_t len);

void synth_init_iir(SynthFilter *state, FilterType type, float cutoff,
		float reso, float damping);
void synth_set_iir_coeff(SynthFilter *iir, float cutoff, float reso,
		float damping);
float synth_process_iir(SynthFilter *state, float input);

void synth_init_4pole(SynthFilter *state, float cutoff, float reso);
void synth_set_4pole_coeff(SynthFilter *state, float cutoff, float reso);
float synth_process_4pole(SynthFilter *state, float input);

#endif
