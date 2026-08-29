#include "FnSession.h"
#include "Result.h"
#include "Session.h"
#include "proto/Request.h"
#include "proto/Response.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static Result SessionInit(void) {
  if (SessionInstance == NULL) {
    SessionInstance = SessionNew(NULL);
    return ResultInit("Session created successfully.", true);
  }

  return ResultInit("The current session instance has already been created.",
                    false);
}

Response *FnSessionCreate(Request *rq) {
  if (!rq)
    return ResponseCreateError(0, "Invalid request.");

  uint32_t msg_id = RequestMsgId(rq);

  if (RequestParamCount(rq) != 1)
    return ResponseCreateError(msg_id, "Expected 1 string parameter.");

  if (!RequestParamIsType(rq, PARAM_STR, 0))
    return ResponseCreateError(msg_id, "Expected a string parameter.");

  RequestParam *p = RequestParamGet(rq, 0);
  if (!SessionInstance) {
    SessionInstance = SessionNew(p->value.str_value);
    return ResponseCreateStrResult(msg_id, "Session created successfully.");
  }
  return ResponseCreateStrResult(msg_id, "Session already started!");
}

Response *FnSessionGetName(Request *rq) {
  const char *name = SessionGetName(SessionInstance);
  return ResponseCreateStrResult(RequestMsgId(rq), name);
}

Response *FnSessionGetWorktime(Request *rq) {
  time_t seconds = SessionGetWorkingTime(SessionInstance);

  int hours = seconds / 3600;
  int minutes = (seconds % 3600) / 60;
  int secs = seconds % 60;

  char worktime[32];

  snprintf(worktime, sizeof(worktime), "%02d:%02d:%02d", hours, minutes, secs);

  return ResponseCreateStrResult(RequestMsgId(rq), worktime);
}
