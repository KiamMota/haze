#include "Request.h"
#include "HazeLog.h"
#include "mpack/mpack-common.h"
#include "mpack/mpack-expect.h"
#include "mpack/mpack-reader.h"
#include "mpack/mpack.h"
#include "proto/RawBuffer.h"

#include <stdlib.h>
#include <string.h>

Request *ServerRequestNew(void) {
  Request *request = calloc(1, sizeof(Request));

  if (!request)
    return NULL;

  request->type = HAZE_RPC_REQUEST;

  return request;
}

RawBuffer *RequestMarshal(Request *request) {
  if (!request) {
    return NULL;
  }

  char *data = NULL;
  size_t size = 0;

  mpack_writer_t writer;

  HazeLogError("ResponseMarshal failed: %s",
               mpack_error_to_string(mpack_writer_error(&writer)));
  mpack_writer_init_growable(&writer, &data, &size);

  mpack_start_array(&writer, 4);

  mpack_write_uint(&writer, (uint64_t)request->type);
  mpack_write_uint(&writer, (uint64_t)request->msgid);

  if (request->method) {
    mpack_write_str(&writer, request->method, strlen(request->method));
  } else {
    mpack_write_nil(&writer);
  }

  /* Params */
  mpack_write_nil(&writer);

  mpack_finish_array(&writer);

  mpack_error_t error = mpack_writer_destroy(&writer);

  if (error != mpack_ok) {
    HazeLogError("Marshal failed: %s", mpack_error_to_string(error));

    if (data) {
      MPACK_FREE(data);
    }

    return NULL;
  }

  RawBuffer *buffer = RawBufferNew(data, size);

  if (!buffer) {
    MPACK_FREE(data);
    return NULL;
  }

  return buffer;
}

Request *RequestUnmarshal(RawBuffer *b) {
  if (!b || !RawBufferLen(b))
    return NULL;
  const unsigned char *data = RawBufferData(b);

  HazeLogDebug("RequestUnmarshal bytes: %02X %02X %02X %02X", data[0], data[1],
               data[2], data[3]);

  HazeLogDebug("RequestUnmarshal len: %zu", RawBufferLen(b));

  mpack_reader_t reader;
  mpack_reader_init_data(&reader, (const char *)RawBufferData(b),
                         RawBufferLen(b));

  Request *request = NULL;
  mpack_tag_t tag;

  /* Array */
  tag = mpack_read_tag(&reader);

  if (mpack_reader_error(&reader) != mpack_ok)
    goto fail;

  HazeLogDebug("Request root tag: %s",
               mpack_type_to_string(mpack_tag_type(&tag)));

  if (mpack_tag_type(&tag) != mpack_type_array) {
    HazeLogError("Unmarshal failed: Expected ARRAY, got %s",
                 mpack_type_to_string(mpack_tag_type(&tag)));
    goto fail;
  }

  if (mpack_tag_array_count(&tag) != 4) {
    HazeLogError("Unmarshal failed: Expected 4 items, got %u",
                 mpack_tag_array_count(&tag));
    goto fail;
  }

  request = ServerRequestNew();

  if (!request)
    goto fail;

  /* Type */
  tag = mpack_read_tag(&reader);

  if (mpack_reader_error(&reader) != mpack_ok)
    goto fail;

  if (mpack_tag_type(&tag) != mpack_type_uint &&
      mpack_tag_type(&tag) != mpack_type_int) {
    HazeLogError("Unmarshal failed [Type]: Expected INT/UINT, got %s",
                 mpack_type_to_string(mpack_tag_type(&tag)));
    goto fail;
  }

  if (mpack_tag_type(&tag) == mpack_type_uint)
    request->type = (HazeServerRPCType)mpack_tag_uint_value(&tag);
  else
    request->type = (HazeServerRPCType)mpack_tag_int_value(&tag);

  if (request->type != HAZE_RPC_REQUEST) {
    HazeLogError("Unmarshal failed [Type]: Expected %d, got %d",
                 HAZE_RPC_REQUEST, request->type);
    goto fail;
  }

  /* MsgID */
  tag = mpack_read_tag(&reader);

  if (mpack_reader_error(&reader) != mpack_ok)
    goto fail;

  if (mpack_tag_type(&tag) != mpack_type_uint &&
      mpack_tag_type(&tag) != mpack_type_int) {
    HazeLogError("Unmarshal failed [MsgID]: Expected INT/UINT, got %s",
                 mpack_type_to_string(mpack_tag_type(&tag)));
    goto fail;
  }

  if (mpack_tag_type(&tag) == mpack_type_uint)
    request->msgid = (uint32_t)mpack_tag_uint_value(&tag);
  else
    request->msgid = (uint32_t)mpack_tag_int_value(&tag);

  /* Method */
  tag = mpack_read_tag(&reader);

  if (mpack_reader_error(&reader) != mpack_ok)
    goto fail;

  if (mpack_tag_type(&tag) != mpack_type_str) {
    HazeLogError("Unmarshal failed [Method]: Expected STR, got %s",
                 mpack_type_to_string(mpack_tag_type(&tag)));
    goto fail;
  }

  uint32_t method_len = mpack_tag_str_length(&tag);

  request->method = malloc(method_len + 1);

  if (!request->method)
    goto fail;

  const char *method = mpack_read_bytes_inplace(&reader, method_len);

  if (!method || mpack_reader_error(&reader) != mpack_ok)
    goto fail;

  memcpy(request->method, method, method_len);
  request->method[method_len] = '\0';

  mpack_done_str(&reader);

  if (mpack_reader_error(&reader) != mpack_ok)
    goto fail;

  /* Params */
  mpack_discard(&reader);

  if (mpack_reader_error(&reader) != mpack_ok)
    goto fail;

  mpack_done_array(&reader);

  if (mpack_reader_error(&reader) != mpack_ok)
    goto fail;

  mpack_reader_destroy(&reader);
  return request;

fail:
  if (mpack_reader_error(&reader) != mpack_ok) {
    HazeLogError("MPack Reader Error: %s",
                 mpack_error_to_string(mpack_reader_error(&reader)));
  }

  mpack_reader_destroy(&reader);

  if (request)
    RequestFree(&request);

  return NULL;
}
void HazeServerRequestSetMethod(Request *request, const char *method) {
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
  if (!request)
    return NULL;

  size_t method_len = request->method ? strlen(request->method) + 1 : 0;
  size_t params_len =
      request->parameters ? RawBufferLen(request->parameters) : 0;
  size_t total_size =
      sizeof(request->type) + sizeof(request->msgid) + method_len + params_len;

  void *raw_data = malloc(total_size);
  if (!raw_data)
    return NULL;

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

  return RawBufferSetData(request->parameters, RawBufferData(b),
                          RawBufferLen(b));
}

