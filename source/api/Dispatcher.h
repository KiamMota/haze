#ifndef DISPATCHER_H
#define DISPATCHER_H


#include "Context.h"
#include "api/proto/Request.h"
#include "api/proto/Response.h"

Response* DispatchRPCMessage(Context* ctx, Request* rq);

#endif
