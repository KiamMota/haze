#include "Funcs.h"
#include "audio/Sample.h"
#include "audio/SampleList.h"
#include "session/Session.h"

const char *Ping(void) { return "pong"; }

Result haze_session_init(void) {
  if (SessionInstance == NULL) {
    SessionInstance = SessionNew(NULL);
    return ResultOk();
  }

  return ResultErr("The current session instance has already been created.");
}

Result haze_session_create(const char *session_name) {
  if (!session_name) {
    return ResultErr("Invalid session name.");
  }

  if (!SessionInstance) {
    SessionInstance = SessionNew(session_name);
    return ResultOk();
  }

  return ResultErr("Session already started!");
}

const char *haze_session_get_name(void) {
  if (!SessionInstance) {
    return "";
  }

  const char *name = SessionGetName(SessionInstance);
  return name ? name : "";
}

time_t haze_session_get_working_time(void) {
  if (!SessionInstance) {
    return 0;
  }

  return SessionGetWorkingTime(SessionInstance);
}

Result haze_sample_import(const char *path) {
  if (!SessionInstance || !SessionGetSampleList(SessionInstance)) {
    return ResultErr("No active project or sample list.");
  }

  return SampleListImportByFile(SessionGetSampleList(SessionInstance), path);
}

Result haze_sample_play(const char *sample_name) {
  if (!sample_name || sample_name[0] == '\0') {
    return ResultErr("Invalid sample name.");
  }

  if (!SessionInstance || !SessionGetSampleList(SessionInstance)) {
    return ResultErr("No active session or sample list.");
  }

  Sample *sample_getted = SampleListGetSampleByName(
      SessionGetSampleList(SessionInstance), sample_name);
  if (!sample_getted) {
    return ResultErr("Sample not found.");
  }

  return SamplePlay(sample_getted);
}
