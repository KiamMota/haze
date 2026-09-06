#include "HazePaths.h"
#include "fs/Path.h"
#include <stdlib.h>

HazePaths* HazePathsInstance;
void HazePathsFree(HazePaths **paths) {
    if (!paths || !*paths)
        return;

    PathFree(&(*paths)->HazeRoot);
    PathFree(&(*paths)->HazeProjects);
    PathFree(&(*paths)->HazeSamples);
    PathFree(&(*paths)->HazeCache);
    PathFree(&(*paths)->HazeConfig);
    PathFree(&(*paths)->HazeCommunity);
    PathFree(&(*paths)->HazeLogs);

    free(*paths);
    *paths = NULL;
    
    if (HazePathsInstance == *paths) {
        HazePathsInstance = NULL;
    }
}
HazePaths* HazePathsLoad(void) {
    HazePaths* paths = malloc(sizeof(HazePaths));
    if (!paths)
        return NULL;

    Path* home = PathHome();
    if (!home) {
        free(paths);
        return NULL;
    }

    paths->HazeRoot = PathJoin(home->path, "Haze", NULL);
    PathFree(&home);

    if (!paths->HazeRoot) {
        free(paths);
        return NULL;
    }

    paths->HazeProjects  = PathJoin(paths->HazeRoot->path, "projects", NULL);
    paths->HazeSamples   = PathJoin(paths->HazeRoot->path, "samples", NULL);
    paths->HazeCache     = PathJoin(paths->HazeRoot->path, "cache", NULL);
    paths->HazeConfig    = PathJoin(paths->HazeRoot->path, "config", NULL);
    paths->HazeCommunity = PathJoin(paths->HazeRoot->path, "community", NULL);
    paths->HazeLogs      = PathJoin(paths->HazeRoot->path, "logs", NULL);


    if (!paths->HazeProjects ||
        !paths->HazeSamples ||
        !paths->HazeCache ||
        !paths->HazeConfig ||
        !paths->HazeCommunity ||
        !paths->HazeLogs) {
        HazePathsFree(&paths);
        return NULL;
    }

    HazePathsInstance = paths;
    return paths;
}
