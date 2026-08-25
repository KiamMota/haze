#include "HazeFuncPing.h"

HazeServerResponse *HazeFuncPing(HazeServerRequest *req)
{
    HazeServerResponse *res = HazeServerResponseNew();
    HazeServerResponseSetMsgId(res, req->msgid);
    HazeServerResponseSetResult(res, "pong", 4);
    return res;
}
