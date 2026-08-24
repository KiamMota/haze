#include "HazeServerRequest.h"
#include <stdlib.h>

void HazeServerRequestFree(HazeServerRequest** request)
{
    if (!request || !*request)
        return;

    free((*request)->method);
    free(*request);

    *request = NULL;
}
