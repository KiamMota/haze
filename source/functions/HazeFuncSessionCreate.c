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
    if (!rq)
        return ResponseCreateError(0, "Invalid request.");

    uint32_t msg_id = RequestMsgId(rq);

    if (RequestParamCount(rq) != 1)
        return ResponseCreateError(msg_id, "Expected 1 string parameter.");

    if (!RequestParamIsType(rq, PARAM_STR, 0))
        return ResponseCreateError(msg_id, "Expected a string parameter.");

    RequestParam *p = RequestParamGet(rq, 0);
    const char *name = p ? p->value.str_value : NULL;  /* ou ignore se não usar */

    if (SessionInstance != NULL)
        return ResponseCreateError(msg_id, "Session already exists.");
        /* ou ResponseCreateStrResult se quiser idempotente */

    SessionInstance = SessionNew(name);  /* ou SessionNew(NULL) se name não importa */
    if (!SessionInstance)
        return ResponseCreateError(msg_id, "Failed to create session.");

    return ResponseCreateStrResult(msg_id, "Session created successfully.");
}
