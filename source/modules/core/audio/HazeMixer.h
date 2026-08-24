#ifndef HAZEMIXER_DEC
#define HAZEMIXER_DEC

#include <glib.h>

typedef struct {
    GPtrArray *channels;
} HazeMixer;

HazeMixer *HazeMixerNew(void);
void HazeMixerFree(HazeMixer **m);

#endif
