#ifndef PROTO_RESPONSE_H
#define PROTO_RESPONSE_H

#include "MessagePackRPC.h"
#include "core/proto/RawBuffer.h"
#include <stddef.h>

typedef struct {
  HazeServerRPCType type;
  uint32_t msgid;

  RawBuffer *error;
  RawBuffer *result;
} Response;

Response *ResponseNew(void);
RawBuffer* ResponseMarshal(Response *s);
bool ResponseFree(Response **response);
bool ResponseSetError(Response *s, HazeServerRPCError err);
bool ResponseSetMsgId(Response *s, uint32_t msgid);
bool ResponseSetResult(Response *s,  void* data, size_t len);
#endif
