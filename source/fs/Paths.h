#ifndef PATHSS_H
#define PATHSS_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
  const char *home;
  const char *config;
  const char *data;
  const char *cache;
  const char *projects;
  const char *samples;
  const char *themes;
  const char *plugins;
  const char *logs;
  const char *instances;
} Paths;

typedef enum {
  PATHS_HOME,
  PATHS_CONFIG,
  PATHS_DATA,
  PATHS_CACHE,
  PATHS_PROJECTS,
  PATHS_SAMPLES,
  PATHS_THEMES,
  PATHS_PLUGINS,
  PATHS_LOGS,
  PATHS_INSTANCES,
} PathsEnum;

extern Paths* PathsInstance;

Paths *PathsNew(void);
void PathsFree(Paths **pt);
bool PathsExists(Paths* p, PathsEnum path);
const char* PathsJoin(const char* path, ...);
const char* PathsGet(Paths* p, PathsEnum path);

#endif
