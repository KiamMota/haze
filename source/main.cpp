#include "modules/core/audio/HazeEngine.h"
#include "modules/core/audio/HazeSample.h"
#include "modules/Log.h"
#include <cstdio>
#include <cstring>

typedef enum {
  PLAY,
  
} CliCommand;

typedef struct {
  bool headless;
  bool have_command;
  CliCommand cmd;
} CliArgs;

CliArgs ParseArgs(int argc, char** argv) {
  CliArgs args;

  if (argc == 1) {
    args.headless = false;
  }
  if (argc >= 3) {
    args.have_command = true;
  }
  for (int i = 0; i<argc; i++) {
    if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--headless") == 0) {
      args.headless = true;
    } 
    if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--play")) {

    }
  }
  return args;
}

int main(int argc, char** argv) {
  CliArgs args = ParseArgs(argc, argv);
  if (args.headless) {
    printf("starting headlles");
    return 0;
  }
  return 0;
}
