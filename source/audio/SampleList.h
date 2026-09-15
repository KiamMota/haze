#ifndef SAMPLE_LIST_H
#define SAMPLE_LIST_H

#include "HazeMacros.h"
#include "audio/AudioEngine.h"
#include "audio/ResultAudio.h"
#include "audio/Sample.h"
#include <stdint.h>

EXPORT_CPP_BEGIN

typedef struct {
  Sample **samples;
  uint64_t len;
} SampleList;

SampleList *SampleListNew(void);
void SampleListFree(SampleList **list);

ResultAudio SampleListImportByFile(SampleList *list, const AudioEngine* eng, const char *path);
ResultAudio SampleListDeleteSampleByName(SampleList *list, const char *name);

uint64_t SampleListLen(SampleList *list);
Sample *SampleListGetSampleByName(SampleList *list, const char *name);
const char **SampleListStr(SampleList *list);

EXPORT_CPP_END

#endif
