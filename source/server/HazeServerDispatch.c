/* HazeServerFuncDispatch.c */
#include "HazeServerDispatch.h"
#include "HazeLog.h"
#include "proto/MessagePackRPC.h"
#include "proto/Request.h"
#include "proto/Response.h"
#include "server/FuncTable.h"
#include <string.h>

/* ---------------------------------------------------------- */
/* Tabela de dispatch                                          */
/* ---------------------------------------------------------- */

HazeRpcHandler HazeServerDispatchLookup(const char *method) {
  if (!method)
    return NULL;
  for (int i = 0; haze_rpc_table[i].method; i++) {
    if (strcmp(haze_rpc_table[i].method, method) == 0)
      return haze_rpc_table[i].handler;
  }
  return NULL;
}

Response *HazeServerDispatch(Request *req) {
  if (!req) {
    HazeLogWarn("Received NULL request.");
    return NULL;
  }

  HazeLogDebug("Received request | ID: %u | Method: '%s'", RequestMsgId(req),
               RequestMethod(req));

  HazeRpcHandler handler = HazeServerDispatchLookup(RequestMethod(req));

  if (!handler) {
    HazeLogWarn("Method not found | ID: %u | Method: '%s'", RequestMsgId(req),
                RequestMethod(req));

    Response *res = ResponseNew();

    ResponseSetMsgId(res, RequestMsgId(req));
    ResponseSetError(res, HAZE_RPC_ERROR_METHOD_NOT_FOUND);

    HazeLogDebug("Sending response | ID: %u | Error: %d", ResponseMsgId(res),
                 ResponseError(res));

    return res;
  }

  Response *res = handler(req);
  HazeLogDebug("RESULT ptr=%p len=%zu first=%u", RawBufferData(res->result),
               RawBufferLen(res->result),
               ((unsigned char *)RawBufferData(res->result))[0]);

  HazeLogDebug("Sending response | ID: %u | Error: %d | Result: %.*s",
               ResponseMsgId(res), ResponseError(res),
               (int)RawBufferLen(res->result),
               (const char *)RawBufferData(res->result));
  HazeLogDebug("Sending response | ID: %u | Error: %d | Result: %.*s",
               ResponseMsgId(res), ResponseError(res),
               (int)RawBufferLen(res->result),
               (const char *)RawBufferData(res->result));

  return res;
}
