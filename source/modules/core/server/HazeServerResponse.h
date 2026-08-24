#ifndef HAZE_SERVER_RESPONSE_H
#define HAZE_SERVER_RESPONSE_H

#include "HazeServerMRPC.h"

typedef struct {
    HazeServerRPCType type;
    uint32_t msgid;
    void* error;
    void* result;
} HazeServerResponse;

HazeServerResponse* HazeServerResponseNew(void);
void HazeServerResponseSetError(HazeServerResponse* s, const char* err);
void HazeServerResponseSetMsgId(HazeServerResponse* s, uint32_t msgid);
void HazeServerResponseSetResult(HazeServerResponse* s, void* data, unsigned int len);
void HazeServerResponseFree(HazeServerResponse** response);
#endif
