#ifndef AUDIOENGINE_H
#define AUDIOENGINE_H

#include "miniaudio/miniaudio.h"
#include <stdbool.h>

typedef ma_engine AudioEngine;

AudioEngine* AudioEngineNew(void);
void AudioEngineFree(AudioEngine** hz);

#endif
