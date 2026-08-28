#include "Request.h"
#include "HazeLog.h"
#include "HazeMacros.h"
#include "audio/HazeEngine.h"
#include "mpack/mpack-common.h"
#include "mpack/mpack-expect.h"
#include "mpack/mpack-platform.h"
#include "mpack/mpack-reader.h"
#include "mpack/mpack-writer.h"
#include "proto/MessagePackRPC.h"
#include "proto/RawBuffer.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

Request *RequestNew(void) {
  Request *request = calloc(1, sizeof(Request));

  request->type = HAZE_RPC_REQUEST;

  request->method = calloc(1, sizeof(char));
  if (!request->method) {
    free(request);
    return NULL;
  }

  request->parameters = malloc(sizeof(RequestParam *));
  if (!request->parameters) {
    free(request->method);
    free(request);
    return NULL;
  }

  request->parameters[0] = NULL;
  return request;
}

RequestParam *RequestParamNew(void) {
  RequestParam *param = malloc(sizeof(RequestParam));
  param->size = 0;
  param->type = PARAM_NIL;
  RequestParamValue v = {.bin_value = 0};
  param->value = v;
  return param;
}

void RequestParamFree(RequestParam **r) {
  PTR_FREE_ASSERT(r);

  (*r)->size = 0;
  free((*r));
  *r = NULL;
}

RequestParam *RequestParamGet(const Request *r, uint32_t ind) {
  if (!r || !r->parameters)
    return NULL;
  uint32_t i = 0;
  while (r->parameters[i] != NULL) {
    if (i == ind)
      return r->parameters[i];
    i++;
  }
  return NULL; // ind >= count ou hole
}

RequestParam *RequestParamInitStr(const char *str) {
  RequestParam *param = RequestParamNew();
  param->type = PARAM_STR;
  param->size = strlen(str);
  RequestParamValue v = {.str_value = str};
  param->value = v;
  return param;
}
RequestParam *RequestParamInitBool(bool value) {
  RequestParam *param = RequestParamNew();
  RequestParamValue v = {.bool_value = value};
  param->type = PARAM_BOOL;
  param->value = v;
  param->size = 1;
  return param;
}

RequestParam *RequestParamInitInt(int val) {
  RequestParam *param = RequestParamNew();
  RequestParamValue v = {.int_value = val};
  param->type = PARAM_INT;
  param->value = v;
  param->size = 1;
  return param;
}

RequestParam *RequestParamInitNil(void) {
  RequestParam *param = RequestParamNew();

  param->type = PARAM_NIL;
  param->size = 0;

  return param;
}
RequestParam *RequestParamInitDouble(double f) {
  RequestParam *param = RequestParamNew();

  RequestParamValue v = {.double_value = f};

  param->type = PARAM_DOUBLE;
  param->value = v;
  param->size = 1;

  return param;
}

RequestParam *RequestParamInitFloat(float f) {
  RequestParam *param = RequestParamNew();

  RequestParamValue v = {.float_value = f};

  param->type = PARAM_FLOAT;
  param->value = v;
  param->size = 1;

  return param;
}

RequestParam *RequestParamInitBin(RawBuffer *bf) {
  RequestParam *param = RequestParamNew();

  RequestParamValue v = {.bin_value = (void *)RawBufferData(bf)};

  param->type = PARAM_BIN;
  param->value = v;
  param->size = RawBufferLen(bf);

  return param;
}

bool RequestParamAppend(Request *rq, RequestParam *param, uint32_t ind) {
  if (!rq || !rq->parameters || !param) {
    printf("param: %p\n", (void *)param);
    printf("rq: %p\n", (void *)rq);
    fflush(stdout);
    return false;
  }

  if (rq->parameters[ind] != NULL) {
    return false;
  }

  int tamanho_antigo = RequestParamCount(rq);

  RequestParam **new_params =
      realloc(rq->parameters, sizeof(RequestParam *) * (ind + 2));

  if (!new_params) {
    return false; // Falha de memória (realloc retornou NULL)
  }

  rq->parameters = new_params;

  for (uint32_t i = tamanho_antigo; i < ind; i++) {
    rq->parameters[i] = NULL;
  }

  rq->parameters[ind] = param;

  rq->parameters[ind + 1] = NULL;

  return true;
}

