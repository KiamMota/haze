#ifndef HAZEMIXER_DEC
#define HAZEMIXER_DEC
#include <stddef.h>

typedef struct {
    void   **channels;
    size_t   count;
    size_t   capacity;
} HazeMixer;

HazeMixer *HazeMixerNew(void);
void       HazeMixerFree(HazeMixer **m);
#endif
