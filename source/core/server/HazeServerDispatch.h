/* HazeServerFuncDispatch.h */
#ifndef HAZE_SERVER_FUNC_DISPATCH_H
#define HAZE_SERVER_FUNC_DISPATCH_H

#include "core/proto/RawBuffer.h"
#include "core/proto/Request.h"
#include "core/proto/Response.h"

typedef Response *(*HazeRpcHandler)(Request *req);

typedef struct {
    const char    *method;
    HazeRpcHandler handler;
} HazeRpcEntry;

HazeRpcHandler HazeServerDispatchLookup(const char *method);
void          *HazeServerDispatch(RawBuffer *b);

#endif
