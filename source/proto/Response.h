#ifndef PROTO_RESPONSE_H
#define PROTO_RESPONSE_H

#include "MessagePackRPC.h"
#include "RawBuffer.h"
#include <stddef.h>

typedef struct {
  HazeServerRPCType type;
  uint32_t msgid;

  RawBuffer *error;
  RawBuffer *result;
} Response;

Response *ResponseNew(void);
RawBuffer* ResponseMarshal(Response *s);
Response* ResponseUnmarshal(RawBuffer *b);
bool ResponseFree(Response **response);
bool ResponseSetError(Response *s, MpackRPCError err);
bool ResponseSetMsgId(Response *s, uint32_t msgid);
bool ResponseSetResult(Response *s,  void* data, size_t len);
#endif
