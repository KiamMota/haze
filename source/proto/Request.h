/* HazeServerRequest.h */
#ifndef HAZE_SERVER_REQUEST_H
#define HAZE_SERVER_REQUEST_H

#include "MessagePackRPC.h"
#include "RawBuffer.h"
#include "mpack/mpack-reader.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


typedef struct {
  mpack_reader_t reader; 
} RequestReader;

typedef struct {
  HazeServerRPCType type;
  uint32_t msgid;
  char *method;
  RawBuffer *parameters;
  // RequestReader reader;
} Request;

Request *RequestUnmarshal(RawBuffer *b);
RawBuffer* RequestMarshal(Request* r);

Request *ServerRequestNew(void);

void HazeServerRequestSetMethod(Request *request, const char *method);
bool RequestSetParameters(Request *request, RawBuffer *b);

static inline const RawBuffer* RequestParameters(const Request* r) {
  return r->parameters;
}
static inline uint32_t RequestMsgId(const Request *r) {
  return r->msgid;
}
static inline const char* RequestMethod(const Request *r) {
  return r->method;
}

void RequestFree(Request **request);

int RequestParamCount(Request* r);
bool RequestParamIsString(Request* r, uint32_t index);

#endif
