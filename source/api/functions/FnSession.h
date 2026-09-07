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
Result haze_session_init(void);

/// @module       session
/// @acessor      create
/// @param        string session_name
/// @description  Create a new Haze session
/// @return       Result
/// @since        1.0.0
Result haze_session_create(const char *session_name);

/// @module       session
/// @acessor      get_name
/// @description  Get the current session name
/// @return       string
/// @since        1.0.0
const char *haze_session_get_name(void);

/// @module       session
/// @acessor      get_working_time
/// @description  Get the current session working time
/// @return       int
/// @since        1.0.0
time_t haze_session_get_working_time(void);



#endif
