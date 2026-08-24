#include "HazeChannel.h"
#include "Macros.h"
#include <klib/kmalloc.h>

HazeChannel *HazeChannelNew(void) {
    HazeChannel *ch = KMALLOC(HazeChannel);
    if (!ch)
        return NULL;
    ch->Volume = 1.0f;
    ch->Pan    = 0.0f;
    ch->Muted  = false;
    ch->Solo   = false;
    return ch;
}

void HazeChannelFree(HazeChannel **ch) {
    PTR_FREE_ASSERT(ch);
    KFREE(*ch);
    *ch = NULL;
}

float HazeChannelVolume(HazeChannel *v){
  return v->Volume;
}
float HazeChannelPan(HazeChannel* v) {
  return v->Pan;
}
bool HazeChannelMuted(HazeChannel *m){
  return m->Muted;
}
bool HazeChannelSolo(HazeChannel *s) {
  return s->Solo;
}

