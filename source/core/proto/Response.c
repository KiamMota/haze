#include "Response.h"
#include "core/proto/RawBuffer.h"
#include "mpack/mpack.h"

#include <stddef.h>
#include <stdlib.h>

Response *ResponseNew(void) {
  Response *response = calloc(1, sizeof(Response));

  if (!response)
    return NULL;

  response->type = HAZE_RPC_RESPONSE;
  response->msgid = 0;
  response->error = RawBufferNew(NULL, 0);
  response->result = RawBufferNew(NULL, 0);

  return response;
}
bool ResponseSetResult(Response *s, void *data, size_t len) {
  if (!s)
    return false;

  return RawBufferSetData(s->result, data, len);
}

RawBuffer *ResponseMarshal(Response *s) {
  if (!s)
    return NULL;

  RawBuffer *buffer = RawBufferNew(NULL, 0);

  char *data = RawBufferData(buffer);
  size_t size = RawBufferLen(buffer);

  mpack_writer_t writer;
  mpack_writer_init_growable(&writer, &data, &size);

  mpack_start_array(&writer, 4);

  mpack_write_u8(&writer, HAZE_RPC_RESPONSE);
  mpack_write_u32(&writer, s->msgid);

  if (RawBufferLen(s->error)) {
    mpack_write_bytes(&writer, RawBufferData(s->error), RawBufferLen(s->error));
  } else {
    mpack_write_nil(&writer);
  }

  if (RawBufferLen(s->result)) {
    mpack_write_bytes(&writer, RawBufferData(s->result),
                      RawBufferLen(s->result));
  } else {
    mpack_write_nil(&writer);
  }

  mpack_finish_array(&writer);

  if (mpack_writer_destroy(&writer) != mpack_ok) {
    RawBufferFree(&buffer);
    return NULL;
  }
  RawBufferSetData(buffer, data, size);

  return buffer;
}

bool ResponseFree(Response **response) {
  if (!response || !*response)
    return false;

  free(*response);
  *response = NULL;
  return true;
}

bool ResponseSetError(Response *s, HazeServerRPCError err) {
  if (!s)
    return false;
  s->error = (void *)(uintptr_t)err;
  return true;
}

bool ResponseSetMsgId(Response *s, uint32_t msgid) {
  if (!s)
    return false;
  s->msgid = msgid;
  return true;
}
