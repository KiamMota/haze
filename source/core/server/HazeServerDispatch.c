/* HazeServerFuncDispatch.c */
#include "HazeServerDispatch.h"
#include "HazeLog.h"
#include "core/functions/HazeFuncPing.h"
#include "core/proto/MessagePackRPC.h"
#include "core/proto/RawBuffer.h"
#include "core/proto/Request.h"
#include "core/proto/Response.h"
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


RawBuffer *HazeServerDispatch(RawBuffer *b) {
    if (!b) {
        HazeLogError("%s", "Dispatch received NULL RawBuffer");
        return NULL;
    }

    Request *req = RequestUnmarshal(b);
    if (!req) {
        HazeLogError("%s", "RequestUnmarshal failed: invalid or malformed buffer");
        
        // 1. Cria a estrutura de resposta para o erro
        Response *res = ResponseNew();
        if (!res) return NULL;

        // Como o request é NULL, não podemos acessar req->msgid. Usamos 0.
        ResponseSetMsgId(res, 0); 
        ResponseSetError(res, MPACKRPC_MALFORMED_REQ);

        // 2. Serializa a resposta de erro para enviar ao cliente
        RawBuffer *buf = ResponseMarshal(res); 
        if (!buf) {
            HazeLogError("%s", "ResponseMarshal failed: unable to serialize malformed messagepack error");
        }

        // 3. Libera a memória da resposta e sai imediatamente da função
        ResponseFree(&res);
        return buf; 
    }

    // Se passou do bloco acima, 'req' é garantidamente válido
    HazeLogDebug("Request parsed: ID=%d, Method='%s'", req->msgid, req->method ? req->method : "NULL");

    HazeRpcHandler handler = HazeServerDispatchLookup(req->method);

    Response *res = NULL;
    if (!handler) {
        HazeLogWarn("Method not found: '%s'", req->method);
        res = ResponseNew();
        if (res) {
            ResponseSetMsgId(res, req->msgid);
            ResponseSetError(res, HAZE_RPC_ERROR_METHOD_NOT_FOUND);
        }
    } else {
        HazeLogDebug("Executing handler for '%s'...", req->method);
        res = handler(req);
    }

    if (!res) {
        HazeLogError("%s", "Response structure (Response*) is NULL before ResponseMarshal");
        RequestFree(&req);
        return NULL;
    }

    RawBuffer *buf = ResponseMarshal(res); 
    if (!buf) {
        HazeLogError("%s", "ResponseMarshal failed: unable to serialize response");
    }

    RequestFree(&req);
    ResponseFree(&res);
    return buf;
}

