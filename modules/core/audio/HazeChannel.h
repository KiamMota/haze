#ifndef HAZECHANNEL_DEC
#define HAZECHANNEL_DEC

typedef struct {
  float Volume;
  float Pan;
  bool Muted;
  bool Solo;
} HazeChannel;

HazeChannel* HazeChannelNew(void);
void HazeChannelFree(HazeChannel** hzc);

#endif
