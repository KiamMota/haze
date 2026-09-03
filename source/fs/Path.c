#include "Path.h"
#include "HazeMacros.h"
#include <linux/limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Path *PathNew(const char *path) {
  Path *p = malloc(sizeof(Path));

  if (p == NULL)
    return NULL;

  p->path = path ? strdup(path) : NULL;

  return p;
}

void PathFree(Path **path) {
  PTR_FREE_ASSERT(path);
  free((*path)->path);
  free(*path);
  *path = NULL;
}

Path *PathBuild(const char *first, ...) {
  va_list args;

  va_start(args, first);

  const char *part = first;

  while (part != NULL) {
    printf("%s\n", part);
    part = va_arg(args, const char *);
  }

  va_end(args);

  return NULL;
}

Path *PathAbs(const char *path) {
  char resolved[PATH_MAX];

  if (realpath(path, resolved) == NULL)
    return NULL;

  return PathNew(resolved);
}

Path *PathHome(void) {
  Path *path = PathNew(NULL);

  if (path == NULL)
    return NULL;

#if WINDOWS
  path->path = strdup(getenv("USERPROFILE"));
#elif LINUX || MACOS || BSD
  path->path = strdup(getenv("HOME"));
#else
  free(path);
  return NULL;
#endif

  if (path->path == NULL) {
    free(path);
    return NULL;
  }

  return path;
}

const char *PathHomeStr(void) {
#if WINDOWS
  return strdup(getenv("USERPROFILE"));
#elif LINUX || MACOS || BSD
  return strdup(getenv("HOME"));
#else
  free(path);
  return NULL;
#endif
}
bool PathIsNullOrEmpty(const Path *path) {
  if (path == NULL || path->path == NULL)
    return true;
  return false;
}

bool PathExists(const Path *path) {
  if (PathIsNullOrEmpty(path))
    return false;

#ifdef WINDOWS
  return GetFileAttributesA(path->path) != INVALID_FILE_ATTRIBUTES;
#else
  struct stat st;
  return stat(path->path, &st) == 0;
#endif
}
