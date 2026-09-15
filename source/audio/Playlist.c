#include "Playlist.h"
#include <stdlib.h>

Playlist* PlaylistNew(void) {
  Playlist* pl = (Playlist*)malloc(sizeof(Playlist));
  pl->clips = (ClipInstance**)malloc(sizeof(ClipInstance*));
  pl->quantity = 0;
  return pl;
}