RawBuffer *RequestMarshal(Request *request) {
  if (!request) {
    return NULL;
  }

  char *data = NULL;
  size_t size = 0;
  mpack_writer_t writer;

  mpack_writer_init_growable(&writer, &data, &size);
  if (mpack_writer_error(&writer) != mpack_ok) {
    HazeLogError("RequestMarshal init failed: %s",
                 mpack_error_to_string(mpack_writer_error(&writer)));
    return NULL;
  }

  /* array principal: [type, msgid, method, params] */
  mpack_start_array(&writer, 4);

  mpack_write_uint(&writer, (uint64_t)request->type);
  mpack_write_uint(&writer, (uint64_t)request->msgid);

  if (request->method) {
    mpack_write_str(&writer, request->method,
                    (uint32_t)strlen(request->method));
  } else {
    mpack_write_nil(&writer);
  }

  /* sub-array de parâmetros */
  int param_count = RequestParamCount(request);
  mpack_start_array(&writer, (uint32_t)param_count);

  for (int i = 0; i < param_count; i++) {
    RequestParam *param = request->parameters[i];

    if (param == NULL) {
      mpack_write_nil(&writer);
      continue;
    }

    switch (param->type) {
    case PARAM_NIL:
      mpack_write_nil(&writer);
      break;

    case PARAM_BOOL:
      mpack_write_bool(&writer, param->value.bool_value);
      break;

    case PARAM_INT:
      mpack_write_int(&writer, param->value.int_value);
      break;

    case PARAM_UINT:
      mpack_write_uint(&writer, param->value.uint_value);
      break;

    case PARAM_FLOAT:
      mpack_write_float(&writer, param->value.float_value);
      break;

    case PARAM_DOUBLE:
      mpack_write_double(&writer, param->value.double_value);
      break;

    case PARAM_STR:
      if (param->value.str_value) {
        mpack_write_str(&writer, param->value.str_value, (uint32_t)param->size);
      } else {
        mpack_write_nil(&writer);
      }
      break;

    case PARAM_BIN:
      if (param->value.bin_value && param->value.bin_value->data) {
        mpack_write_bin(&writer, (const char *)param->value.bin_value->data,
                        (uint32_t)param->value.bin_value->len);
      } else {
        mpack_write_nil(&writer);
      }
      break;

    case PARAM_ARRAY:
    case PARAM_MAP:
    case PARAM_UND:
    default:
      mpack_write_nil(&writer);
      break;
    }
  }

  mpack_finish_array(&writer); /* params */
  mpack_finish_array(&writer); /* request */

  mpack_error_t error = mpack_writer_destroy(&writer);
  if (error != mpack_ok) {
    HazeLogError("RequestMarshal failed: %s", mpack_error_to_string(error));
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

  mpack_reader_t reader;
  mpack_reader_init_data(&reader, RawBufferData(b), RawBufferLen(b));

  Request *request = NULL;
  mpack_tag_t tag;

  /* Array */
  tag = mpack_read_tag(&reader);

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

  request = RequestNew();

  /* Type */
  tag = mpack_read_tag(&reader);

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

  /*Method */

  tag = mpack_read_tag(&reader);
  if (mpack_tag_type(&tag) != mpack_type_str) {
    HazeLogError("Unmarshal failed [Method]: Expected STR, got %s",
                 mpack_type_to_string(mpack_tag_type(&tag)));
    goto fail;
  }

  // this method does not take ownership of the memory.
  uint32_t method_len = mpack_tag_str_length(&tag);
  const char *method_bytes = mpack_read_bytes_inplace(&reader, method_len);
  if (!method_bytes) {
    goto fail;
  }

  char *method_copy = malloc((size_t)method_len + 1);
  if (!method_copy) {
    goto fail;
  }
  memcpy(method_copy, method_bytes, method_len);
  method_copy[method_len] = '\0';

  RequestSetMethod(request, method_copy);
  free(method_copy); // SetMethod copia de novo internamente

  mpack_done_str(&reader);

  mpack_done_str(&reader);

  /* Params */

  mpack_tag_t param_arr = mpack_read_tag(&reader);

  if (mpack_tag_type(&param_arr) != mpack_type_array) {
    HazeLogError("%s", "Unmarshal failed [Params]: parameters is not array");
    goto fail;
  }

  uint32_t count = mpack_tag_array_count(&param_arr);
  for (uint32_t i = 0; i < count; i++) {
    mpack_tag_t type_element = mpack_read_tag(&reader);

    switch (mpack_tag_type(&type_element)) {

    case mpack_type_nil:
      RequestParamAppend(request, RequestParamInitNil(), i);
      break;

    case mpack_type_bool: {
      bool v = mpack_tag_bool_value(&type_element);

      RequestParamAppend(request, RequestParamInitBool(v), i);
      break;
    }

    case mpack_type_int:
    case mpack_type_uint: {
      int v = mpack_tag_int_value(&type_element);

      RequestParamAppend(request, RequestParamInitInt(v), i);
      break;
    }

    case mpack_type_float: {
      float v = mpack_tag_float_value(&type_element);

      RequestParamAppend(request, RequestParamInitFloat(v), i);
      break;
    }

    case mpack_type_double: {
      double v = mpack_tag_double_value(&type_element);

      RequestParamAppend(request, RequestParamInitDouble(v), i);
      break;
    }

    case mpack_type_str: {
      size_t len = mpack_tag_str_length(&type_element);

      char *buff = malloc(len + 1);

      mpack_read_bytes(&reader, buff, len);
      buff[len] = '\0';
      RequestParamAppend(request, RequestParamInitStr(buff), i);
      free(buff);
      break;
    }

    case mpack_type_bin: {
      size_t len = mpack_tag_bin_length(&type_element);

      void *buff = malloc(len);
      mpack_read_bytes(&reader, buff, len);
      RequestParamAppend(request, RequestParamInitBin(RawBufferNew(buff, len)),
                         i);
      free(buff);
      break;
    }

    default:
      break;
    }
  }

  mpack_done_array(&reader);
  mpack_discard(&reader);
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

void RequestSetMethod(Request *request, const char *method) {
  if (!request)
    return;

  free(request->method);
  request->method = NULL;

  if (!method)
    return;

  size_t meth_len = strlen(method);
  char *temp = malloc(meth_len + 1);
  if (!temp)
    return; // Proteção contra falha de malloc antes de qualquer escrita

  strcpy(temp, method);
  request->method = temp;
}

void RequestFree(Request **request) {
  if (!request || !*request)
    return;

  if ((*request)->method) {
    free((*request)->method);
    (*request)->method = NULL;
  }

  if ((*request)->parameters) {
    int count = RequestParamCount(*request);

    if (count > 0) {
      for (int i = 0; i < count; i++) {
        if ((*request)->parameters[i]) {
          RequestParamFree(&((*request)->parameters[i]));
        }
      }
    }
    free((*request)->parameters);
    (*request)->parameters = NULL;
  }

  free(*request);
  *request = NULL;
}

uint32_t RequestParamCount(const Request *r) {
  if (!r || !r->parameters)
    return 0;

  size_t i = 0;

  while (r->parameters[i] != NULL) {
    i++;
  }

  return (int)i;
}

bool RequestParamIsType(const Request *r, RequestParamType type,
                        uint32_t index) {
  if (!r || !r->parameters)
    return false;

  if (index >= (uint32_t)RequestParamCount(r))
    return false;

  RequestParam *param = r->parameters[index];

  if (!param)
    return false;

  return RequestParamTypeGet(param) == type;
}

const char* RequestPrint(const Request *r) {
  printf("[");
  printf("%d, ", r->type);
  printf("%u, ", r->msgid);
  printf("\"%s\", [", r->method);

  for (uint32_t i = 0; i < RequestParamCount(r); i++) {
    RequestParam *param = r->parameters[i];
    RequestParamValue value = RequestParamValueGet(param);

    switch (RequestParamTypeGet(param)) {
    case PARAM_INT:
      printf("%d", value.int_value);
      break;

    case PARAM_STR:
      printf("\"%s\"", value.str_value);
      break;

    case PARAM_BOOL:
      printf("%s", value.bool_value ? "true" : "false");
      break;

    default:
      printf("<unknown>");
      break;
    }

    if (i + 1 < RequestParamCount(r))
      printf(", ");
  }

  printf("]]\n");
}
