#include "Paths.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#define PATHS_EXISTS(path) (_access(path, 0) == 0)
#else
#include <unistd.h>
#define PATHS_EXISTS(path) (access(path, F_OK) == 0)
#endif

Paths* PathsInstance;

static char *JoinPath(const char *a, const char *b) {
  size_t len = strlen(a) + strlen(b) + 2;
  char *path = malloc(len);

  if (!path)
    return NULL;

  snprintf(path, len, "%s/%s", a, b);

  return path;
}

Paths *PathsNew(void) {
  Paths *p = malloc(sizeof(Paths));

  if (!p)
    return NULL;

  const char *home = getenv("HOME");

  if (!home) {
    free(p);
    return NULL;
  }

  p->home = home;

  char *config = JoinPath(home, ".config/haze");
  char *data = JoinPath(home, ".local/share/haze");
  char *cache = JoinPath(home, ".cache/haze");

  if (!config || !data || !cache) {
    free(config);
    free(data);
    free(cache);
    free(p);
    return NULL;
  }

  p->config = config;
  p->data = data;
  p->cache = cache;

  p->projects = JoinPath(data, "projects");
  p->samples = JoinPath(data, "samples");
  p->themes = JoinPath(data, "themes");
  p->plugins = JoinPath(data, "plugins");
  p->logs = JoinPath(data, "logs");
  p->running = JoinPath(data, ".running");

  if (!p->projects || !p->samples || !p->themes || !p->plugins || !p->logs) {
    PathsFree(&p);
    return NULL;
  }

  return p;
}

void PathsFree(Paths **pt)
{
    if (!pt || !*pt)
        return;

    Paths *p = *pt;

    free((void *)p->config);
    free((void *)p->data);
    free((void *)p->cache);

    free((void *)p->projects);
    free((void *)p->samples);
    free((void *)p->themes);
    free((void *)p->plugins);
    free((void *)p->logs);
    free((void *)p->running);

    free(p);

    *pt = NULL;
}


bool PathsExists(Paths *p, PathsEnum path)
{
    if (!p)
        return false;

    const char *value = PathsGet(p, path);

    if (!value)
        return false;

    return PATHS_EXISTS(value);
}

const char* PathsGet(Paths* p, PathsEnum path)
{
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

        case PATHS_RUNNING:
            return p->running;

        default:
            return NULL;
    }
}
