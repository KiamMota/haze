#include "Sample.h"
#include "HazeEngine.h"
#include "HazeMacros.h"
#include "Result.h"
#include <stdlib.h>
#include <string.h>

Sample *SampleNew(void)
{
    Sample *s = malloc(sizeof(Sample));
    if (!s)
        return NULL;

    memset(s, 0, sizeof(Sample));
    s->sample_name = malloc(1);
    if (!s->sample_name) {
        free(s);
        return NULL;
    }
    s->sample_name[0] = '\0';
    s->volume = 1.0f;
    s->pitch = 1.0f;

    return s;
}

Result SampleInit(Sample *s, const char *sample_name, const uint8_t *data, size_t size)
{
    if (!s)
        return ResultMsgE("sample is null");
    if (!data || size == 0)
        return ResultMsgE("invalid data");
    if (!sample_name)
        return ResultMsgE("sample name is null");

    s->buf = malloc(size);
    if (!s->buf)
        return ResultMsgE("out of memory");

    memcpy(s->buf, data, size);
    s->buf_size = size;

    if (ma_decoder_init_memory(s->buf, size, NULL, &s->decoder) != MA_SUCCESS) {
        free(s->buf);
        s->buf = NULL;
        return ResultMsgE("failed to init decoder");
    }

    if (ma_sound_init_from_data_source(HazeEngineGet(), &s->decoder, 0, NULL, &s->handle) != MA_SUCCESS) {
        ma_decoder_uninit(&s->decoder);
        free(s->buf);
        s->buf = NULL;
        return ResultMsgE("failed to init sound");
    }

    ma_uint64 frames;
    ma_sound_get_length_in_pcm_frames(&s->handle, &frames);
    ma_uint32 rate = ma_engine_get_sample_rate(HazeEngineGet());

    s->duration = (float)frames / (float)rate;
    s->sample_rate = rate;
    s->volume = 1.0f;
    s->pitch = 1.0f;

    free(s->sample_name);
    s->sample_name = malloc(strlen(sample_name) + 1);
    if (!s->sample_name) {
        ma_sound_uninit(&s->handle);
        ma_decoder_uninit(&s->decoder);
        free(s->buf);
        s->buf = NULL;
        return ResultMsgE("out of memory");
    }
    strcpy(s->sample_name, sample_name);

    return (Result){ .success = true, .msg = NULL };
}

Result SampleInitFromFile(Sample *s, const char *path)
{
    if (!s)
        return ResultMsgE("sample is null");
    if (!path || !*path)
        return ResultMsgE("invalid path");

    if (ma_sound_init_from_file(HazeEngineGet(), path, 0, NULL, NULL, &s->handle) != MA_SUCCESS)
        return ResultMsgE("failed to load sample from file");

    ma_uint64 frames;
    ma_sound_get_length_in_pcm_frames(&s->handle, &frames);

    s->sample_rate = ma_engine_get_sample_rate(HazeEngineGet());
    s->duration = (float)frames / (float)s->sample_rate;
    s->volume = 1.0f;
    s->pitch = 1.0f;
    s->buf = NULL;
    s->buf_size = 0;

    const char *name = strrchr(path, '/');
    name = name ? name + 1 : path;

    free(s->sample_name);
    s->sample_name = malloc(strlen(name) + 1);
    if (!s->sample_name) {
        ma_sound_uninit(&s->handle);
        return ResultMsgE("out of memory");
    }
    strcpy(s->sample_name, name);

    return (Result){ .success = true, .msg = NULL };
}

Result SampleSeek(Sample *s, double seconds)
{
    if (!s)
        return ResultMsgE("sample is null");

    ma_uint32 rate = ma_engine_get_sample_rate(HazeEngineGet());
    ma_uint64 frame = (ma_uint64)(seconds * rate);

    if (ma_sound_seek_to_pcm_frame(&s->handle, frame) != MA_SUCCESS)
        return ResultMsgE("failed to seek");

    return (Result){ .success = true, .msg = NULL };
}

Result SampleSetVolume(Sample *s, float v)
{
    if (!s)
        return ResultMsgE("sample is null");

    if (v < 0.0f)
        v = 0.0f;
    if (v > 1.0f)
        v = 1.0f;

    s->volume = v;
    ma_sound_set_volume(&s->handle, v);

    return (Result){ .success = true, .msg = NULL };
}

Result SamplePlay(Sample *s)
{
    if (!s)
        return ResultMsgE("sample is null");

    if (ma_sound_start(&s->handle) != MA_SUCCESS)
        return ResultMsgE("failed to play sample");

    return (Result){ .success = true, .msg = NULL };
}

Result SampleStop(Sample *s)
{
    if (!s)
        return ResultMsgE("sample is null");

    if (ma_sound_stop(&s->handle) != MA_SUCCESS)
        return ResultMsgE("failed to stop sample");

    return (Result){ .success = true, .msg = NULL };
}

void SampleFree(Sample **s)
{
    PTR_FREE_ASSERT(s);

    ma_sound_uninit(&(*s)->handle);

    if ((*s)->buf) {
        ma_decoder_uninit(&(*s)->decoder);
        free((*s)->buf);
        (*s)->buf = NULL;
    }

    free((*s)->sample_name);
    free(*s);
    *s = NULL;
}

Result SampleRename(Sample *s, const char *newName)
{
    if (!s)
        return ResultMsgE("sample is null");
    if (!newName)
        return ResultMsgE("new name is null");

    char *copy = strdup(newName);
    if (!copy)
        return ResultMsgE("out of memory");

    free(s->sample_name);
    s->sample_name = copy;

    return ResultOk();
}

/* ===================== getters ===================== */

float SampleGetVolume(Sample *s)
{
    if (!s)
        return 0.0f;
    return s->volume;
}

float SampleGetPitch(Sample *s)
{
    if (!s)
        return 0.0f;
    return s->pitch;
}

float SampleGetDuration(Sample *s)
{
    if (!s)
        return 0.0f;
    return s->duration;
}

float SampleGetSampleRate(Sample *s)
{
    if (!s)
        return 0.0f;
    return (float)s->sample_rate;
}

float SampleGetCursor(Sample *s)
{
    if (!s)
        return 0.0f;

    ma_uint64 cursor = 0;
    ma_sound_get_cursor_in_pcm_frames(&s->handle, &cursor);
    return (float)cursor / (float)s->sample_rate;
}

bool SampleIsPlaying(Sample *s)
{
    if (!s)
        return false;
    return ma_sound_is_playing(&s->handle);
}

const char *SampleGetName(Sample *s)
{
    if (!s)
        return NULL;
    return s->sample_name;
}
