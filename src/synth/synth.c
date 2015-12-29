#include <string.h>
#include <stdlib.h>
#include "synth/synth.h"

static tinymt32_t synthRNG;

void synth_osc_init(SynthOsc *osc, OscFn fn, float gain, float phase,
		float freq, float dc) {
	osc->fn = fn;
	osc->phase = phase;
	osc->freq = FREQ_TO_RAD(freq);
	osc->amp = gain;
	osc->dcOffset = dc;
}

void synth_osc_set_wavetables(SynthOsc *osc, const float* tbl1,
		const float* tbl2) {
	osc->wtable1 = tbl1;
	osc->wtable2 = tbl2;
}

float synth_osc_sin(SynthOsc *osc, float lfo, float lfo2) {
	float phase = truncPhase(osc->phase + osc->freq + lfo);
	osc->phase = phase;
	return WTABLE_LOOKUP(wtable_sin, phase) * osc->amp;
}

float synth_osc_sin_math(SynthOsc *osc, float lfo, float lfo2) {
	float phase = truncPhase(osc->phase + osc->freq + lfo);
	osc->phase = phase;
	return sinf(phase) * osc->amp;
}

float synth_osc_sin_dc(SynthOsc *osc, float lfo, float lfo2) {
	float phase = truncPhase(osc->phase + osc->freq + lfo);
	osc->phase = phase;
	return maddf(WTABLE_LOOKUP(wtable_sin, phase), osc->amp, osc->dcOffset);
}

float synth_osc_rect(SynthOsc *osc, float lfo, float lfo2) {
	float phase = truncPhase(osc->phase + osc->freq + lfo);
	osc->phase = phase;
	return stepf(phase, PI, osc->amp, -osc->amp);
}

float synth_osc_rect_phase(SynthOsc *osc, float lfo, float lfo2) {
	float phase = truncPhase(osc->phase + osc->freq + lfo);
	osc->phase = phase;
	return stepf(phase, PI + lfo2, osc->amp, -osc->amp);
}

float synth_osc_rect_dc(SynthOsc *osc, float lfo, float lfo2) {
	float phase = truncPhase(osc->phase + osc->freq + lfo);
	osc->phase = phase;
	return osc->dcOffset + stepf(phase, PI, osc->amp, -osc->amp);
}

float synth_osc_saw(SynthOsc *osc, float lfo, float lfo2) {
	float phase = truncPhase(osc->phase + osc->freq + lfo);
	osc->phase = phase;
	return (phase * INV_PI - 1.0f) * osc->amp;
}

float synth_osc_saw_dc(SynthOsc *osc, float lfo, float lfo2) {
	float phase = truncPhase(osc->phase + osc->freq + lfo);
	osc->phase = phase;
	return osc->dcOffset + (phase * INV_PI - 1.0f) * osc->amp;
}

float synth_osc_tri(SynthOsc *osc, float lfo, float lfo2) {
	float phase = truncPhase(osc->phase + osc->freq + lfo);
	osc->phase = phase;
	float x = 2.0f - (phase * INV_HALF_PI);
	x = 1.0f - stepf(x, 0.0f, -x, x);
	if (x > -1.0f) {
		return x * osc->amp;
	} else {
		return -osc->amp;
	}
}

float synth_osc_tri_dc(SynthOsc *osc, float lfo, float lfo2) {
	float phase = truncPhase(osc->phase + osc->freq + lfo);
	osc->phase = phase;
	float x = 2.0f - (phase * INV_HALF_PI);
	x = 1.0f - stepf(x, 0.0f, -x, x);
	if (x > -1.0f) {
		return maddf(x, osc->amp, osc->dcOffset);
	} else {
		return osc->dcOffset - osc->amp;
	}
}

float synth_osc_wtable_simple(SynthOsc *osc, float lfo, float lfo2) {
	float phase = truncPhase(osc->phase + osc->freq + lfo);
	osc->phase = phase;
	return WTABLE_LOOKUP(osc->wtable1, phase) * osc->amp;
}

