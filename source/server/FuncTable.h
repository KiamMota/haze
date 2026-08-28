#ifndef FUNC_TABLE_H
#define FUNC_TABLE_H

#include "HazeServerDispatch.h"
#include "functions/HazeFuncPing.h"
#include "functions/HazeFuncSessionCreate.h"

const HazeRpcEntry haze_rpc_table[] = 
{
  {"ping", HazeFuncPing},
  {"session/create", HazeFuncSessionCreate},
  {NULL, NULL}
};

#endif
