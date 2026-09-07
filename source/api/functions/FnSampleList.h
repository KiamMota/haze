#ifndef FN_SAMPLE_LIST_H
#define FN_SAMPLE_LIST_H

#include "Result.h"
/// @module       sample_list
/// @acessor      import
/// @param        string sample_path
/// @description  Import a sample into the current session
/// @return       Result
/// @since        1.0.0
Result FnSampleListImportSample(const char *sample_path);

/// @module sample_list
/// @acessor import
/// @param string sample_path
/// @description Remove a sample of sample list
/// @return Result
/// @since 1.0.0

Result FnSampleListRemoveSample(const char *sample_name);

/// @module       sample
/// @acessor      play
/// @param        string sample_name
/// @description  Play a sample from the current session
/// @return       Result
/// @since        1.0.0
Result FnSamplePlay(const char *sample_name);

#endif
