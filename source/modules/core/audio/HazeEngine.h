#ifndef HAZEENGINE_DEC
#define HAZEENGINE_DEC

#include "miniaudio.h"
#include <stdbool.h>
typedef ma_engine HazeAudioEngine;

extern HazeAudioEngine Engine;

bool HazeEngineInit(void);
void HazeEngineFree(void);
HazeAudioEngine* HazeEngineGet(void);

#endif
