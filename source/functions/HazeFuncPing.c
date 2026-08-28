#include "HazeFuncPing.h"
#include "proto/Request.h"
#include "proto/Response.h"

Response *HazeFuncPing(Request *req)
{
    return ResponseCreateStrResult(RequestMsgId(req), "pong!");
}
