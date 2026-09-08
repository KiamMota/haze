#ifndef FUNCS_H
#define FUNCS_H

#include "Result.h"
#include <stdbool.h>
#include <time.h>

const char* Ping(void);

/// @module       session
/// @acessor      init
/// @param        not
/// @description  Initialize the Haze session
/// @return       Result
/// @since        1.0.0
Result FnSessionInit(void);

/// @module       session
/// @acessor      create
/// @param        string session_name
/// @description  Create a new Haze session
/// @return       Result
/// @since        1.0.0
Result FnSessionCreate(const char *session_name);

/// @module       session
/// @acessor      get_name
/// @description  Get the current session name
/// @return       string
/// @since        1.0.0
const char *FnSessionGetName(void);

/// @module       session
/// @acessor      get_working_time
/// @description  Get the current session working time
/// @return       int
/// @since        1.0.0
time_t FnSessionGetWorkingTime(void);



#endif
