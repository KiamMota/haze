/* HazeServerFuncDispatch.c */
#include "HazeServerDispatch.h"
#include "functions/HazeFuncPing.h"
#include "server/HazeServerMRPC.h"
#include <string.h>
#include <stdlib.h>


/* ---------------------------------------------------------- */
/* Tabela de dispatch                                          */
/* ---------------------------------------------------------- */

static const HazeRpcEntry haze_rpc_table[] = {
    { "ping", HazeFuncPing },
    { NULL,   NULL          }
};

HazeRpcHandler HazeServerDispatchLookup(const char *method)
{
    if (!method) return NULL;
    for (int i = 0; haze_rpc_table[i].method; i++) { 
        if (strcmp(haze_rpc_table[i].method, method) == 0)
            return haze_rpc_table[i].handler;
    }
    return NULL;
}

void *HazeServerDispatch(const void *data, size_t len, size_t *out_len)
{
    HazeServerRequest *req = HazeServerRequestUnmarshal(data, len);
    if (!req) return NULL;

    HazeRpcHandler handler = HazeServerDispatchLookup(req->method);

    HazeServerResponse *res;
    if (!handler) {
        res = HazeServerResponseNew();
        HazeServerResponseSetMsgId(res, req->msgid);
        HazeServerResponseSetError(res, HAZE_RPC_ERROR_METHOD_NOT_FOUND); 
    } else {
        res = handler(req);
    }

    void *buf = HazeServerResponseMarshal(res, out_len);

    HazeServerRequestFree(&req);
    HazeServerResponseFree(&res);
    return buf;
}
