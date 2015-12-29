#include "synth/sequencer.h"
#include "synth/synth.h"

SeqTrack* initTrack(SeqTrack *track, SeqTrackFn fn, int8_t *notes,
		uint16_t length, uint32_t ticks, float tempoScale) {
	track->fn = fn;
	track->notes = notes;
	track->length = length;
	track->ticks = (uint32_t) (ticks * tempoScale);
	track->lastNoteTick = 0xffffffff;
	track->currNote = 0;
	track->gain = 1.0f;
	track->pitchBend = 0;
	track->tempoScale = tempoScale;
	track->direction = 1;
	track->cutoff = 1000;
	track->resonance = 0.5;
	track->damping = 0.5;
	return track;
}

void updateTrack(Synth *synth, SeqTrack *track, uint32_t tick) {
	if (tick != track->lastNoteTick) {
		if (!(tick % track->ticks)) {
			int16_t curr = track->currNote;
			int8_t note = track->notes[curr];
			if (note >= 0) {
				track->fn(synth, track, note, tick);
			}
			curr = (curr + track->direction) % track->length;
			if (curr < 0) {
				curr = track->length - 1;
			}
			track->currNote = curr;
			track->lastNoteTick = tick;
		}
	}
}

void updateAllTracks(Synth *synth, SeqTrack* *tracks, uint8_t numTracks,
		uint32_t tick) {
	while (numTracks--) {
		updateTrack(synth, *(tracks++), tick);
	}
}
