#include "HazeChannel.h"
#include <glib.h>

HazeChannel *HazeChannelNew(void)
{
    HazeChannel *ch = g_new0(HazeChannel, 1);

    if (!ch)
        return NULL;

    ch->Volume = 1.0f;
    ch->Pan    = 0.0f;
    ch->Muted  = false;
    ch->Solo   = false;

    return ch;
}

void HazeChannelFree(HazeChannel **ch)
{
    if (!ch || !*ch)
        return;

    g_free(*ch);
    *ch = NULL;
}

float HazeChannelVolume(HazeChannel *v)
{
    return v->Volume;
}

float HazeChannelPan(HazeChannel *v)
{
    return v->Pan;
}

bool HazeChannelMuted(HazeChannel *m)
{
    return m->Muted;
}

bool HazeChannelSolo(HazeChannel *s)
{
    return s->Solo;
}