float synth_osc_wtable_morph(SynthOsc *osc, float lfo, float morph) {
	float phase = truncPhase(osc->phase + osc->freq + lfo);
	osc->phase = phase;
	uint32_t idx = WTABLE_INDEX(phase);
	return mixf(WTABLE_LOOKUP_RAW(osc->wtable1, idx),
			WTABLE_LOOKUP_RAW(osc->wtable2, idx), morph) * osc->amp;
}

float synth_osc_whitenoise(SynthOsc *osc, float lfo, float lfo2) {
	return NORM_RANDF(&synthRNG) * osc->amp;
}

float synth_osc_whitenoise_dc(SynthOsc *osc, float lfo, float lfo2) {
	return osc->dcOffset + NORM_RANDF(&synthRNG) * osc->amp;
}

float synth_osc_brownnoise(SynthOsc *osc, float lfo, float lfo2) {
	float b = osc->phase;
	while (1) {
		float r = tinymt32_generate_float01(&synthRNG) - 0.5f;
		b += r;
		if (b < -8.0f || b > 8.0f)
			b -= r;
		else
			break;
	}
	osc->phase = b;
	return b * 0.125f * osc->amp;
}

static uint32_t synthPinkIdx = 0;

float synth_osc_pinknoise(SynthOsc *osc, float lfo, float lfo2) {
	float prevr, curr, r, phase;
	uint32_t k = __CLZ(synthPinkIdx) & 0xf;
	prevr = synth_pinknoise_buf[k];
	while (1) {
		curr = r = tinymt32_generate_float01(&synthRNG) - 0.5f;
		r -= prevr;
		phase = osc->phase + r;
		if (phase < -4.0f || phase > 4.0f)
			phase -= r;
		else
			break;
	}
	synth_pinknoise_buf[k] = curr;
	osc->phase = phase;
	synthPinkIdx++;
	return (tinymt32_generate_float01(&synthRNG) - 0.5f + phase) * 0.125f;
}

float synth_osc_nop(SynthOsc *osc, float lfo, float lfo2) {
	return osc->dcOffset;
}

float synth_osc_impulse(SynthOsc *osc, float lfo, float lfo2) {
	float phase = osc->phase + osc->freq + lfo;
	osc->phase = phase;
	return (osc->amp * phase * expf(1.0f - phase)) + osc->dcOffset;
}

void synth_adsr_init(ADSR *env, float attRate, float decayRate,
		float releaseRate, float attGain, float sustainGain) {
	env->attackRate = attRate * ADSR_SCALE
	;
	env->decayRate = decayRate * ADSR_SCALE
	;
	env->releaseRate = releaseRate * ADSR_SCALE
	;
	env->attackGain = attGain * ADSR_SCALE
	;
	env->sustainGain = sustainGain * ADSR_SCALE
	;
	env->phase = ATTACK;
	env->fn = synth_adsr_update_attack;
	env->currGain = 0.0f;
}

float synth_adsr_update_attack(ADSR *env, float envMod) {
	float gain = env->currGain + env->attackRate * envMod;
	if (gain >= env->attackGain) {
		gain = env->attackGain;
		//env->phase = DECAY;
		env->fn = synth_adsr_update_decay;
	}
	return env->currGain = gain;
}

float synth_adsr_update_decay(ADSR *env, float envMod) {
	float gain = env->currGain - env->decayRate * envMod;
	if (gain <= env->sustainGain) {
		gain = env->sustainGain;
		//env->phase = RELEASE; // skip SUSTAIN phase for now
		env->fn = synth_adsr_update_release;
	}
	return env->currGain = gain;
}

float synth_adsr_update_release(ADSR *env, float envMod) {
	float gain = env->currGain - env->releaseRate;
	if (gain < 3e-5f) { // ~0.98 in 16bit
		env->currGain = 0.0f;
		env->phase = IDLE;
		env->fn = synth_adsr_update_idle;
	}
	return env->currGain = gain;
}

float synth_adsr_update_idle(ADSR *env, float envMod) {
	return env->currGain;
}

