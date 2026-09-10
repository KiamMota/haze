#ifndef RUNNING_H
#define RUNNING_H

#include "Result.h"
#include <stdint.h>

typedef enum {
  RN_STATUS_UP,
  RN_STATUS_DOWN,
  RN_STATUS_STOPPED,
} Status;

typedef struct {
  uint64_t pid;
  const char* session_name;
  unsigned short port;
  Status status;
  int created_at;
} Running;

Running* RunningNew(const char* session_name, unsigned short port, Status st);
Running* RunningFree(Running** r);
Result RunningWriteFile(Running* r);
unsigned short RunningGetLastPort(void);

#endif
