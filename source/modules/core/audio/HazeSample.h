#ifndef HAZESAMPLE_DEC
#define HAZESAMPLE_DEC

#include "miniaudio.h"
#include <glib.h>
#include <stdbool.h>

typedef struct {
    ma_sound handle;
    ma_decoder decoder;
    GBytes *buf;
} HazeSample;

bool HazeSampleInit(HazeSample *s, GBytes *buf);
bool HazeSampleSeek(HazeSample *s, gdouble seconds);
bool HazeSampleSetVolume(HazeSample *s, float v);
bool HazeSamplePlay(HazeSample *s);
bool HazeSampleStop(HazeSample *s);
void HazeSampleFree(HazeSample *s);

#endif
