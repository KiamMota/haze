#include "HazeLog.h"
#include "HazePaths.h"
#include "HazeVersion.h"
#include "session/Session.h"
#include "audio/HazeEngine.h"
#include "fs/Path.h"
#include "server/HazeServer.h"
#include <stdio.h>
#include <string.h>
#include <uv.h>

#define MODULE_MAIN "MAIN"

void VersionMessage(void) { fprintf(stdout, "haze %s\n", HAZE_VERSION_STR); }

int main(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
      VersionMessage();
      return 0;
    }
  }

  HazeLogInfo(MODULE_MAIN, "Initializing Haze service (version %s)...", HAZE_VERSION_STR);

  HazeLogInfo(MODULE_MAIN, "Loading application paths...");
  HazePathsInstance = HazePathsLoad();
  if (!HazePathsInstance) {
    HazeLogError(MODULE_MAIN, "Failed to load haze paths.");
    return 1;
  }
  HazeLogInfo(MODULE_MAIN, "Paths loaded successfully. Projects directory: %s", PathStr(HazePathsInstance->HazeProjects));

  HazeLogInfo(MODULE_MAIN, "Initializing session instance...");
  SessionInstance = SessionNew(NULL);
  HazeLogInfo(MODULE_MAIN, "Session initialized. Name: %s", SessionGetName(SessionInstance));

  HazeLogInfo(MODULE_MAIN, "Starting audio engine...");
  if (!HazeEngineInit()) {
    HazeLogError(MODULE_MAIN, "Failed to start audio engine.");
    return 1;
  }
  HazeLogInfo(MODULE_MAIN, "Audio engine started successfully.");

  HazeLogInfo(MODULE_MAIN, "Starting headless Haze environment...");
  HazeLogInfo(MODULE_MAIN, "Preparing to start Haze Server...");

  int port = 7192;
  HazeServer *mainServer = NULL;

  while (1) {
    HazeLogInfo(MODULE_MAIN, "Attempting to create Haze Server instance on port %d...", port);
    mainServer = HazeServerNew(NULL, port);

    if (!mainServer) {
      HazeLogError(MODULE_MAIN, "Failed to create Haze Server instance.");
      return 1;
    }

    HazeLogInfo(MODULE_MAIN, "Starting Haze Server on port %d...", port);
    int err = HazeServerStart(mainServer);

    if (err == UV_EADDRINUSE || err == UV_EACCES) {
      HazeLogWarn(MODULE_MAIN, "Port %d is unavailable (%s), trying port %d...", port,
                  uv_strerror(err), port + 1);

      HazeServerFree(&mainServer);
      port++;
      continue;
    }

    if (err != 0) {
      HazeLogError(MODULE_MAIN, "Failed to start server: %s", uv_strerror(err));
      HazeServerFree(&mainServer);
      return 1;
    }

    HazeLogInfo(MODULE_MAIN, "Haze Server successfully started on %s:%d", HazeServerAddress(mainServer),
                HazeServerPort(mainServer));
    break;
  }

  HazeLogInfo(MODULE_MAIN, "Entering main event loop (HazeServerRun)...");
  HazeServerRun(mainServer);

  HazeLogWarn(MODULE_MAIN, "HazeServerRun returned unexpectedly.");
  HazeLogInfo(MODULE_MAIN, "Server process terminating, cleaning up resources...");
  HazeServerFree(&mainServer);

  HazeLogInfo(MODULE_MAIN, "Haze service shut down gracefully.");
  return 0;
}
