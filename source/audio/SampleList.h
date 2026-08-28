#ifndef SAMPLE_LIST_H
#define SAMPLE_LIST_H

#include "audio/Sample.h"
#include <stdint.h>
typedef struct {
  Sample** samples;
  uint64_t len;
} SampleList;

SampleList* SampleListNew(void);
void SampleListFree(SampleList** list);
bool SampleListImport(Sample* s);
bool SampleListDeleteSampleById(SampleList* list, int id);
bool SampleListDeleteSampleByName(SampleList* list, const char* name);
uint64_t SampleListLen(SampleList* list);
Sample* SampleListGetSampleById(SampleList* list, int id);
Sample* SampleListGetSampleByName(SampleList* list, const char* name);

#endif
