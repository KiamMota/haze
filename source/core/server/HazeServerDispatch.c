/* HazeServerFuncDispatch.c */
#include "core/server/HazeServerDispatch.h"
#include "core/functions/HazeFuncPing.h"
#include "core/proto/RawBuffer.h"
#include "core/proto/Request.h"
#include <stdlib.h>
#include <string.h>

/* ---------------------------------------------------------- */
/* Tabela de dispatch                                          */
/* ---------------------------------------------------------- */

static const HazeRpcEntry haze_rpc_table[] = {{"ping", HazeFuncPing},
                                              {NULL, NULL}};

HazeRpcHandler HazeServerDispatchLookup(const char *method) {
  if (!method)
    return NULL;
  for (int i = 0; haze_rpc_table[i].method; i++) {
    if (strcmp(haze_rpc_table[i].method, method) == 0)
      return haze_rpc_table[i].handler;
  }
  return NULL;
}

void *HazeServerDispatch(RawBuffer *b) {
  if (!b)
    return NULL;
  Request *req = RequestUnmarshal(b);
  if (!req)
    return NULL;

  HazeRpcHandler handler = HazeServerDispatchLookup(req->method);

  Response *res;
  if (!handler) {
    res = ResponseNew();
    ResponseSetMsgId(res, req->msgid);
    ResponseSetError(res, HAZE_RPC_ERROR_METHOD_NOT_FOUND);
  } else {
    res = handler(req);
  }

  void *buf = ResponseMarshal(res);

  RequestFree(&req);
  ResponseFree(&res);
  return buf;
}
