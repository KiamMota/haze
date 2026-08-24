#ifndef SERVER_DEC
#define SERVER_DEC
#include "modules/HazeMacros.h"
#include <uv.h>
#include <stdint.h>
#include <stdbool.h>

EXPORT_CPP_BEGIN

typedef struct {
    char        *addr;
    uint16_t     port;
    uv_loop_t   *loop;
    uv_tcp_t     tcp;
} HazeServer;

HazeServer    *HazeServerNew(const char *addr, uint16_t port);
void           HazeServerFree(HazeServer **s);
int            HazeServerStart(HazeServer *s);
void           HazeServerStop(HazeServer *s);
void           HazeServerRun(HazeServer *s);
uint16_t       HazeServerPort(HazeServer *s);
const char    *HazeServerAddress(HazeServer *s);

EXPORT_CPP_END
#endif
