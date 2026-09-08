#include "api/functions/FnSampleList.h"
#include "api/functions/FnSession.h"
#include "api/proto/Object.h"
#include "api/proto/Request.h"
#include "api/proto/Response.h"

#include <stdint.h>
#include <string.h>

Response *DispatchRPCMessage(Request *rq) {
  if (!rq) {
    return ResponseCreateError(0, "Invalid request.");
  }

  const char *method_name = RequestMethod(rq);
  uint32_t msgid = RequestMsgId(rq);

  if (!method_name) {
    return ResponseCreateError(msgid, "Method not specified.");
  }

  if (strcmp(method_name, "session/init") == 0) {
    if (RequestParamCount(rq) != 0) {
      return ResponseCreateError(msgid, "Expected 0 parameters.");
    }

    return ResponseCreateResult(msgid, FnSessionInit());
  }

  if (strcmp(method_name, "session/create") == 0) {
    if (RequestParamCount(rq) != 1) {
      return ResponseCreateError(msgid, "Expected 1 parameter.");
    }

    Object *obj = RequestParamGet(rq, 0);

    if (!obj) {
      return ResponseCreateError(msgid, "Invalid parameter.");
    }

    ObjectValue value = ObjectGetValue(obj);

    if (!value.str_value) {
      return ResponseCreateError(msgid, "Invalid session name.");
    }

    return ResponseCreateResult(msgid, FnSessionCreate(value.str_value));
  }

  if (strcmp(method_name, "session/get_name") == 0) {
    if (RequestParamCount(rq) != 0) {
      return ResponseCreateError(msgid, "Expected 0 parameters.");
    }

    return ResponseCreateString(msgid, FnSessionGetName());
  }

  if (strcmp(method_name, "session/get_working_time") == 0) {
    if (RequestParamCount(rq) != 0) {
      return ResponseCreateError(msgid, "Expected 0 parameters.");
    }

    return ResponseCreateInt(msgid, (int64_t)FnSessionGetWorkingTime());
  }

  if (strcmp(method_name, "sample_list/import") == 0) {
    if (RequestParamCount(rq) != 1) {
      return ResponseCreateError(msgid, "Expected 1 parameter.");
    }

    Object *obj = RequestParamGet(rq, 0);

    if (!obj) {
      return ResponseCreateError(msgid, "Invalid parameter.");
    }

    ObjectValue value = ObjectGetValue(obj);

    if (!value.str_value) {
      return ResponseCreateError(msgid, "Invalid sample path.");
    }

    return ResponseCreateResult(msgid, FnSampleListImportSample(value.str_value));
  }

  return ResponseCreateError(msgid, "Method not found.");
}
