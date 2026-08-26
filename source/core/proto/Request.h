/* HazeServerRequest.h */
#ifndef HAZE_SERVER_REQUEST_H
#define HAZE_SERVER_REQUEST_H

#include "MessagePackRPC.h"
#include "core/proto/RawBuffer.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef struct {
    HazeServerRPCType type;
    uint32_t msgid;

    char *method;
    RawBuffer* parameters;
} Request;

Request* RequestUnmarshal(RawBuffer* b);

Request *HazeServerRequestNew(void);
void HazeServerRequestSetMethod(
    Request *request,
    const char *method
);

bool RequestSetParameters(
    Request *request, RawBuffer* b
);

RawBuffer *RequestGetRaw(
    Request *request
);

void RequestFree(
    Request **request
);

#endif
