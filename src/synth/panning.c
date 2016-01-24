#include <math.h>
#include "synth/panning.h"

CT_DSPNode *ct_synth_panning(char *id, CT_DSPNode *src, CT_DSPNode *lfo,
                             float pos) {
    CT_DSPNode *node = ct_synth_node(id, 2);
    CT_PanningState *state =
        (CT_PanningState *)calloc(1, sizeof(CT_PanningState));
    state->src = src->buf;
    state->lfo = (lfo != NULL ? lfo->buf : ct_synth_zero);
    state->pos = pos;
    node->state = state;
    node->handler = ct_synth_process_panning;
    return node;
}

uint8_t ct_synth_process_panning(CT_DSPNode *node, CT_DSPStack *stack,
                                 CT_Synth *synth, uint32_t offset) {
    CT_PanningState *state = (CT_PanningState *)(node->state);
    const float *src = state->src;
    float pos = state->pos + *(state->lfo);
    const float ampL = sqrtf(1.0f - pos) * (SQRT2 / 2);
    const float ampR = sqrtf(pos) * (SQRT2 / 2);
    float *buf = node->buf;
    uint32_t len = AUDIO_BUFFER_SIZE;
    while (len--) {
        *buf++ = *src * ampL;
        *buf++ = *src++ * ampR;
    }
    return 0;
}
