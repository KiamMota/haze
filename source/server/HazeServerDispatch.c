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


  HazeRpcHandler handler = HazeServerDispatchLookup(RequestMethod(req));

  if (!handler) {
    return ResponseCreateError(RequestMsgId(req), "Method not found");
  }

  Response *res = handler(req);
  return res;
}