void RequestFree(Request **request) {
  if (!request || !*request)
    return;

  free((*request)->method);
  free((*request)->parameters);
  free(*request);

  *request = NULL;
}

int RequestParamCount(const Request *r) {
    if (!r || !r->parameters)
        return 0;

    mpack_reader_t rd;
    mpack_reader_init_data(
        &rd,
        RawBufferData(r->parameters),
        RawBufferLen(r->parameters)
    );

    int count = mpack_expect_array(&rd);

    for (int i = 0; i < count; i++) {
        mpack_discard(&rd);
    }

    mpack_done_array(&rd);

    if (mpack_reader_destroy(&rd) != mpack_ok)
        return 0;

    return count;
}

bool RequestParamIsType(const Request *r, ParamType type, uint32_t index) {
    if (!r || !r->parameters)
        return false;

    mpack_reader_t reader;

    mpack_reader_init_data(
        &reader,
        RawBufferData(r->parameters),
        RawBufferLen(r->parameters)
    );

    uint32_t count = mpack_expect_array(&reader);

    if (index >= count) {
        mpack_reader_destroy(&reader);
        return false;
    }

    for (uint32_t i = 0; i < index; i++) {
        mpack_discard(&reader);
    }

    mpack_tag_t tag = mpack_peek_tag(&reader);

    bool result = false;

    switch (type) {
        case PARAM_STR:
            result = tag.type == mpack_type_str;
            break;

        case PARAM_INT:
            result = tag.type == mpack_type_int;
            break;

        case PARAM_UINT:
            result = tag.type == mpack_type_uint;
            break;

        case PARAM_BOOL:
            result = tag.type == mpack_type_bool;
            break;

        case PARAM_FLOAT:
            result = tag.type == mpack_type_float;
            break;

        case PARAM_DOUBLE:
            result = tag.type == mpack_type_double;
            break;

        case PARAM_ARRAY:
            result = tag.type == mpack_type_array;
            break;

        case PARAM_MAP:
            result = tag.type == mpack_type_map;
            break;

        case PARAM_NIL:
            result = tag.type == mpack_type_nil;
            break;

        default:
            result = false;
            break;
    }

    mpack_reader_destroy(&reader);

    return result;
}
