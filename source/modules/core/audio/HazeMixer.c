#include "HazeMixer.h"
#include "HazeChannel.h"
#include <stdlib.h>

HazeMixer *HazeMixerNew(void)
{
    HazeMixer *m = calloc(1, sizeof(HazeMixer));
    if (!m) return NULL;
    m->capacity = 8;
    m->count    = 0;
    m->channels = calloc(m->capacity, sizeof(void *));
    if (!m->channels) { free(m); return NULL; }
    return m;
}

void HazeMixerFree(HazeMixer **m)
{
    if (!m || !*m) return;
    for (size_t i = 0; i < (*m)->count; i++) {
        HazeChannel *ch = (*m)->channels[i];
        HazeChannelFree(&ch);
    }
    free((*m)->channels);
    free(*m);
    *m = NULL;
}
