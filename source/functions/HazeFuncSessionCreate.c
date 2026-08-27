#include "HazeFuncSessionCreate.h"
#include "HazeLog.h"
#include "Session.h"
#include "functions/Result.h"
#include "proto/Request.h"
#include "proto/Response.h"

static Result SessionInit(void)
{
    if (SessionInstance == NULL) {
        SessionInstance = SessionNew(NULL);
        return ResultInit("Session created successfully.", true);
    }

    return ResultInit(
        "The current session instance has already been created.",
        false
    );
}

Response *HazeFuncSessionCreate(Request *rq)
{
    if (!rq) {
        return ResponseCreateStrResult(
            0,
            "Invalid request."
        );
    }

    uint32_t msg_id = RequestMsgId(rq);

    if (RequestParamCount(rq) != 1) {
        return ResponseCreateStrResult(
            msg_id,
            "Invalid parameter count. Expected 1 parameter."
        );
    }

    if (!RequestParamIsType(rq, PARAM_STR, 0)) {
        return ResponseCreateStrResult(
            msg_id,
            "Invalid parameter type. Expected a string."
        );
    }

    Result res = SessionInit();

    return ResponseCreateStrResult(
        msg_id,
        res.msg
    );
}
