#ifndef CHANNEL_LIST_H
#define CHANNEL_LIST_H

#include "audio/AudioEngine.h"
#include "audio/Channel.h"
#include "audio/ResultAudio.h"

typedef struct {
  size_t quantity;
  Channel** channels;
} ChannelList;

ChannelList* ChannelListNew(const AudioEngine* eng);
void ChannelListFree(ChannelList** chann);
ResultAudio ChannelListAdd(ChannelList* clist, const AudioEngine* eng);

#endif
