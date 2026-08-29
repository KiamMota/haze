#include "SampleList.h"
#include "HazeMacros.h"
#include "audio/Sample.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

  for (uint64_t i = 0; i < (*list)->len; i++)
    SampleFree(&(*list)->samples[i]);

  free((*list)->samples);
  free(*list);

  *list = NULL;
}

Result SampleListImportByFile(SampleList *list, const char *path) {
  if (!list || !path)
    return ResultErr("invalid argument");

  uint64_t index = list->len;

  Sample *sample = SampleNew();
  if (!sample)
    return ResultErr("failed to allocate sample");

  sample->id = index;

  Result result = SampleInitFromFile(sample, path);
  if (!ResultIsOk(result)) {
    SampleFree(&sample);
    return result;
  }

  const char *name = sample->sample_name;
  int occurrences = 0;

  for (uint64_t i = 0; i < index; i++) {
    Sample *existing = list->samples[i];

    if (existing && strcmp(existing->sample_name, name) == 0)
      occurrences++;
  }

  if (occurrences > 0) {
    size_t len = strlen(name);
    char *new_name = malloc(len + 32);

    if (!new_name) {
      SampleFree(&sample);
      return ResultErr("failed to allocate sample name");
    }

    snprintf(new_name, len + 32, "%s #%d", name, occurrences);

    free(sample->sample_name);
    sample->sample_name = new_name;
  }

  Sample **new_samples =
      realloc(list->samples, sizeof(*list->samples) * (list->len + 1));

  if (!new_samples) {
    SampleFree(&sample);
    return ResultErr("failed to resize sample list");
  }

  list->samples = new_samples;
  list->samples[index] = sample;
  list->len++;

  return ResultOk();
}

uint64_t SampleListLen(SampleList *list) {
  return list ? list->len : 0;
}

const char **SampleListStr(SampleList *list) {
  if (!list)
    return NULL;

  uint64_t len = SampleListLen(list);

  const char **strvec = malloc(sizeof(char *) * (len + 1));
  if (!strvec)
    return NULL;

  for (uint64_t i = 0; i < len; i++) {
    const char *name = list->samples[i]->sample_name;

    strvec[i] = strdup(name);

    if (!strvec[i]) {
      for (uint64_t j = 0; j < i; j++)
        free((void *)strvec[j]);

      free(strvec);
      return NULL;
    }
  }

  strvec[len] = NULL;

  return strvec;
}

Result SampleListDeleteSampleByName(SampleList *list, const char *name) {
  if (!list || !name)
    return ResultErr("invalid argument");

  for (uint64_t i = 0; i < list->len; i++) {
    Sample *sample = list->samples[i];

    if (!sample || strcmp(sample->sample_name, name) != 0)
      continue;

    SampleFree(&list->samples[i]);

    for (uint64_t j = i; j + 1 < list->len; j++)
      list->samples[j] = list->samples[j + 1];

    list->samples[list->len - 1] = NULL;
    list->len--;

    return ResultOk();
  }

  return ResultErr("sample not found");
}

Sample *SampleListGetSampleByName(SampleList *list, const char *name) {
  if (!list || !name)
    return NULL;

  for (uint64_t i = 0; i < list->len; i++) {
    Sample *sample = list->samples[i];

    if (sample && strcmp(sample->sample_name, name) == 0)
      return sample;
  }

  return NULL;
}
