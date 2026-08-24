#include "HazeSample.h"
#include "HazeEngine.h"

bool HazeSampleInit(HazeSample *s, GBytes *buf) {
    if (!s || !buf)
        return false;

    s->buf = g_bytes_ref(buf);

    gsize size = 0;
    gconstpointer data = g_bytes_get_data(s->buf, &size);

    if (ma_decoder_init_memory(
            data,
            size,
            NULL,
            &s->decoder
        ) != MA_SUCCESS) {
        g_bytes_unref(s->buf);
        s->buf = NULL;
        return false;
    }

    if (ma_sound_init_from_data_source(
            HazeEngineGet(),
            &s->decoder,
            0,
            NULL,
            &s->handle
        ) != MA_SUCCESS) {
        ma_decoder_uninit(&s->decoder);
        g_bytes_unref(s->buf);
        s->buf = NULL;
        return false;
    }

    return true;
}

bool HazeSampleSeek(HazeSample *s, gdouble seconds) {
    if (!s)
        return false;

    ma_uint32 sample_rate =
        ma_engine_get_sample_rate(HazeEngineGet());

    ma_uint64 frame =
        (ma_uint64)(seconds * sample_rate);

    return ma_sound_seek_to_pcm_frame(
        &s->handle,
        frame
    ) == MA_SUCCESS;
}

bool HazeSampleSetVolume(HazeSample *s, float v) {
    if (!s)
        return false;

    if (v < 0.0f)
        v = 0.0f;

    if (v > 1.0f)
        v = 1.0f;

    return ma_sound_set_volume(&s->handle, v) == MA_SUCCESS;
}

bool HazeSamplePlay(HazeSample *s) {
    if (!s)
        return false;

    return ma_sound_start(&s->handle) == MA_SUCCESS;
}

bool HazeSampleStop(HazeSample *s) {
    if (!s)
        return false;

    return ma_sound_stop(&s->handle) == MA_SUCCESS;
}

void HazeSampleFree(HazeSample *s) {
    if (!s)
        return;

    ma_sound_uninit(&s->handle);
    ma_decoder_uninit(&s->decoder);

    if (s->buf) {
        g_bytes_unref(s->buf);
        s->buf = NULL;
    }
}
