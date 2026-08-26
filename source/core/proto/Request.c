#include "Request.h"
#include "HazeLog.h"
#include "core/proto/RawBuffer.h"
#include "mpack/mpack.h"

#include <stdlib.h>
#include <string.h>

Request *HazeServerRequestNew(void)
{
    Request *request =
        calloc(1, sizeof(Request));

    if (!request)
        return NULL;

    request->type = HAZE_RPC_REQUEST;

    return request;
}

Request *RequestUnmarshal(RawBuffer *b) {
  if (!b || !RawBufferLen(b)) return NULL;

  mpack_reader_t reader;
  mpack_reader_init_data(&reader, RawBufferData(b), RawBufferLen(b));
  Request *request = NULL;

  /* 1. Expect an array of 4 elements */
  mpack_tag_t tag = mpack_read_tag(&reader);
  if (mpack_reader_error(&reader) != mpack_ok) goto fail;
  
  if (mpack_tag_type(&tag) != mpack_type_array) {
      HazeLogError("Unmarshal failed: Expected ARRAY, got type %d", mpack_tag_type(&tag));
      goto fail;
  }
  if (mpack_tag_array_count(&tag) != 4) {
      HazeLogError("Unmarshal failed: Expected 4 items, got %d items", mpack_tag_array_count(&tag));
      goto fail;
  }

  request = HazeServerRequestNew();
  if (!request) goto fail;

  /* 2. Type */
  tag = mpack_read_tag(&reader);
  if (mpack_tag_type(&tag) != mpack_type_uint && mpack_tag_type(&tag) != mpack_type_int) {
      HazeLogError("Unmarshal failed [Type]: Expected INT/UINT, got type %d", mpack_tag_type(&tag));
      goto fail;
  }
  request->type = (HazeServerRPCType)mpack_tag_uint_value(&tag);
  if (request->type != HAZE_RPC_REQUEST) {
      HazeLogError("Unmarshal failed [Type]: Value is %d, expected %d (HAZE_RPC_REQUEST)", request->type, HAZE_RPC_REQUEST);
      goto fail;
  }

  /* 3. MsgID */
  tag = mpack_read_tag(&reader);
  if (mpack_tag_type(&tag) != mpack_type_uint && mpack_tag_type(&tag) != mpack_type_int) {
      HazeLogError("Unmarshal failed [MsgID]: Expected INT/UINT, got type %d", mpack_tag_type(&tag));
      goto fail;
  }
  request->msgid = (uint32_t)mpack_tag_uint_value(&tag);

  /* 4. Method */
  tag = mpack_read_tag(&reader);
  if (mpack_tag_type(&tag) != mpack_type_str) {
      HazeLogError("Unmarshal failed [Method]: Expected STR, got type %d (Hint: Python client might be sending BIN)", mpack_tag_type(&tag));
      goto fail;
  }
  
  uint32_t method_len = mpack_tag_str_length(&tag);
  request->method = malloc(method_len + 1);
  if (!request->method) goto fail;

  mpack_read_bytes(&reader, request->method, method_len);
  request->method[method_len] = '\0';
  mpack_done_str(&reader);

  if (mpack_reader_error(&reader) != mpack_ok) goto fail;

  /* 5. Params */
  mpack_discard(&reader);
  mpack_done_array(&reader);

  if (mpack_reader_error(&reader) == mpack_ok) {
    mpack_reader_destroy(&reader);
    return request;
  }

fail:
  if (mpack_reader_error(&reader) != mpack_ok) {
      HazeLogError("MPack Reader Error: %s", mpack_error_to_string(mpack_reader_error(&reader)));
  }
  mpack_reader_destroy(&reader);
  if (request) {
      RequestFree(&request);
  }
  return NULL;
}

void HazeServerRequestSetMethod(
    Request *request,
    const char *method
)
{
    if (!request)
        return;

    free(request->method);
    request->method = NULL;

    if (!method)
        return;

    request->method = malloc(strlen(method) + 1);

    if (!request->method)
        return;

    strcpy(request->method, method);
}

RawBuffer *RequestGetRaw(Request *request) {
    if (!request) return NULL;

    size_t method_len = request->method ? strlen(request->method) + 1 : 0;
    size_t params_len = request->parameters ? RawBufferLen(request->parameters) : 0;
    size_t total_size = sizeof(request->type) + sizeof(request->msgid) + method_len + params_len;

    void *raw_data = malloc(total_size);
    if (!raw_data) return NULL;

    uint8_t *ptr = (uint8_t *)raw_data;
    
    memcpy(ptr, &request->type, sizeof(request->type));
    ptr += sizeof(request->type);

    memcpy(ptr, &request->msgid, sizeof(request->msgid));
    ptr += sizeof(request->msgid);

    if (method_len > 0) {
        memcpy(ptr, request->method, method_len);
        ptr += method_len;
    }

    if (params_len > 0) {
        memcpy(ptr, RawBufferData(request->parameters), params_len);
    }

    return RawBufferNew(raw_data, total_size);
}


bool RequestSetParameters(Request *request, RawBuffer *b) {
    if (!request || !b || RawBufferLen(b) == 0) {
        return false;
    }

    if (!request->parameters) {
        request->parameters = RawBufferNew(RawBufferData(b), RawBufferLen(b));
        return request->parameters != NULL;
    }

    return RawBufferSetData(request->parameters, RawBufferData(b), RawBufferLen(b));
}



void RequestFree(
    Request **request
)
{
    if (!request || !*request)
        return;

    free((*request)->method);
    free((*request)->parameters);
    free(*request);

    *request = NULL;
}
