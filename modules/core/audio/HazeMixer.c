#include "HazeMixer.h"
#include "HazeChannel.h"
#include <klib/klinklist.h>
#include <klib/klist.h>
#include <klib/kmalloc.h>

HazeMixer* HazeMixerNew(void) {
  HazeMixer* hz = KMALLOC(HazeMixer);
  hz->Channels = klist_new();
  return hz;
} 

void HazeMixerFree(HazeMixer **m) {
    if (!m || !*m)
        return;

    for (usize i = 0; i < klist_len((*m)->Channels); i++) {
        HazeChannel *ch = klist_get((*m)->Channels, i);
        HazeChannelFree(&ch);
    }

    klist_free(&(*m)->Channels);
    KFREE(*m);
    *m = NULL;
}
