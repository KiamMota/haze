#include "FnSession.h"
#include "Result.h"
#include "Session.h"
#include "proto/Object.h"
#include "proto/Request.h"
#include "proto/Response.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

static Result SessionInit(void) {
  if (SessionInstance == NULL) {
    SessionInstance = SessionNew(NULL);
    return ResultOk();
  }

  return ResultErr("The current session instance has already been created.");
}

Response *FnSessionCreate(Request *rq) {
  if (!rq)
    return ResponseCreateError(0, "Invalid request.");

  uint32_t msg_id = RequestMsgId(rq);

  if (RequestParamCount(rq) != 1)
    return ResponseCreateError(msg_id, "Expected 1 string parameter.");

  if (!RequestParamIsType(rq, OBJ_STR, 0))
    return ResponseCreateError(msg_id, "Expected a string parameter.");

  Object **p_ptr = RequestParamGet(rq, 0);
  if (!p_ptr || !*p_ptr)
    return ResponseCreateError(msg_id, "Invalid parameter object.");

  Object *p = *p_ptr;
  const char *session_name = p->value.str_value;

  if (!SessionInstance) {
    SessionInstance = SessionNew(session_name);
    return ResponseCreateStrResult(msg_id, "Session created successfully.");
  }

  return ResponseCreateStrResult(msg_id, "Session already started!");
}

Response *FnSessionGetName(Request *rq) {
  if (!rq)
    return ResponseCreateError(0, "Invalid request.");

  const char *name = SessionGetName(SessionInstance);
  return ResponseCreateStrResult(RequestMsgId(rq), name ? name : "");
}

Response *FnSessionGetWorktime(Request *rq) {
  if (!rq)
    return ResponseCreateError(0, "Invalid request.");

  time_t seconds = SessionGetWorkingTime(SessionInstance);

  int hours = (int)(seconds / 3600);
  int minutes = (int)((seconds % 3600) / 60);
  int secs = (int)(seconds % 60);

  char worktime[32];
  snprintf(worktime, sizeof(worktime), "%02d:%02d:%02d", hours, minutes, secs);

  return ResponseCreateStrResult(RequestMsgId(rq), worktime);
}
