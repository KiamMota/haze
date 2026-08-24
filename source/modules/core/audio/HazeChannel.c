#include "HazeChannel.h"
#include <stdlib.h>

HazeChannel *HazeChannelNew(void)
{
    HazeChannel *ch = calloc(1, sizeof(HazeChannel));
    if (!ch) return NULL;
    ch->Volume = 1.0f;
    ch->Pan    = 0.0f;
    ch->Muted  = false;
    ch->Solo   = false;
    return ch;
}

void HazeChannelFree(HazeChannel **ch)
{
    if (!ch || !*ch) return;
    free(*ch);
    *ch = NULL;
}
