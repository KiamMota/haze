/* HazeServerFuncDispatch.c */
#include "HazeServerDispatch.h"
#include "HazeLog.h"
#include "proto/MessagePackRPC.h"
#include "proto/RawBuffer.h"
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


Response *HazeServerDispatch(Request *req)
{
    if (!req) {
        HazeLogError("%s", "Dispatch received NULL Request");
        return NULL;
    }

    HazeLogDebug(
        "Request dispatched: ID=%d, Method='%s'",
        req->msgid,
        req->method ? req->method : "NULL"
    );

    HazeRpcHandler handler = HazeServerDispatchLookup(req->method);

    if (!handler) {
        HazeLogWarn(
            "Method not found: '%s'",
            req->method ? req->method : "NULL"
        );

        Response *res = ResponseNew();
        if (!res) {
            HazeLogError("%s", "Failed to create error response");
            return NULL;
        }

        ResponseSetMsgId(res, req->msgid);
        ResponseSetError(res, HAZE_RPC_ERROR_METHOD_NOT_FOUND);

        return res;
    }

    HazeLogDebug("Executing handler for '%s'...", req->method);

    Response *res = handler(req);

    if (!res) {
        HazeLogError(
            "Handler returned NULL for method '%s'",
            req->method ? req->method : "NULL"
        );
        return NULL;
    }

    return res;
}
