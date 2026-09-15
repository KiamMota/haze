#include "Channel.h"
#include "audio/AudioEngine.h"
#include "audio/ResultAudio.h"
#include <stdlib.h>
#include <string.h>

Channel *ChannelNew(const AudioEngine *eng, const Channel *parent) {
  Channel *ch = calloc(1, sizeof(Channel));
  if (!ch)
    return NULL;

  ma_sound_group *parentGroup =
      parent ? (ma_sound_group *)&parent->_chann : NULL;
  if (ma_sound_group_init(eng, 0, parentGroup, &ch->_chann) != MA_SUCCESS) {
    free(ch);
    return NULL;
  }
  ch->chann_name = strdup("Unnamed Channel");
  ch->Volume = 1.0f;
  ch->Pan = 0.0f;
  ch->Muted = false;
  ch->Solo = false;
  return ch;
}

ResultAudio ChannelRename(Channel *c, const char *newName) {
  if (!c || !newName) {
    return ResultAudioErr("invalid argument");
  }

  char *name = strdup(newName);

  if (!name) {
    return ResultAudioErr("Erro allocating name");
  }

  free((char *)c->chann_name);
  c->chann_name = name;

  return ResultAudioOk();
}

void ChannelFree(Channel **ch) {
  if (!ch || !*ch)
    return;
  free((*ch)->chann_name);
  free(*ch);
  *ch = NULL;
}
