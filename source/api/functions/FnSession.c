#include "FnSession.h"
#include "audio/Sample.h"
#include "audio/SampleList.h"
#include "session/Session.h"

const char *Ping(void) { return "pong"; }

Result FnSessionInit(void) {
  if (SessionInstance == NULL) {
    SessionInstance = SessionNew(NULL);
    return ResultOk();
  }

  return ResultErr("The current session instance has already been created.");
}

Result FnSessionCreate(const char *session_name) {
  if (!session_name) {
    return ResultErr("Invalid session name.");
  }

  if (!SessionInstance) {
    SessionInstance = SessionNew(session_name);
    return ResultOk();
  }

  return ResultErr("Session already started!");
}

const char *FnSessionGetName(void) {
  if (!SessionInstance) {
    return "";
  }

  const char *name = SessionGetName(SessionInstance);
  return name ? name : "";
}

time_t FnSessionGetWorkingTime(void) {
  if (!SessionInstance) {
    return 0;
  }

  return SessionGetWorkingTime(SessionInstance);
}

Result FnSampleImport(const char *path) {
  if (!SessionInstance || !SessionGetSampleList(SessionInstance)) {
    return ResultErr("No active project or sample list.");
  }

  return SampleListImportByFile(SessionGetSampleList(SessionInstance), path);
}


