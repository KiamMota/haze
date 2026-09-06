#ifndef FNPROJECT_H
#define FNPROJECT_H

#include "api/proto/Request.h"
#include "api/proto/Response.h"

// required args: false
Response* FnProjectSave(Request* req);
// required args: true, 1
Response* FnProjectLoad(Request* req);

#endif
