#include "AudioEngine.h"
#include <stdlib.h>


AudioEngine* AudioEngineNew(void) {
    AudioEngine* engine = malloc(sizeof(AudioEngine));
    if (!engine)
        return NULL;

    ma_result result = ma_engine_init(NULL, engine);

    if (result != MA_SUCCESS) {
        free(engine);
        return NULL;
    }

    return engine;
}

void AudioEngineFree(AudioEngine** engine) {
    if (!engine || !*engine)
        return;

    ma_engine_uninit(*engine);
    free(*engine);
    *engine = NULL;
}
