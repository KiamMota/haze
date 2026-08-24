#ifndef HAZECHANNEL_DEC
#define HAZECHANNEL_DEC

<<<<<<< HEAD:source/modules/core/audio/HazeChannel.h
=======
#include <klib/ktypes.h>
>>>>>>> 49f8b9abe702b1d87cce11b955185d839ac0fe9d:modules/core/audio/HazeChannel.h

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
