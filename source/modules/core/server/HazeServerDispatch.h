/* HazeServerFuncDispatch.h */
#ifndef HAZE_SERVER_FUNC_DISPATCH_H
#define HAZE_SERVER_FUNC_DISPATCH_H

#include "HazeServerRequest.h"
#include "HazeServerResponse.h"

typedef HazeServerResponse *(*HazeRpcHandler)(HazeServerRequest *req);

typedef struct {
    const char    *method;
    HazeRpcHandler handler;
} HazeRpcEntry;

HazeRpcHandler HazeServerDispatchLookup(const char *method);
void          *HazeServerDispatch(const void *data, size_t len, size_t *out_len);

#endif
