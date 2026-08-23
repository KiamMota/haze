#ifndef HAZETRACK_DEC
#define HAZETRACK_DEC

#include "HazeSample.h"
#include <klib/klinklist.h>

typedef struct {
  KLinkedList* Sample;
  float Volume;
  float Pan;
  bool Muted;
  bool Solo;
} HazeTrack;

HazeTrack* HazeTrackNew(void);
void HazeTrackFree(HazeTrack** hzt);

#endif
