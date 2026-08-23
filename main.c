#include "modules/core/audio/HazeEngine.h"
#include "modules/core/audio/HazeSample.h"
#include "modules/core/helpers/log.h"
#include <klib/kfile.h>

int main(void) {
  if (!HazeEngineInit())
    return 1;

  KFile *file = kfile_open("shooting_stars.mp3", KFILE_READ);
  if (!file) {
    HazeLogFatal("arquivo não encontrado");
    kfile_free(&file);
    return 1;
  }

  KBuffer *buf = kfile_read(file);
  kfile_free(&file);
  if (!buf) {
    HazeLogFatal("falha ao ler arquivo");
    return 1;
  }

  HazeSample sound = {0};
  if (!HazeSampleInit(&sound, buf)) {
    HazeLogFatal("falha ao iniciar som");
    return 1;
  }

  HazeLogInfo("tocando — s: stop | p: play | 0: reiniciar | q: sair");
  ma_sound_start(&sound.handle);
  float volume = 1.0f;

  char c;
  while ((c = getchar()) != 'q') {
    if (c == 's')
      HazeSampleStop(&sound);
    if (c == 'p')
      HazeSamplePlay(&sound);
    if (c == '0')
      HazeSampleStop(&sound);
    if (c == '+') {
      volume += 0.1f;
      HazeSampleSetVolume(&sound, volume);
    }
    if (c == '-') {
      volume -= 0.1f;
      HazeSampleSetVolume(&sound, volume);
    }
  }

  HazeSampleFree(&sound);
  HazeEngineFree();
  return 0;
}
