#include "FnSampleList.h"

#include "audio/AudioEngine.h"
#include "audio/Sample.h"
#include "audio/SampleList.h"

Result FnSampleListImportSample(SampleList* s, const AudioEngine* eng, const char *sample_path) {
  if (!sample_path || sample_path[0] == '\0') {
    return ResultErr("Invalid sample path.");
  }
  if (!s) {
    return ResultErr("No sample list.");
  }

  return SampleListImportByFile(s, eng, sample_path);
}

Result FnSampleListRemoveSample(SampleList* s, const char *sample_name) {
  if (!sample_name || sample_name[0] == '\0') {
    return ResultErr("Invalid sample name.");
  }

  if (!s) {
    return ResultErr("No sample list.");
  }

  return SampleListDeleteSampleByName(s, sample_name);
}

Result FnSamplePlay(SampleList* s, const char *sample_name) {
  if (!sample_name || sample_name[0] == '\0') {
    return ResultErr("Invalid sample name.");
  }


  if (!s) {
    return ResultErr("No sample list.");
  }

  Sample *sample = SampleListGetSampleByName(s, sample_name);

  if (!sample) {
    return ResultErr("Sample not found.");
  }

  return SamplePlay(sample);
}
