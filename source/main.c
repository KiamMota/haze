#include "HazeLog.h"
#include "HazeVersion.h"
#include "Session.h"
#include "audio/HazeEngine.h"
#include "server/HazeServer.h"
#include <stdio.h>
#include <string.h>
#include <uv.h> // Incluído assumindo que uv_strerror, UV_EADDRINUSE, etc, vêm daqui

void VersionMessage(void) { 
    fprintf(stdout, "haze %s\n", HAZE_VERSION_STR); 
}

int main(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            VersionMessage();
            return 0;
        }
    }

    Session* s = SessionNew(NULL);
    printf("session name: %s\n", SessionGetName(s));

    HazeLogInfo("starting audio engine...");
    if (!HazeEngineInit()) {
        HazeLogError("failed to start audio engine.");
        return 1;
    }

    HazeLogInfo("Starting headless Haze...");
    HazeLogInfo("Starting Haze Server...");

    int port = 7192;
    HazeServer *mainServer = NULL;

    while (1) {
        mainServer = HazeServerNew(NULL, port);

        if (!mainServer) {
            HazeLogError("Failed to create Haze Server instance.");
            return 1;
        }

        int err = HazeServerStart(mainServer);

        if (err == UV_EADDRINUSE || err == UV_EACCES) {
            HazeLogInfo("Port %d is unavailable (%s), trying %d...", 
                        port, uv_strerror(err), port + 1);

            HazeServerFree(&mainServer);
            port++;
            continue;
        }

        if (err != 0) {
            HazeLogError("Failed to start server: %s", uv_strerror(err));
            HazeServerFree(&mainServer);
            return 1;
        }

        HazeLogInfo("Haze Server started on %s:%d",
                    HazeServerAddress(mainServer),
                    HazeServerPort(mainServer));
        break; 
    }

    HazeServerRun(mainServer);

    HazeLogError("HazeServerRun returned");
    HazeLogInfo("Server process terminated.");
    HazeServerFree(&mainServer);

    return 0;
}
