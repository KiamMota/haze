#ifndef HAZESAMPLE_DEC
#define HAZESAMPLE_DEC
#include "miniaudio/miniaudio.h"
#include "HazeMacros.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

EXPORT_CPP_BEGIN
typedef struct {
  uint64_t id;
  ma_sound handle;
  ma_decoder decoder;
  uint8_t *buf;
  size_t buf_size;
  float volume;
  float pitch;
  float duration;
  ma_uint32 sample_rate;
  char* sample_name;
} Sample;

bool HazeSampleInit(Sample *s, const char* sample_name, const uint8_t *data, size_t size);
bool HazeSampleInitFromFile(Sample *s, const char *path);
bool HazeSampleSeek(Sample *s, double seconds);
bool HazeSampleSetVolume(Sample *s, float v);
bool HazeSamplePlay(Sample *s);
bool HazeSampleStop(Sample *s);
void HazeSampleFree(Sample **s);

// getters

bool HazeSampleIsPlaying(Sample *s);
float HazeSampleGetVolume(Sample *s);
float HazeSampleGetPitch(Sample *s);
float HazeSampleGetDuration(Sample *s);
float HazeSampleGetSampleRate(Sample *s);
float HazeSampleGetCursor(Sample* s);
const char* HazeSampleGetName(Sample* s);

EXPORT_CPP_END
#endif
