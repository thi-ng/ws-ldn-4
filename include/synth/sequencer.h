#ifndef __SEQUENCER_H_
#define __SEQUENCER_H_

#include "synth/synth.h"

typedef struct SeqTrack SeqTrack;

typedef void (*SeqTrackFn)(Synth *synth, SeqTrack *track, int8_t note, uint32_t tick);
typedef void (*SeqTrackUserFn)(SeqTrack *track, SynthVoice *voice, float freq, uint32_t tick);

struct SeqTrack {
	SeqTrackFn fn;
	SeqTrackUserFn userFn;
	int8_t *notes;
	uint16_t length;
	int16_t currNote;
	uint32_t ticks;
	uint32_t lastNoteTick;
	int32_t pitchBend;
	int32_t direction;
	float gain;
	float tempoScale;
	float cutoff;
	float resonance;
	float damping;
	float attack;
	float decay;
};

SeqTrack* initTrack(SeqTrack *track, SeqTrackFn fn, int8_t *notes, uint16_t length, uint32_t ticks, float tempoScale);
void updateTrack(Synth *synth, SeqTrack *track, uint32_t tick);
void updateAllTracks(Synth *synth, SeqTrack* *tracks, uint8_t numTracks, uint32_t tick);

#endif
