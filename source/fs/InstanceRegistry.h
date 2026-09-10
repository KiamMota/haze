#ifndef RUNNING_H
#define RUNNING_H

#include "Result.h"
#include "fs/Paths.h"
#include "session/Session.h"
#include <stdint.h>
#include <stdio.h>


typedef struct {
  uint64_t pid;
  const char *session_name;
  unsigned short port;
  int created_at;
  const char* path;
  FILE *handle;
} InstanceReg;

extern InstanceReg* InstanceRegInstance;

InstanceReg *InstanceRegistryNew(Session* s, Paths* pt, unsigned short port);
void InstanceRegistryFree(InstanceReg **r);
Result InstanceRegistryRemove(InstanceReg *f);
unsigned short InstanceRegistryGetLastPort(void);

#endif
