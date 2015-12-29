#ifndef __EX05_WAVEPLAYER_H
#define __EX05_WAVEPLAYER_H

#include "ex05/types.h"
#include "ex05/main.h"

int initPlayback(uint32_t freq);
void startPlayback(void);
void stopPlayback(void);
void playWaveFile(uint32_t freq);
void togglePlaybackResume(PlaybackState state);
void WavePlayer_CallBack(void);

#endif
