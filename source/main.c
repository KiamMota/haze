#include "HazeVersion.h"
#include "audio/HazeEngine.h"
#include "fs/InstanceRegistry.h"
#include "fs/Paths.h"
#include "logc/log.h"
#include "server/HazeServer.h"
#include "session/Session.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <uv.h>

#define MODULE_MAIN "MAIN"

void VersionMessage(void) { fprintf(stdout, "haze %s\n", HAZE_VERSION_STR); }

int TraitArgs(int argc, char **argv) {
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
  return 1;
}

int main(int argc, char **argv) {
  if (!TraitArgs(argc, argv)) {
    return 0;
  }

  log_info("[%s] Initializing Haze service (version %s)...", MODULE_MAIN,
           HAZE_VERSION_STR);

  SessionInstance = SessionNew(NULL);
  PathsInstance = PathsNew();

  if (!HazeEngineInit()) {
    log_error("[%s] Failed to start audio engine.", MODULE_MAIN);
    return 1;
  }

  log_info("[%s] Session initialized. Name: %s", MODULE_MAIN,
           SessionGetName(SessionInstance));
  log_info("[%s] Audio engine started successfully.", MODULE_MAIN);

  int port = InstanceRegistryGetLastPort() + 1;
  HazeServer *mainServer = NULL;

  // Loop responsável exclusivamente por encontrar e abrir a porta
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
    break; // Sai do loop assim que conectar com sucesso
  }

  // 1. Grava a instância no arquivo do sistema
  InstanceRegInstance =
      InstanceRegistryNew(SessionInstance, PathsInstance, (unsigned short)port);

  // 2. Configura os sinais de interrupção ANTES de rodar o evento
  HazeServerSetupSignals(mainServer);

  // 3. Executa o loop principal de eventos (bloqueante)
  HazeServerRun(mainServer);

  // 4. Executado somente após o sinal mandar parar o loop (uv_stop)
  log_info("[%s] Shutting down and cleaning up resources...", MODULE_MAIN);

  if (InstanceRegInstance) {
    InstanceRegistryRemove(InstanceRegInstance);
    InstanceRegistryFree(&InstanceRegInstance);
  }

  HazeServerFree(&mainServer);

  log_info("[%s] Haze service shut down gracefully.", MODULE_MAIN);
  return 0;
}