void synth_voice_init(SynthVoice *voice, uint32_t flags) {
	synth_osc_init(&(voice->lfoPitch), synth_osc_nop, 0.0f, 0.0f, 0.0f, 0.0f);
	synth_osc_init(&(voice->lfoMorph), synth_osc_nop, 0.0f, 0.0f, 0.0f, 0.0f);
#ifdef SYNTH_USE_FILTER
	synth_init_iir(&(voice->filter[0]), IIR_HP, 0.0f, 0.85f, 0.5f);
	synth_init_iir(&(voice->filter[1]), IIR_HP, 0.0f, 0.85f, 0.5f);
#endif
	voice->age = 0;
	voice->flags = flags;
}

void synth_init(Synth *synth) {
	synth->nextVoice = 0;
	for (uint8_t i = 0; i < SYNTH_POLYPHONY; i++) {
		SynthVoice *voice = synth_new_voice(synth);
		synth_adsr_init(&(voice->env), 0.0025f, 0.00025f, 0.00005f, 1.0f,
				0.25f);
		voice->env.phase = IDLE;
	}
	synth_osc_init(&(synth->lfoFilter), synth_osc_nop, 0.0f, 0.0f, 0.0f, 0.0f);
	synth_osc_init(&(synth->lfoEnvMod), synth_osc_nop, 0.0f, 0.0f, 0.0f, 0.0f);
	tinymt32_init(&synthRNG, SYNTH_RNG_SEED);
}

SynthVoice* synth_new_voice(Synth *synth) {
	SynthVoice* voice = NULL;
	uint32_t maxAge = 0;
	for (uint32_t i = 0; i < SYNTH_POLYPHONY; i++) {
		if (synth->voices[i].env.phase == IDLE) {
			voice = &synth->voices[i];
			break;
		}
		if (synth->voices[i].age > maxAge) {
			voice = &synth->voices[i];
			maxAge = synth->voices[i].age;
		}
	}
	synth_voice_init(voice, 0);
	return voice;
}

void synth_bus_init(SynthFXBus *bus, int16_t *buf, size_t len, uint8_t decay) {
	if (bus->buf != NULL) {
		free(bus->buf);
	}
	bus->buf = buf;
	bus->len = len;
	bus->readPos = 1;
	bus->writePos = 0;
	bus->readPtr = &buf[bus->readPos];
	bus->writePtr = &buf[bus->writePos];
	bus->decay = decay;
	memset(buf, 0, len << 1);
}

void synth_init_iir(SynthFilter *state, FilterType type, float cutoff,
		float reso, float damping) {
	state->f[0] = 0.0f; // lp
	state->f[1] = 0.0f; // hp
	state->f[2] = 0.0f; // bp
	state->f[3] = 0.0f; // br
	state->type = type;
	state->fn = synth_process_iir;
	synth_set_iir_coeff(state, cutoff, reso, damping);
}

void synth_set_iir_coeff(SynthFilter *iir, float cutoff, float reso,
		float damping) {
	iir->cutoff = cutoff;
	iir->resonance = reso;
	iir->freq = 2.0f * sinf(PI * fminf(0.25f, cutoff / (SAMPLERATE * 2.0f)));
	iir->damp = fminf(2.0f * (1.0f - powf(reso, damping)),
			fminf(2.0f, 2.0f / iir->freq - iir->freq * 0.5f));
}

float synth_process_iir(SynthFilter *state, float input) {
	float *f = state->f;
	float damp = state->damp;
	float freq = state->freq;
	// 1st pass
	f[3] = input - damp * f[2];
	*f += freq * f[2];
	f[1] = f[3] - *f;
	f[2] += freq * f[1];
	float output = f[state->type];

	// 2nd pass
	f[3] = input - damp * f[2];
	*f += freq * f[2];
	f[1] = f[3] - *f;
	f[2] += freq * f[1];
	return 0.5f * (output + f[state->type]);
}

