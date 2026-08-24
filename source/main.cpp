#include "modules/HazeLog.h"
#include "modules/core/server/HazeServer.h"
#include <cstdio>
#include <cstring>
#include <string.h>

typedef struct {
  bool headless;
} CliArgs;

CliArgs ParseArgs(int argc, char **argv) {
  CliArgs args;

  if (argc == 1) {
    args.headless = false;
  }
  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--headless") == 0) {
      args.headless = true;
    } 
  }
  return args;
}

int HeadlessMode(CliArgs args) {
  if (!args.headless)
    return 1;
  HazeLogInfo("Starting headless Haze...");
  HazeLogInfo("Starting Haze Server...");
  HazeServer *mainServer = HazeServerNew(NULL, 7192);

  if (mainServer) {
    HazeLogInfo("Haze Server started on %s:%d", HazeServerAddress(mainServer),
                HazeServerPort(mainServer));
    int err = HazeServerStart(mainServer);
    if (err != 0) {
      HazeLogError("Failed to start server: %s", uv_strerror(err));
      return 1;
    }
    HazeServerRun(mainServer); // bloqueia aqui
  }
  HazeLogInfo("Server process terminated.");

  return 0;
}

int main(int argc, char **argv) {
  CliArgs args = ParseArgs(argc, argv);
  if (args.headless){
    return HeadlessMode(args);
  }

  return 0;
}
