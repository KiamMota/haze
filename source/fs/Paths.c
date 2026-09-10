#include "Paths.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#define PATHS_EXISTS(path) (_access(path, 0) == 0)
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#define PATH_SEP "\\"
#else
#include <unistd.h>
#define PATHS_EXISTS(path) (access(path, F_OK) == 0)
#include <sys/stat.h>
#define MKDIR(path) mkdir(path, 0755)
#define PATH_SEP "/"
#endif

Paths *PathsInstance = NULL;

static char *JoinPath(const char *a, const char *b) {
    size_t len = strlen(a) + strlen(PATH_SEP) + strlen(b) + 1;
    char *path = malloc(len);
    if (!path)
        return NULL;

    snprintf(path, len, "%s%s%s", a, PATH_SEP, b);
    return path;
}

static const char *GetHome(void) {
#ifdef _WIN32
    return getenv("USERPROFILE");
#else
    return getenv("HOME");
#endif
}

static int EnsureDir(const char *path) {
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

    char *base = JoinPath(home, ".haze");
    if (!base) {
        free((void *)p->home);
        free(p);
        return NULL;
    }
    EnsureDir(base);

    p->config = JoinPath(base, "config");
    p->data = JoinPath(base, "data");
    p->cache = JoinPath(base, "cache");
    p->projects = JoinPath(base, "projects");
    p->samples = JoinPath(base, "samples");
    p->themes = JoinPath(base, "themes");
    p->plugins = JoinPath(base, "plugins");
    p->logs = JoinPath(base, "logs");
    p->running = JoinPath(base, ".running");

    free(base);

    // Cria os diretórios principais necessários
    EnsureDir(p->config);
    EnsureDir(p->data);
    EnsureDir(p->cache);
    EnsureDir(p->projects);
    EnsureDir(p->samples);
    EnsureDir(p->themes);
    EnsureDir(p->plugins);
    EnsureDir(p->logs);

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
    free((void *)p->running);

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
        case PATHS_RUNNING:
            return p->running;
        default:
            return NULL;
    }
}
