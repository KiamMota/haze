/* HazeServerRequest.h */
#ifndef HAZE_SERVER_REQUEST_H
#define HAZE_SERVER_REQUEST_H

#include "HazeServerMRPC.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef struct {
    HazeServerRPCType type;
    uint32_t msgid;

    char *method;

    void *parameters;
    size_t parameters_len;

} HazeServerRequest;

HazeServerRequest* HazeServerRequestUnmarshal(const void* data, size_t len);

HazeServerRequest *HazeServerRequestNew(void);

void HazeServerRequestSetMethod(
    HazeServerRequest *request,
    const char *method
);

bool HazeServerRequestSetParameters(
    HazeServerRequest *request,
    const void *data,
    size_t len
);

void *HazeServerRequestGetRaw(
    HazeServerRequest *request,
    size_t *len
);

void HazeServerRequestFree(
    HazeServerRequest **request
);

#endif
