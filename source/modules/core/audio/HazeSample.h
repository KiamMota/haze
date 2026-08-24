#ifndef HAZESAMPLE_DEC
#define HAZESAMPLE_DEC

#include "miniaudio.h"
#include <klib/kbuffer.h>
#include <klib/ktime.h>

typedef struct {
  ma_sound handle;
  ma_decoder decoder;
  KBuffer *buf;
} HazeSample;

bool HazeSampleInit(HazeSample* s, KBuffer* b);
bool HazeSampleSeek(HazeSample* s, KTime* t);
bool HazeSampleSetVolume(HazeSample* s, float v);
bool HazeSamplePlay(HazeSample* s);
bool HazeSampleStop(HazeSample* s);
void HazeSampleFree(HazeSample* s);

#endif
