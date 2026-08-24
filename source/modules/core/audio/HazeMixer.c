#include "HazeMixer.h"
#include "HazeChannel.h"

HazeMixer *HazeMixerNew(void)
{
    HazeMixer *m = g_new0(HazeMixer, 1);

    if (!m)
        return NULL;

    m->channels = g_ptr_array_new();

    return m;
}

void HazeMixerFree(HazeMixer **m)
{
    if (!m || !*m)
        return;

    for (guint i = 0; i < (*m)->channels->len; i++) {
        HazeChannel *channel =
            g_ptr_array_index((*m)->channels, i);

        HazeChannelFree(&channel);
    }

    g_ptr_array_unref((*m)->channels);

    g_free(*m);
    *m = NULL;
}
