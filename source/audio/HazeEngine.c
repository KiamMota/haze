#include "HazeEngine.h"

HazeAudioEngine Engine; // definição aqui

bool HazeEngineInit(void) {
    if (ma_engine_init(NULL, &Engine) != MA_SUCCESS)
        return false;
    return true;
}

void HazeEngineFree(void) {
    ma_engine_uninit(&Engine);
}

HazeAudioEngine *HazeEngineGet(void) {
    return &Engine;
}
