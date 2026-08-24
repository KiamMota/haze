#ifndef HAZESAMPLE_DEC
#define HAZESAMPLE_DEC
#include "miniaudio.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    ma_sound    handle;
    ma_decoder  decoder;
    uint8_t    *buf;
    size_t      buf_size;
} HazeSample;

bool HazeSampleInit(HazeSample *s, const uint8_t *data, size_t size);
bool HazeSampleSeek(HazeSample *s, double seconds);
bool HazeSampleSetVolume(HazeSample *s, float v);
bool HazeSamplePlay(HazeSample *s);
bool HazeSampleStop(HazeSample *s);
void HazeSampleFree(HazeSample *s);
#endif
