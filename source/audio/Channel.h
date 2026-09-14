#ifndef HAZECHANNEL_DEC
#define HAZECHANNEL_DEC

#include "Result.h"
#include "audio/AudioEngine.h"
#include <stdbool.h>

typedef struct {
    ma_sound_group _chann;
    float Volume;
    float Pan;
    bool Muted;
    bool Solo;
    char* chann_name;
} Channel;

Channel *ChannelNew(const AudioEngine* eng, const Channel* parent);
Result ChannelRename(Channel* c, const char* newName);
void ChannelFree(Channel **hzc);
float ChannelVolume(Channel *v);
float ChannelPan(Channel *v);
bool ChannelMuted(Channel *m);
bool ChannelSolo(Channel *s);

#endif
