/* HazeServerRequest.h */
#ifndef HAZE_SERVER_REQUEST_H
#define HAZE_SERVER_REQUEST_H

#include "MessagePackRPC.h"
#include "RawBuffer.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
  HazeServerRPCType type;
  uint32_t msgid;

  char *method;
  RawBuffer *parameters;
} Request;

Request *RequestUnmarshal(RawBuffer *b);
RawBuffer* RequestMarshal(Request* r);

Request *HazeServerRequestNew(void);

void HazeServerRequestSetMethod(Request *request, const char *method);

bool RequestSetParameters(Request *request, RawBuffer *b);

RawBuffer *RequestGetRaw(Request *request);

void RequestFree(Request **request);

#endif
