#include "HazeSample.h"
#include "HazeEngine.h"
#include <stdlib.h>
#include <string.h>

bool HazeSampleInit(HazeSample *s, const char *sample_name, const uint8_t *data,
                    size_t size) {
  if (!s || !data || size == 0)
    return false;
  s->buf = malloc(size);
  if (!s->buf)
    return false;
  memcpy(s->buf, data, size);
  s->buf_size = size;
  if (ma_decoder_init_memory(s->buf, size, NULL, &s->decoder) != MA_SUCCESS) {
    free(s->buf);
    s->buf = NULL;
    return false;
  }
  if (ma_sound_init_from_data_source(HazeEngineGet(), &s->decoder, 0, NULL,
                                     &s->handle) != MA_SUCCESS) {
    ma_decoder_uninit(&s->decoder);
    free(s->buf);
    s->buf = NULL;
    return false;
  }
  ma_uint64 frames;
  ma_sound_get_length_in_pcm_frames(&s->handle, &frames);
  ma_uint32 rate = ma_engine_get_sample_rate(HazeEngineGet());
  s->duration = (float)frames / rate;
  s->volume = 1.0f;
  s->pitch = 1.0f;
  s->sample_name = malloc(sizeof(char));
  s->sample_name = memcpy(s->sample_name, sample_name, strlen(sample_name) + 1);
  return true;
}

bool HazeSampleInitFromFile(HazeSample *s, const char *path) {
    if (!s || !path)
        return false;

    if (ma_sound_init_from_file(
            HazeEngineGet(), path, 0, NULL, NULL, &s->handle
        ) != MA_SUCCESS)
        return false;

    ma_uint64 frames;
    ma_sound_get_length_in_pcm_frames(&s->handle, &frames);

    s->sample_rate = ma_engine_get_sample_rate(HazeEngineGet());
    s->duration = (float)frames / s->sample_rate;

    s->volume = 1.0f;
    s->pitch = 1.0f;
    s->buf = NULL;
    s->buf_size = 0;

    const char *name = strrchr(path, '/');
    name = name ? name + 1 : path;

    s->sample_name = malloc(strlen(name) + 1);
    if (!s->sample_name)
        return false;

    strcpy(s->sample_name, name);

    return true;
}

bool HazeSampleSeek(HazeSample *s, double seconds) {
  if (!s)
    return false;
  ma_uint32 rate = ma_engine_get_sample_rate(HazeEngineGet());
  ma_uint64 frame = (ma_uint64)(seconds * rate);
  return ma_sound_seek_to_pcm_frame(&s->handle, frame) == MA_SUCCESS;
}

bool HazeSampleSetVolume(HazeSample *s, float v) {
    if (!s)
        return false;

    if (v < 0.0f)
        v = 0.0f;

    if (v > 1.0f)
        v = 1.0f;

    s->volume = v;
    ma_sound_set_volume(&s->handle, v);

    return true;
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
  free(s->buf);
  s->buf = NULL;
}

float HazeSampleGetVolume(HazeSample *s) {
  if (!s)
    return 0.0f;
  return s->volume;
}

float HazeSampleGetPitch(HazeSample *s) {
  if (!s)
    return 0.0f;
  return s->pitch;
}

float HazeSampleGetDuration(HazeSample *s) {
  if (!s)
    return 0.0f;
  return s->duration;
}

float HazeSampleGetSampleRate(HazeSample *s) {
  if (!s)
    return 0.0f;

  return s->sample_rate;
}

float HazeSampleGetCursor(HazeSample *s) {
  if (!s)
    return 0.0f;
  ma_uint64 cursor;
  ma_sound_get_cursor_in_pcm_frames(&s->handle, &cursor);
  return (float)cursor / s->sample_rate;
}
bool HazeSampleIsPlaying(HazeSample *s) {
  return ma_sound_is_playing(&s->handle);
}

const char* HazeSampleGetName(HazeSample* s) {
  return s->sample_name;
}
