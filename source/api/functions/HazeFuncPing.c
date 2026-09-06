#include "api/functions/HazeFuncPing.h"
#include "api/proto/Request.h"
#include "api/proto/Response.h"

Response *HazeFuncPing(Request *req)
{
    return ResponseCreateStrResult(RequestMsgId(req), "pong!");
}
