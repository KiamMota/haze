#ifndef HAZE_SERVER_RESPONSE_H
#define HAZE_SERVER_RESPONSE_H

#include "HazeServerMRPC.h"
#include <stddef.h>

typedef struct {
    HazeServerRPCType type;
    uint32_t msgid;

    void *error;
    size_t error_len;

    void *result;
    size_t result_len;
} HazeServerResponse;

HazeServerResponse* HazeServerResponseNew(void);
void* HazeServerResponseMarshal(HazeServerResponse* s, size_t* buf_len);
bool HazeServerResponseFree(HazeServerResponse** response);
bool HazeServerResponseSetError(HazeServerResponse* s, HazeServerRPCError err);
bool HazeServerResponseSetMsgId(HazeServerResponse* s, uint32_t msgid);
bool HazeServerResponseSetResult(HazeServerResponse* s, void* data, unsigned int len);
#endif
