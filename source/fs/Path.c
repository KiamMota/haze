#include "Path.h"
#include "HazeMacros.h"
#include <linux/limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if WINDOWS
#include <windows.h>
#elif LINUX || MACOS || BSD
#include <sys/stat.h>
#endif

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
    if (!first)
        return NULL;

    va_list args;

    va_start(args, first);
    size_t total_len = strlen(first);
    
    const char *part = va_arg(args, const char *);
    while (part != NULL) {
        total_len += 1 + strlen(part); // +1 para a barra '/'
        part = va_arg(args, const char *);
    }
    va_end(args);

    char *combined = malloc(total_len + 1);
    if (!combined)
        return NULL;

    strcpy(combined, first);
    va_start(args, first);
    part = va_arg(args, const char *);
    while (part != NULL) {
      #if WINDOWS
        strcat(combined, "\\");      
      #else  
        strcat(combined, "/");
      #endif
        strcat(combined, part);
        part = va_arg(args, const char *);
    }
    va_end(args);

    Path *new_path = malloc(sizeof(Path));
    if (!new_path) {
        free(combined);
        return NULL;
    }

    new_path->path = combined;
    return new_path;
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

const char *PathStr(const Path *p) {
  if (PathIsNullOrEmpty(p)) return NULL;

  return p->path;
}

bool PathIsNullOrEmpty(const Path* path) {
    return path == NULL ||
           path->path == NULL ||
           path->path[0] == '\0';
}

bool PathExists(const Path *path) {
  if (PathIsNullOrEmpty(path))
    return false;

#if WINDOWS
  return GetFileAttributesA(path->path) != INVALID_FILE_ATTRIBUTES;
#else
  struct stat st;
  return stat(path->path, &st) == 0;
#endif
}
