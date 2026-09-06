#ifndef DISPATCHER_H
#define DISPATCHER_H


#include "api/proto/Request.h"
#include "api/proto/Response.h"

Response* DispatchRPCMessage(const char* methodName, Request* rq);

#endif
