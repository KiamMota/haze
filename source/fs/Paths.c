#include "Paths.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#define PATHS_EXISTS(path) (_access(path, 0) == 0)
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#define PATH_SEP '\\'
#else
#include <unistd.h>
#define PATHS_EXISTS(path) (access(path, F_OK) == 0)
#include <sys/stat.h>
#define MKDIR(path) mkdir(path, 0755)
#define PATH_SEP '/'
#endif

Paths *PathsInstance = NULL;

const char *PathsJoin(const char *path, ...) {
  char buffer[4096];
  buffer[0] = '\0';

  va_list args;
  va_start(args, path);

  const char *current = path;
  size_t length = 0;

  while (current != NULL) {
    if (length > 0 && buffer[length - 1] != PATH_SEP)
      buffer[length++] = PATH_SEP;

    size_t size = strlen(current);

    if (length + size >= sizeof(buffer) - 1)
      break;

    memcpy(buffer + length, current, size);
    length += size;
    buffer[length] = '\0';

    current = va_arg(args, const char *);
  }

  va_end(args);

  // Retorna uma cópia dinamica alocada na Heap (compatível com free())
  return strdup(buffer);
}

static const char *GetHome(void) {
#ifdef _WIN32
  return getenv("USERPROFILE");
#else
  return getenv("HOME");
#endif
}

static int EnsureDir(const char *path) {
  if (!path)
    return -1;
  if (MKDIR(path) == 0)
    return 0;
  if (errno == EEXIST)
    return 0;
  return -1;
}

Paths *PathsNew(void) {
  const char *home = GetHome();
  if (!home)
    return NULL;

  Paths *p = malloc(sizeof(Paths));
  if (!p)
    return NULL;

  p->home = strdup(home);
  if (!p->home) {
    free(p);
    return NULL;
  }

  const char *base = PathsJoin(home, ".haze", NULL);
  if (!base) {
    free((void *)p->home);
    free(p);
    return NULL;
  }
  EnsureDir(base);

  p->config = PathsJoin(base, "config", NULL);
  p->data = PathsJoin(base, "data", NULL);
  p->cache = PathsJoin(base, "cache", NULL);
  p->projects = PathsJoin(base, "projects", NULL);
  p->samples = PathsJoin(base, "samples", NULL);
  p->themes = PathsJoin(base, "themes", NULL);
  p->plugins = PathsJoin(base, "plugins", NULL);
  p->logs = PathsJoin(base, "logs", NULL);
  p->instances = PathsJoin(base, "instances", NULL);

  // Libera a string base intermediária
  free((void *)base);

  EnsureDir(p->config);
  EnsureDir(p->data);
  EnsureDir(p->cache);
  EnsureDir(p->projects);
  EnsureDir(p->samples);
  EnsureDir(p->themes);
  EnsureDir(p->plugins);
  EnsureDir(p->logs);
  EnsureDir(p->instances);

  return p;
}

void PathsFree(Paths **pt) {
  if (!pt || !*pt)
    return;

  Paths *p = *pt;

  free((void *)p->home);
  free((void *)p->config);
  free((void *)p->data);
  free((void *)p->cache);
  free((void *)p->projects);
  free((void *)p->samples);
  free((void *)p->themes);
  free((void *)p->plugins);
  free((void *)p->logs);
  free((void *)p->instances);

  free(p);
  *pt = NULL;
}

bool PathsExists(Paths *p, PathsEnum path) {
  if (!p)
    return false;

  const char *value = PathsGet(p, path);
  if (!value)
    return false;

  return PATHS_EXISTS(value);
}

const char *PathsGet(Paths *p, PathsEnum path) {
  if (!p)
    return NULL;

  switch (path) {
  case PATHS_HOME:
    return p->home;
  case PATHS_CONFIG:
    return p->config;
  case PATHS_DATA:
    return p->data;
  case PATHS_CACHE:
    return p->cache;
  case PATHS_PROJECTS:
    return p->projects;
  case PATHS_SAMPLES:
    return p->samples;
  case PATHS_THEMES:
    return p->themes;
  case PATHS_PLUGINS:
    return p->plugins;
  case PATHS_LOGS:
    return p->logs;
  case PATHS_INSTANCES:
    return p->instances;
  default:
    return NULL;
  }
}
