#ifndef HAZESAMPLE_DEC
#define HAZESAMPLE_DEC

#include "miniaudio/miniaudio.h"
#include "HazeMacros.h"
#include "Result.h"
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
    char *sample_name;
} Sample;

Sample *SampleNew(void);
void SampleFree(Sample **s);

Result SampleRename(Sample *s, const char *newName);
Result SampleInit(Sample *s, const char *sample_name, const uint8_t *data, size_t size);
Result SampleInitFromFile(Sample *s, const char *path);
Result SampleSeek(Sample *s, double seconds);
Result SampleSetVolume(Sample *s, float v);
Result SamplePlay(Sample *s);
Result SampleStop(Sample *s);

/* getters */
bool SampleIsPlaying(Sample *s);
float SampleGetVolume(Sample *s);
float SampleGetPitch(Sample *s);
float SampleGetDuration(Sample *s);
float SampleGetSampleRate(Sample *s);
float SampleGetCursor(Sample *s);
const char *SampleGetName(Sample *s);

EXPORT_CPP_END
#endif
