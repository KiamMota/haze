#ifndef SUCCESS_H
#define SUCCESS_H

#include <stdbool.h>
typedef struct {
  const char *msg;
  bool ok;
} Result;


static inline Result ResultInit(const char* msg, bool ok) {
    Result res = {.msg = msg, .ok = ok};
    return res;
}



#endif
