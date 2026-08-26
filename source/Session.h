#ifndef SESSION_H
#define SESSION_H

#include <stdbool.h>

typedef struct {
  char* session_name;
} Session;

Session* SessionNew(const char* SessionName);
void SessionFree(Session** s);
bool SessionSetName(const char* SessionName);
const char* SessionName(Session* s);

#endif
