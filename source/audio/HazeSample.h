#ifndef HAZESAMPLE_DEC
#define HAZESAMPLE_DEC
#include "miniaudio/miniaudio.h"
#include "HazeMacros.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

EXPORT_CPP_BEGIN
typedef struct {
  ma_sound handle;
  ma_decoder decoder;
  uint8_t *buf;
  size_t buf_size;
  float volume;
  float pitch;
  float duration;
  ma_uint32 sample_rate;
  char* sample_name;
} HazeSample;

bool HazeSampleInit(HazeSample *s, const char* sample_name, const uint8_t *data, size_t size);
bool HazeSampleInitFromFile(HazeSample *s, const char *path);
bool HazeSampleSeek(HazeSample *s, double seconds);
bool HazeSampleSetVolume(HazeSample *s, float v);
bool HazeSamplePlay(HazeSample *s);
bool HazeSampleStop(HazeSample *s);
void HazeSampleFree(HazeSample *s);

// getters

bool HazeSampleIsPlaying(HazeSample *s);
float HazeSampleGetVolume(HazeSample *s);
float HazeSampleGetPitch(HazeSample *s);
float HazeSampleGetDuration(HazeSample *s);
float HazeSampleGetSampleRate(HazeSample *s);
float HazeSampleGetCursor(HazeSample* s);
const char* HazeSampleGetName(HazeSample* s);

EXPORT_CPP_END
#endif
