#ifndef SESSION_H
#define SESSION_H

#include "audio/SampleList.h"
#include <stdbool.h>
#include <time.h>

typedef struct {
  char* session_name;
  char* project_path;
  time_t created_at;
  time_t working_time;
  FILE* log_file;
  SampleList *SampleList;
} Session;

extern Session* SessionInstance;


Session* SessionNew(const char* SessionName);
void SessionFree(Session** s);
bool SessionSetName(Session* s, const char* SessionName);
static inline const char* SessionGetName(const Session* s) {
  return s->session_name;
}
static inline time_t SessionGetWorkingTime(const Session *s) {
    return time(NULL) - s->created_at;
}
static inline time_t SessionGetCreatedAt(const Session* s) {
  return s->created_at;
}

const SampleList* SessionGetSampleList(Session *s);



#endif
