#ifndef HAZE_SERVER_MESSAGE_H
#define HAZE_SERVER_MESSAGE_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    HAZE_RPC_REQUEST = 0,
    HAZE_RPC_RESPONSE = 1,
    HAZE_RPC_NOTIFICATION = 2
} HazeServerRPCType;

typedef struct {
    HazeServerRPCType type;
    uint32_t msgid;
    char* method;
    void* parameters;
} HazeServerRequest;

typedef struct {
    HazeServerRPCType type;
    uint32_t msgid;
    void* error;
    void* result;
} HazeServerResponse;

HazeServerResponse* HazeServerResponseNew(void);
void HazeServerResponseFree(HazeServerResponse** response);

HazeServerRequest* HazeServerRequestNew(void);
void HazeServerRequestFree(HazeServerRequest** request);

#endif
