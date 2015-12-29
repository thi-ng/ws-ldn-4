#ifndef __WAVETABLE_H
#define __WAVETABLE_H

#include <math.h>
#include "synth/synth.h"

#define WAVE_TABLE_LENGTH		1024
#define WAVE_TABLE_SCALE		((float)WAVE_TABLE_LENGTH * INV_TAU)
#define WTABLE_INDEX(x)         ((uint32_t)((x) * WAVE_TABLE_SCALE))
#define WTABLE_LOOKUP(table, x)	(table[WTABLE_INDEX(x)])
#define WTABLE_LOOKUP_RAW(table, idx) (table[idx])

extern const float wtable_sin[WAVE_TABLE_LENGTH];
extern const float wtable_harmonics_1[WAVE_TABLE_LENGTH];
extern const float wtable_harmonics_2[WAVE_TABLE_LENGTH];
extern const float wtable_harmonics_3[WAVE_TABLE_LENGTH];
extern const float wtable_noise[WAVE_TABLE_LENGTH];
extern const float wtable_sin_pow[WAVE_TABLE_LENGTH];
extern const float wtable_sin_pow2[WAVE_TABLE_LENGTH];
extern const float wtable_sin_exp[WAVE_TABLE_LENGTH];
extern const float wtable_sin_exp2[WAVE_TABLE_LENGTH];
extern const float wtable_super_saw[WAVE_TABLE_LENGTH];

#endif
