#ifndef PATH_H
#define PATH_H

#include <stdbool.h>
typedef struct {
  char* path;
} Path;

Path* PathNew(const char* path);
void PathFree(Path** path);
Path* PathBuild(const char* first, ...);
Path* PathAbs(const char* path);
Path* PathHome(void);
const char* PathHomeStr(void);
bool PathIsNullOrEmpty(const Path* Path);
bool PathExists(const Path* path);
bool PathIsFile(const Path* path);
bool PathIsDir(const Path* path);

#endif
