#ifndef SAMPLE_LIST_H
#define SAMPLE_LIST_H

#include "HazeMacros.h"
#include "Result.h"
#include "audio/Sample.h"
#include <stdint.h>

EXPORT_CPP_BEGIN

typedef struct {
  Sample **samples;
  uint64_t len;
} SampleList;

SampleList *SampleListNew(void);
void SampleListFree(SampleList **list);

Result SampleListImportByFile(SampleList *list, const char *path);
Result SampleListDeleteSampleByName(SampleList *list, const char *name);

uint64_t SampleListLen(SampleList *list);
Sample *SampleListGetSampleByName(SampleList *list, const char *name);
const char **SampleListStr(SampleList *list);

EXPORT_CPP_END

#endif
