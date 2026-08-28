#ifndef HAZEENGINE_DEC
#define HAZEENGINE_DEC

#include "miniaudio/miniaudio.h"
#include "HazeMacros.h"
#include <stdbool.h>
EXPORT_CPP_BEGIN
typedef ma_engine HazeAudioEngine;

extern HazeAudioEngine Engine;

bool HazeEngineInit(void);
void HazeEngineFree(void);
HazeAudioEngine* HazeEngineGet(void);
EXPORT_CPP_END

#endif
