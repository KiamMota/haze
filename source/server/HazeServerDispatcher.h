#ifndef HAZE_SERVER_DISPATCHER
#define HAZE_SERVER_DISPATCHER

#include "RawBuffer.h"
#include "api/proto/Request.h"
#include "api/proto/Response.h"
#include "api/Dispatcher.h"

RawBuffer *HazeServerAPIDispatcher(RawBuffer *buffer)
{
    Request *request = RequestUnmarshal(buffer);

    if (!request)
        return NULL;

    // Passamos APENAS a struct request, exatamente como a função espera!
    Response *response = DispatchRPCMessage(request);

    RequestFree(&request);

    if (!response)
        return NULL;

    RawBuffer *result = ResponseMarshal(response);

    ResponseFree(&response);

    return result;
}

#endif
