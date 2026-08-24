#ifndef HAZEMIXER_DEC
#define HAZEMIXER_DEC

#include <stddef.h>
#include <klib/klist.h>
typedef struct {
  KList* Channels;
} HazeMixer;

HazeMixer* HazeMixerNew(void);
void HazeMixerFree(HazeMixer** h);

#endif
