#ifndef FUNC_TABLE_H
#define FUNC_TABLE_H

#include "HazeServerDispatch.h"
#include "functions/FnSession.h"
#include "functions/HazeFuncPing.h"
#include "functions/FnSampleList.h"

const HazeRpcEntry haze_rpc_table[] = {
    {"ping", HazeFuncPing},
    {"session/create", FnSessionCreate},
    {"session/get_name", FnSessionGetName},
    {"session/get_worktime", FnSessionGetWorktime},
    {"samplelist/import", FnSampleListImportSample},
    {NULL, NULL}
};

#endif
