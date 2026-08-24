#include "HazeSample.h"
#include "HazeEngine.h"
#include <stdlib.h>
#include <string.h>

bool HazeSampleInit(HazeSample *s, const uint8_t *data, size_t size)
{
    if (!s || !data || size == 0) return false;

    s->buf = malloc(size);
    if (!s->buf) return false;
    memcpy(s->buf, data, size);
    s->buf_size = size;

    if (ma_decoder_init_memory(s->buf, size, NULL, &s->decoder) != MA_SUCCESS) {
        free(s->buf); s->buf = NULL;
        return false;
    }

    if (ma_sound_init_from_data_source(HazeEngineGet(), &s->decoder, 0, NULL, &s->handle) != MA_SUCCESS) {
        ma_decoder_uninit(&s->decoder);
        free(s->buf); s->buf = NULL;
        return false;
    }

    return true;
}

bool HazeSampleSeek(HazeSample *s, double seconds)
{
    if (!s) return false;
    ma_uint32 rate  = ma_engine_get_sample_rate(HazeEngineGet());
    ma_uint64 frame = (ma_uint64)(seconds * rate);
    return ma_sound_seek_to_pcm_frame(&s->handle, frame) == MA_SUCCESS;
}

bool HazeSampleSetVolume(HazeSample *s, float v)
{
    if (!s) return false;
    if (v < 0.0f) v = 0.0f;
    if (v > 1.0f) v = 1.0f;
    ma_sound_set_volume(&s->handle, v);
    return true;
}

bool HazeSamplePlay(HazeSample *s)
{
    if (!s) return false;
    return ma_sound_start(&s->handle) == MA_SUCCESS;
}

bool HazeSampleStop(HazeSample *s)
{
    if (!s) return false;
    return ma_sound_stop(&s->handle) == MA_SUCCESS;
}

void HazeSampleFree(HazeSample *s)
{
    if (!s) return;
    ma_sound_uninit(&s->handle);
    ma_decoder_uninit(&s->decoder);
    free(s->buf);
    s->buf = NULL;
}
