#ifndef HAZECHANNEL_DEC
#define HAZECHANNEL_DEC

#include <klib/ktypes.h>

typedef struct {
  float Volume;
  float Pan;
  bool Muted;
  bool Solo;
} HazeChannel;

HazeChannel* HazeChannelNew(void);
void HazeChannelFree(HazeChannel** hzc);
float HazeChannelVolume(HazeChannel* v);
float HazeChannelPan(HazeChannel* v);
bool HazeChannelMuted(HazeChannel* m);
bool HazeChannelSolo(HazeChannel* s);

#endif
