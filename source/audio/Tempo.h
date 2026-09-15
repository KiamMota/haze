#ifndef TEMPO_H
#define TEMPO_H

#include <stddef.h>
#include <stdint.h>

#define TICKS_PER_BEAT 960
typedef struct {
    uint32_t bar;
    uint32_t tick;
    float    bpm;
} TempoPoint;

typedef struct {
    TempoPoint *points;   
    size_t      quantity;
} TempoMap;

uint64_t TempoMapToFrame(const TempoMap* map, uint32_t targetBar, uint32_t targetTick, uint32_t sampleRate, uint32_t beatsPerBar);

#endif
