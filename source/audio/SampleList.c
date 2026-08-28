#include "SampleList.h"
#include "HazeMacros.h"
#include "audio/Sample.h"
#include <stdint.h>
#include <stdlib.h>

SampleList *SampleListNew(void) {
    SampleList *sList = malloc(sizeof *sList);

    if (!sList)
        return NULL;

    sList->samples = NULL;
    sList->len = 0;

    return sList;
}

void SampleListFree(SampleList **list) {
    PTR_FREE_ASSERT(list);

    for (size_t i = 0; i < (*list)->len; i++) {
        HazeSampleFree(&(*list)->samples[i]);
    }

    free((*list)->samples);
    free(*list);
    *list = NULL;
}

uint64_t SampleListLen(SampleList *list) {
  return list->len;
}
