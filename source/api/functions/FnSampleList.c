#include "FnSampleList.h"

#include "audio/Sample.h"
#include "audio/SampleList.h"
#include "session/Session.h"

Result FnSampleListImportSample(const char *sample_path) {
  if (!sample_path || sample_path[0] == '\0') {
    return ResultErr("Invalid sample path.");
  }

  if (!SessionInstance) {
    return ResultErr("No active session.");
  }

  SampleList *list = (SampleList *)SessionGetSampleList(SessionInstance);

  if (!list) {
    return ResultErr("No sample list.");
  }

  return SampleListImportByFile(list, sample_path);
}

Result FnSampleListRemoveSample(const char *sample_name) {
  if (!sample_name || sample_name[0] == '\0') {
    return ResultErr("Invalid sample name.");
  }

  if (!SessionInstance) {
    return ResultErr("No active session.");
  }

  SampleList *list = (SampleList *)SessionGetSampleList(SessionInstance);

  if (!list) {
    return ResultErr("No sample list.");
  }

  return SampleListDeleteSampleByName(list, sample_name);
}

Result FnSamplePlay(const char *sample_name) {
  if (!sample_name || sample_name[0] == '\0') {
    return ResultErr("Invalid sample name.");
  }

  if (!SessionInstance) {
    return ResultErr("No active session.");
  }

  SampleList *list = (SampleList *)SessionGetSampleList(SessionInstance);

  if (!list) {
    return ResultErr("No sample list.");
  }

  Sample *sample = SampleListGetSampleByName(list, sample_name);

  if (!sample) {
    return ResultErr("Sample not found.");
  }

  return SamplePlay(sample);
}
