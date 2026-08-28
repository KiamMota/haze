#include "HazeFuncPing.h"
#include "proto/Request.h"
#include "proto/Response.h"

Response *HazeFuncPing(Request *req)
{
    Response *res = ResponseNew();
    ResponseSetMsgId(res, req->msgid);
    ResponseSetResult(res, "pong", 4);
    return res;
}