void synth_init_4pole(SynthFilter *state, float cutoff, float reso) {
	float *f = state->f;
	float *g = state->g;
	f[0] = g[0] = 0.0f;
	f[1] = g[1] = 0.0f;
	f[2] = g[2] = 0.0f;
	f[3] = g[3] = 0.0f;
	state->fn = synth_process_4pole;
	synth_set_4pole_coeff(state, cutoff, reso);
}

void synth_set_4pole_coeff(SynthFilter *state, float cutoff, float reso) {
	float fc = 10.0f * cutoff * INV_NYQUIST_FREQ;
	float ff;
	if (fc <= 1.0f) {
		ff = fc * fc;
	} else {
		ff = fc = 1.0f;
	}
	state->cutoff = cutoff;
	state->resonance = reso;
	state->freq = ff;
	state->damp = 1.0f - fc;
}

float synth_process_4pole(SynthFilter *state, float input) {
	float *f = state->f;
	float *g = state->g;
	float ff = state->freq;

	input -= g[3] * state->resonance * (1.0f - 0.15f * ff);
	input *= 0.35013f * ff * ff;

	ff = state->damp;

	g[0] = input + 0.3f * f[0] + ff * g[0];
	g[1] = g[0] + 0.3f * f[1] + ff * g[1];
	g[2] = g[1] + 0.3f * f[2] + ff * g[2];
	g[3] = g[2] + 0.3f * f[3] + ff * g[3];

	f[0] = input;
	f[1] = g[0];
	f[2] = g[1];
	f[3] = g[2];
	return g[3];
}

void synth_render_slice(Synth *synth, int16_t *ptr, size_t len) {
	int32_t sumL, sumR;
	SynthOsc *lfoEnvMod = &synth->lfoEnvMod;
	SynthFXBus *fx = &synth->bus[0];
	while (len--) {
		sumL = sumR = 0;
		float envMod = lfoEnvMod->fn(lfoEnvMod, 0.0f, 0.0f);
		SynthVoice *voice = &synth->voices[SYNTH_POLYPHONY - 1];
		while (voice >= synth->voices) {
			ADSR *env = &voice->env;
			if (env->phase) {
				voice->age++;
				float gain = env->fn(env, envMod);
				SynthOsc *osc = &voice->lfoPitch;
				float lfoVPitchVal = osc->fn(osc, 0.0f, 0.0f);
				osc = &voice->lfoMorph;
				float lfoVMorphVal = osc->fn(osc, 0.0f, 0.0f);
				osc = &voice->osc[0];
#ifdef SYNTH_USE_FILTER
				SynthFilter *flt = &voice->filter[0];
				sumL += (int32_t) (gain * flt->fn(flt, osc->fn(osc, lfoVPitchVal, lfoVMorphVal)));
				osc++;
				flt++;
				sumR += (int32_t) (gain * flt->fn(flt, osc->fn(osc, lfoVPitchVal, lfoVMorphVal)));
#else
				sumL += (int32_t) (gain * osc->fn(osc, lfoVPitchVal, lfoVMorphVal));
				osc++;
				sumR += (int32_t) (gain * osc->fn(osc, lfoVPitchVal, lfoVMorphVal));
#endif
			}
			voice--;
		}
#ifdef SYNTH_USE_DELAY
		int16_t d = *(fx->readPtr++);
		sumL += d;
		sumR += d;
		fx->readPos++;
		if (fx->readPos >= fx->len) {
			fx->readPos = 0;
			fx->readPtr = &fx->buf[0];
		}
#endif
		*ptr++ = __SSAT(sumL, SYNTH_SATURATE_BITS) << SYNTH_OVERDRIVE_BITS; // signed saturate & opt. overdrive
		*ptr++ = __SSAT(sumR, SYNTH_SATURATE_BITS) << SYNTH_OVERDRIVE_BITS;
#ifdef SYNTH_USE_DELAY
		*(fx->writePtr++) = __SSAT((sumL + sumR) >> fx->decay, 16);
		fx->writePos++;
		if (fx->writePos >= fx->len) {
			fx->writePos = 0;
			fx->writePtr = &fx->buf[0];
		}
#endif
	}
}
