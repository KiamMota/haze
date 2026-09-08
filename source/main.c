#include "logc/log.h"
#include "HazeVersion.h"
#include "audio/HazeEngine.h"
#include "fs/Paths.h"
#include "server/HazeServer.h"
#include "session/Session.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <uv.h>

#define MODULE_MAIN "MAIN"

void VersionMessage(void) { fprintf(stdout, "haze %s\n", HAZE_VERSION_STR); }

int main(int argc, char **argv) {
  // check arguments
  if (argc > 1) {
    const char *flag = argv[1];
    if (strlen(flag) == 1) {
      printf("error: too short flag.\n");
      return 0;
    }
    if (strcmp(flag, "-v") == 0 || strcmp(flag, "--version") == 0) {
      VersionMessage();
      return 0;
    } else {
      printf("unknown flag: '%s'.\n", flag);
      return 0;
    }
  }

  // log configuration (exemplo opcional para arquivo e terminal)
  FILE *log_file = fopen("haze.log", "w");
  if (log_file) {
    log_add_fp(log_file, LOG_INFO);
  }

  log_info("[%s] Initializing Haze service (version %s)...", MODULE_MAIN, HAZE_VERSION_STR);

  log_info("[%s] Loading application paths...", MODULE_MAIN);
  PathsInstance = PathsNew();
  assert(PathsInstance);
  if (!PathsInstance) {
    log_error("[%s] Failed to load haze paths.", MODULE_MAIN);
    return 1;
  }

  log_info("[%s] Initializing session instance...", MODULE_MAIN);
  SessionInstance = SessionNew(NULL);
  log_info("[%s] Session initialized. Name: %s", MODULE_MAIN, SessionGetName(SessionInstance));

  log_info("[%s] Starting audio engine...", MODULE_MAIN);
  if (!HazeEngineInit()) {
    log_error("[%s] Failed to start audio engine.", MODULE_MAIN);
    return 1;
  }
  log_info("[%s] Audio engine started successfully.", MODULE_MAIN);

  log_info("[%s] Starting headless Haze environment...", MODULE_MAIN);
  log_info("[%s] Preparing to start Haze Server...", MODULE_MAIN);

  int port = 7192;
  HazeServer *mainServer = NULL;

  while (1) {
    log_info("[%s] Attempting to create Haze Server instance on port %d...", MODULE_MAIN, port);
    mainServer = HazeServerNew(NULL, port);

    if (!mainServer) {
      log_error("[%s] Failed to create Haze Server instance.", MODULE_MAIN);
      return 1;
    }

    log_info("[%s] Starting Haze Server on port %d...", MODULE_MAIN, port);
    int err = HazeServerStart(mainServer);

    if (err == UV_EADDRINUSE || err == UV_EACCES) {
      log_warn("[%s] Port %d is unavailable (%s), trying port %d...", MODULE_MAIN, port, uv_strerror(err), port + 1);

      HazeServerFree(&mainServer);
      port++;
      continue;
    }

    if (err != 0) {
      log_error("[%s] Failed to start server: %s", MODULE_MAIN, uv_strerror(err));
      HazeServerFree(&mainServer);
      return 1;
    }

    log_info("[%s] Haze Server successfully started on %s:%d", MODULE_MAIN, HazeServerAddress(mainServer), HazeServerPort(mainServer));
    break;
  }

  log_info("[%s] Entering main event loop (HazeServerRun)...", MODULE_MAIN);
  HazeServerRun(mainServer);

  log_warn("[%s] HazeServerRun returned unexpectedly.", MODULE_MAIN);
  log_info("[%s] Server process terminating, cleaning up resources...", MODULE_MAIN);
  HazeServerFree(&mainServer);

  log_info("[%s] Haze service shut down gracefully.", MODULE_MAIN);
  return 0;
}
