#include "HazeServerResponse.h"
#include "mpack/mpack.h"

#include "server/HazeServerMRPC.h"
#include <stdlib.h>

HazeServerResponse *HazeServerResponseNew(void) {
  HazeServerResponse *response = calloc(1, sizeof(HazeServerResponse));

  if (!response)
    return NULL;

  response->type = HAZE_RPC_RESPONSE;
  response->msgid = 0;
  response->error = NULL;
  response->result = NULL;

  return response;
}

void *HazeServerResponseMarshal(
    HazeServerResponse *s,
    size_t *out_len
) {
    if (!s || !out_len)
        return NULL;

    *out_len = 0;

    char *buffer = NULL;
    size_t size = 0;

    mpack_writer_t writer;
    mpack_writer_init_growable(&writer, &buffer, &size);

    mpack_start_array(&writer, 4);

    mpack_write_u8(&writer, HAZE_RPC_RESPONSE);
    mpack_write_u32(&writer, s->msgid);

    if (s->error && s->error_len > 0) {
        mpack_write_bytes(
            &writer,
            s->error,
            s->error_len
        );
    } else {
        mpack_write_nil(&writer);
    }

    if (s->result && s->result_len > 0) {
        mpack_write_bytes(
            &writer,
            s->result,
            s->result_len
        );
    } else {
        mpack_write_nil(&writer);
    }

    mpack_finish_array(&writer);

    if (mpack_writer_destroy(&writer) != mpack_ok) {
        free(buffer);
        return NULL;
    }

    *out_len = size;
    return buffer;
}

bool HazeServerResponseFree(HazeServerResponse **response) {
  if (!response || !*response)
    return false;

  free(*response);
  *response = NULL;
  return true;
}

bool HazeServerResponseSetError(HazeServerResponse *s, HazeServerRPCError err) {
    if (!s) return false;
    s->error = (void *)(uintptr_t)err;
    return true;
}

bool HazeServerResponseSetMsgId(HazeServerResponse *s, uint32_t msgid) {
  if (!s)
    return false;
  s->msgid = msgid;
  return true;
  
}
