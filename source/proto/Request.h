/* HazeServerRequest.h */
#ifndef HAZE_SERVER_REQUEST_H
#define HAZE_SERVER_REQUEST_H

#include "MessagePackRPC.h"
#include "RawBuffer.h"
#include "mpack/mpack-reader.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
  PARAM_UND = 0,
  PARAM_NIL,
  PARAM_BOOL,
  PARAM_INT,
  PARAM_UINT,
  PARAM_FLOAT,
  PARAM_DOUBLE,
  PARAM_STR,
  PARAM_BIN,
  PARAM_ARRAY,
  PARAM_MAP,
} RequestParamType;

typedef struct {
  mpack_reader_t reader;
} RequestReader;

typedef union {
  bool bool_value;
  int64_t int_value;
  uint64_t uint_value;
  float float_value;
  double double_value;
  const char *str_value;
  RawBuffer *bin_value;
} RequestParamValue;

typedef struct {
  RequestParamType type;
  RequestParamValue value;
  size_t size;
} RequestParam;

typedef struct {
  HazeServerRPCType type;
  uint32_t msgid;
  char *method;
  RequestParam **parameters;
} Request;

RequestParam *RequestParamGet(const Request *rq, uint32_t index);
bool RequestParamAppend(Request *rq, RequestParam *param, uint32_t ind);
RequestParam* RequestParamNew(void);
void RequestParamFree(RequestParam** r);
RequestParam* RequestParamInitStr(const char *str);
RequestParam* RequestParamInitInt(int value);
RequestParam* RequestParamInitBool(bool value);
RequestParam* RequestParamInitNil(void);
RequestParam* RequestParamInitDouble(double f);
RequestParam* RequestParamInitFloat(float f);
RequestParam* RequestParamInitBin(RawBuffer* bf);
static inline RequestParamType RequestParamTypeGet(const RequestParam* r) {
  return r->type;
}
static inline RequestParamValue RequestParamValueGet(const RequestParam* r) {
  return r->value;
}
Request *RequestUnmarshal(RawBuffer *b);
RawBuffer *RequestMarshal(Request *r);
Request *RequestNew(void);

void RequestSetMethod(Request *request, const char *method);

static inline uint32_t RequestMsgId(const Request *r) { return r->msgid; }
static inline const char *RequestMethod(const Request *r) { return r->method; }

void RequestFree(Request **request);

uint32_t RequestParamCount(const Request *r);

bool RequestParamIsType(const Request *r, RequestParamType type,
                        uint32_t index);

const char* RequestPrint(const Request* r);

#endif
