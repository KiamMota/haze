/* HazeServerFuncDispatch.h */
#ifndef HAZE_SERVER_FUNC_DISPATCH_H
#define HAZE_SERVER_FUNC_DISPATCH_H

#include "proto/RawBuffer.h"
#include "proto/Request.h"
#include "proto/Response.h"

typedef Response *(*HazeRpcHandler)(Request *req);

typedef struct {
    const char    *method;
    HazeRpcHandler handler;
} HazeRpcEntry;

HazeRpcHandler HazeServerDispatchLookup(const char *method);
Response          *HazeServerDispatch(Request *b);

#endif
