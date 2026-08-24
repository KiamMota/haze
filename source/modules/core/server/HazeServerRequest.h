#ifndef HAZE_SERVER_REQUEST_H
#define HAZE_SERVER_REQUEST_H

#include "HazeServerMRPC.h"
#include <stdint.h>
typedef struct {
    HazeServerRPCType type;
    uint32_t msgid;
    char* method;
    void* parameters;
} HazeServerRequest;

HazeServerRequest* HazeServerRequestNew(void);
void HazeServerRequestFree(HazeServerRequest** request);

#endif
