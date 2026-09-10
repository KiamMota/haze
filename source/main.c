#include "HazeVersion.h"
#include "Result.h"
#include "audio/HazeEngine.h"
#include "fs/Paths.h"
#include "fs/Running.h"
#include "logc/log.h"
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

  log_info("[%s] Initializing Haze service (version %s)...", MODULE_MAIN,
           HAZE_VERSION_STR);

  PathsInstance = PathsNew();
  assert(PathsInstance);
  if (!PathsInstance) {
    log_error("[%s] Failed to load haze paths.", MODULE_MAIN);
    return 1;
  }

  SessionInstance = SessionNew(NULL);
  log_info("[%s] Session initialized. Name: %s", MODULE_MAIN,
           SessionGetName(SessionInstance));

  if (!HazeEngineInit()) {
    log_error("[%s] Failed to start audio engine.", MODULE_MAIN);
    return 1;
  }
  log_info("[%s] Audio engine started successfully.", MODULE_MAIN);

  Running *main_running = RunningNew(SessionGetName(SessionInstance),
                                     RunningGetLastPort(), RN_STATUS_UP);
  int port = RunningGetLastPort();
  HazeServer *mainServer = NULL;

  while (1) {
    mainServer = HazeServerNew(NULL, port);

    if (!mainServer) {
      log_error("[%s] Failed to create Haze Server instance.", MODULE_MAIN);
      return 1;
    }

    int err = HazeServerStart(mainServer);

    if (err == UV_EADDRINUSE || err == UV_EACCES) {
      log_warn("[%s] Port %d is unavailable (%s), trying port %d...",
               MODULE_MAIN, port, uv_strerror(err), port + 1);

      HazeServerFree(&mainServer);
      port++;
      continue;
    }

    if (err != 0) {
      log_error("[%s] Failed to start server: %s", MODULE_MAIN,
                uv_strerror(err));
      HazeServerFree(&mainServer);
      return 1;
    }

    log_info("[%s] Haze Server successfully started on %s:%d", MODULE_MAIN,
             HazeServerAddress(mainServer), HazeServerPort(mainServer));
    break;
  }

  Result res = RunningWriteFile(main_running);
  if (!ResultIsOk(res))
    log_error("Failed to write .running: %s", res.msg);
  RunningFree(&main_running);

  HazeServerRun(mainServer);

  log_warn("[%s] HazeServerRun returned unexpectedly.", MODULE_MAIN);
  HazeServerFree(&mainServer);

  log_info("[%s] Haze service shut down gracefully.", MODULE_MAIN);
  return 0;
}
