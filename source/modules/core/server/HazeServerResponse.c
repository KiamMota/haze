#include "HazeServerResponse.h"
#include <stdlib.h>

HazeServerResponse* HazeServerResponseNew(void)
{
    HazeServerResponse* response = calloc(1, sizeof(HazeServerResponse));

    if (!response)
        return NULL;

    response->type = HAZE_RPC_RESPONSE;
    response->msgid = 0;
    response->error = NULL;
    response->result = NULL;

    return response;
}

void HazeServerResponseFree(HazeServerResponse** response)
{
    if (!response || !*response)
        return;

    free(*response);
    *response = NULL;
}

HazeServerRequest* HazeServerRequestNew(void)
{
    HazeServerRequest* request = calloc(1, sizeof(HazeServerRequest));

    if (!request)
        return NULL;

    request->type = HAZE_RPC_REQUEST;
    request->msgid = 0;
    request->method = NULL;
    request->parameters = NULL;

    return request;
}
