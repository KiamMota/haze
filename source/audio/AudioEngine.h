#ifndef HAZEENGINE_DEC
#define HAZEENGINE_DEC

#include "miniaudio/miniaudio.h"
#include <stdbool.h>

typedef ma_engine AudioEngine;

AudioEngine* AudioEngineNew(void);
void AudioEngineFree(AudioEngine** hz);

#endif
