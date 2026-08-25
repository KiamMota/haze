#ifndef HAZEENGINE_DEC
#define HAZEENGINE_DEC

#include "miniaudio.h"
#include "modules/HazeMacros.h"
#include <stdbool.h>
EXPORT_CPP_BEGIN
typedef ma_engine HazeAudioEngine;

extern HazeAudioEngine Engine;

bool HazeEngineInit(void);
void HazeEngineFree(void);
HazeAudioEngine* HazeEngineGet(void);
EXPORT_CPP_END

#endif
