#ifndef SESSION_H
#define SESSION_H

#include <stdbool.h>
#include <time.h>

typedef struct {
  char* session_name;
  char* project_path;
  time_t working_time;
} Session;

extern Session* SessionInstance;


Session* SessionNew(const char* SessionName);
void SessionFree(Session** s);
bool SessionSetName(Session* s, const char* SessionName);
static inline const char* SessionName(const Session* s) {
  return s->session_name;
}

static inline const char* SessionWorkingTime(const Session *s) {
  return ctime(&s->working_time);
}

#endif
