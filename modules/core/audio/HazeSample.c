#include "HazeSample.h"
#include "HazeEngine.h"

bool HazeSampleInit(HazeSample *s, KBuffer *buf) {
  if (!s || !buf)
    return false;
  s->buf = buf;
  if (ma_decoder_init_memory(kbuffer_data(buf), kbuffer_len(buf), NULL,
                             &s->decoder) != MA_SUCCESS) {
    return false;
  }
  if (ma_sound_init_from_data_source(HazeEngineGet(), &s->decoder, 0, NULL,
                                     &s->handle) != MA_SUCCESS) {
    ma_decoder_uninit(&s->decoder);
    return false;
  }
  return true;
}

bool HazeSampleSeek(HazeSample *s, KTime* t) {
  if (!s)
    return false;
  double seconds = ktime_to_seconds(*t);
  ma_uint32 sample_rate = ma_engine_get_sample_rate(HazeEngineGet());
  ma_uint64 frame = (ma_uint64)(seconds * sample_rate);
  return ma_sound_seek_to_pcm_frame(&s->handle, frame) == MA_SUCCESS;
}

bool HazeSampleSetVolume(HazeSample *s, float v) {
  if (!s)
    return false;
  if (v < 0.0f)
    v = 0.0f;
  if (v > 1.0f)
    v = 1.0f;
  ma_sound_set_volume(&s->handle, v);
  return true;
}

bool HazeSamplePlay(HazeSample *s) { return ma_sound_start(&s->handle); }

bool HazeSampleplay(HazeSample *s) {
  if (!s)
    return false;
  return ma_sound_stop(&s->handle);
}
bool HazeSampleStop(HazeSample *s) {
  if (!s)
    return false;
  return ma_sound_stop(&s->handle);
}
void HazeSampleFree(HazeSample *s) {
  if (!s)
    return;
  ma_sound_uninit(&s->handle);
  ma_decoder_uninit(&s->decoder);
  kbuffer_free(&s->buf);
}
