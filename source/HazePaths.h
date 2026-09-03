#ifndef HAZE_PATHS_H
#define HAZE_PATHS_H

#include "fs/Path.h"
#include <asm-generic/param.h>
typedef struct {
  Path* HazeRoot;
  Path* HazeProjects;
  Path* HazeSamples;
  Path* HazeCache;
  Path* HazeConfig;
  Path* HazeCommunity;
  Path* HazeLogs;

} HazePaths;

extern HazePaths* HazePathsInstance;
HazePaths* HazePathsLoad(void);
const Path* HazePathRoot();
void HazePathsFree(HazePaths** hz);

#endif
