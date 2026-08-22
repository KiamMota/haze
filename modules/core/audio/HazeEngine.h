#ifndef HAZEENGINE_DEC
#define HAZEENGINE_DEC

#include "miniaudio.h"
#include <klib/ktypes.h>
typedef ma_engine HazeAudioEngine;

extern HazeAudioEngine Engine;

bool HazeEngineInit(void);
void HazeEngineFree(void);
HazeAudioEngine* HazeEngineGet(void);


#endif
