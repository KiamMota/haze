#ifndef HAZE_SERVER_MESSAGE_H
#define HAZE_SERVER_MESSAGE_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    HAZE_RPC_REQUEST = 0,
    HAZE_RPC_RESPONSE = 1,
    HAZE_RPC_NOTIFICATION = 2
} HazeServerRPCType;

#endif
