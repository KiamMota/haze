#ifndef SAMPLE_LIST_H
#define SAMPLE_LIST_H

#include "proto/Request.h"
#include "proto/Response.h"

Response *FnSampleListImportSample(Request *rq);
Response *FnSampleListDelete(Request *rq);
Response *FnSampleListGet(Request *rq);
Response *FnSampleListList(Request *rq);

#endif
