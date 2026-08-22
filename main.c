#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include <stdio.h>

int main() {
    ma_engine eng;
    if (ma_engine_init(NULL, &eng) != MA_SUCCESS) {
        printf("Engine não iniciada\n");
        return 1;
    }
    ma_engine_play_sound(&eng, "shooting_stars.mp3", NULL);
    getchar();
    ma_engine_uninit(&eng);
    return 0;
}
