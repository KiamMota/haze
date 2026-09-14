#include "FnSession.h"
#include "audio/Sample.h"
#include "session/Session.h"

const char *Ping(void) { return "pong"; }

Result FnSessionInit(Session* s) {
  if (s == NULL) {
    s = SessionNew(NULL);
    return ResultOk();
  }

  return ResultErr("The current session instance has already been created.");
}

Result FnSessionCreate(Session* s, const char *session_name) {
  if (!session_name) {
    return ResultErr("Invalid session name.");
  }

  if (!s) {
    s = SessionNew(session_name);
    return ResultOk();
  }

  return ResultErr("Session already started!");
}

const char *FnSessionGetName(const Session* s) {
  if (!s) {
    return "";
  }

  const char *name = SessionGetName(s);
  return name ? name : "";
}

time_t FnSessionGetWorkingTime(const Session* s) {
  if (!s) {
    return 0;
  }

  return SessionGetWorkingTime(s);
}


