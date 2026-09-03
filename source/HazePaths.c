#include "HazePaths.h"
#include "HazeLog.h"
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
        HazeLogError("PathHome() falhou.");
        free(paths);
        return NULL;
    }

    paths->HazeRoot = PathBuild(home->path, "Haze", NULL);
    PathFree(&home);

    if (!paths->HazeRoot) {
        HazeLogError("HazeRoot falhou ao construir.");
        free(paths);
        return NULL;
    }

    paths->HazeProjects  = PathBuild(paths->HazeRoot->path, "projects", NULL);
    paths->HazeSamples   = PathBuild(paths->HazeRoot->path, "samples", NULL);
    paths->HazeCache     = PathBuild(paths->HazeRoot->path, "cache", NULL);
    paths->HazeConfig    = PathBuild(paths->HazeRoot->path, "config", NULL);
    paths->HazeCommunity = PathBuild(paths->HazeRoot->path, "community", NULL);
    paths->HazeLogs      = PathBuild(paths->HazeRoot->path, "logs", NULL);

    if (!paths->HazeProjects)  HazeLogError("HazeProjects falhou.");
    if (!paths->HazeSamples)   HazeLogError("HazeSamples falhou.");
    if (!paths->HazeCache)     HazeLogError("HazeCache falhou.");
    if (!paths->HazeConfig)    HazeLogError("HazeConfig falhou.");
    if (!paths->HazeCommunity) HazeLogError("HazeCommunity falhou.");
    if (!paths->HazeLogs)      HazeLogError("HazeLogs falhou.");

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
