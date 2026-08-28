#ifndef HAZE_FUNC_SESSION_CREATE
#define HAZE_FUNC_SESSION_CREATE

#include "proto/Request.h"
#include "proto/Response.h"
Response* FnSessionCreate(Request* req);
Response* FnSessionGetName(Request* rq);
Response* FnSessionGetPath(Request* rq);
Response* FnSessionGetWorktime(Request* rq);


#endif
