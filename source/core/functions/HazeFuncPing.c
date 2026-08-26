#include "HazeFuncPing.h"
#include "core/proto/Request.h"
#include "core/proto/Response.h"

Response *HazeFuncPing(Request *req)
{
    Response *res = ResponseNew();
    ResponseSetMsgId(res, req->msgid);
    ResponseSetResult(res, "pong", 4);
    return res;
}
