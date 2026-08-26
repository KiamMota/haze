#include "HazeFuncSessionCreate.h"
#include "HazeLog.h"
#include "proto/Request.h"
#include "proto/Response.h"
#include <stdio.h>

Response *HazeFuncSessionCreate(Request *rq)
{
    HazeLogDebug("HazeFuncSessionCreate started");

    if (RequestParamCount(rq) == 1) {
      printf("parametros corretos \n");
    }
    if (RequestParamIsString(rq)) {
        HazeLogDebug("Received string parameter");
    }

    HazeLogDebug("Creating response");
    return ResponseCreateStrResult(RequestMsgId(rq), "accepted.");
}
