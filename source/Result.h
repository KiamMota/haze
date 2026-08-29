/**

* @file Result.h
* @brief Simple result type for operations that may fail.
*
* Instead of returning only a boolean, functions return a Result
* containing:
*
* * success: Indicates whether the operation succeeded.
* * msg: Error message, valid only when success is false.
*
* Example:
*
* @code
* Result result = SampleInitFromFile(sample, path);
*
* if (!ResultIsOk(result)) {
* ```
  printf("Error: %s\n", result.msg);
  ```
* ```
  return;
  ```
* }
* @endcode
  */

#ifndef RESULT_H
#define RESULT_H

#include <stdbool.h>
#include <stdio.h>

/**

* @struct Result
* @brief Represents the result of an operation.
*
* @var Result::success
* Indicates whether the operation completed successfully.
*
* * true: The operation succeeded.
* * false: The operation failed.
*
* @var Result::msg
* Descriptive error message.
*
* This field is NULL when success is true.
* When success is false, it points to a string describing the error.
*
* The pointed string must have static or global lifetime and does not
* need to be freed.
  */
typedef struct {
  bool success;
  const char *msg;
} Result;

/**

* @brief Creates a successful Result.
*
* @return A Result with success set to true and msg set to NULL.
  */
static inline Result ResultOk(void) {
  return (Result){.success = true, .msg = NULL};
}

/**

* @brief Creates an error Result.
*
* @param msg Error message.
* ```
         Must not be NULL.
  ```
* ```
         The string must have static or global lifetime,
  ```
* ```
         usually provided as a string literal.
  ```
*
* @return A Result with success set to false and msg pointing to the
* ```
      provided error message.
  ```

*/
static inline Result ResultMsgE(const char *msg) {
  return (Result){.success = false, .msg = msg};
}

/**

* @brief Creates an error Result.
*
* @param msg Error message.
* ```
         Must not be NULL.
  ```
* ```
         The string must have static or global lifetime,
  ```
* ```
         usually provided as a string literal.
  ```
*
* @return A Result with success set to false and msg pointing to the
* ```
      provided error message.
  ```

*/
static inline Result ResultErr(const char *msg) {
  return (Result){.success = false, .msg = msg};
}

/**

* @brief Checks whether a Result represents a successful operation.
*
* @param res Result to check.
*
* @return true if the operation succeeded, false otherwise.
  */
static inline bool ResultIsOk(Result res) { return res.success; }

#endif /* RESULT_H */
