#ifndef FUNCS_H
#define FUNCS_H

#include "Result.h"
#include <stdbool.h>
#include <time.h>

const char* Ping(void);

/// @module session
/// @acessor init
/// @description Initialize the Haze session
/// @return Result
Result haze_session_init(void);

/// @module session
/// @acessor create
/// @param string session_name
/// @description Create a new Haze session
/// @return Result
Result haze_session_create(const char *session_name);

/// @module session
/// @acessor get_name
/// @description Get the current session name
/// @return string
const char *haze_session_get_name(void);

/// @module session
/// @acessor get_working_time
/// @description Get the current session working time
/// @return int
time_t haze_session_get_working_time(void);

/// @module sample
/// @acessor import
/// @param string sample_path
/// @description Import a sample into the current session
/// @return Result
Result haze_sample_list_import_sample(const char* sample_path);

/// @module sample
/// @acessor play
/// @param string sample_name
/// @description Play a sample from the current session
/// @return Result
Result haze_sample_play(const char* sample_name);

#endif
