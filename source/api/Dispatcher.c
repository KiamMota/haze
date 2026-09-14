#include "Result.h"
#include "api/functions/FnSampleList.h"
#include "api/functions/FnSession.h"
#include "audio/Sample.h"
#include "msgpack/Object.h"
#include "api/proto/Request.h"
#include "api/proto/Response.h"
#include "audio/SampleList.h"
#include "msgpack/Object.h"
#include "session/Session.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

Response *DispatchRPCMessage(Request *rq) {

  SampleList *sampleList = SessionGetSampleList(SessionInstance);

  if (!rq) {
    return ResponseCreateError(0, "Invalid request.");
  }

  const char *method_name = RequestMethod(rq);
  uint32_t msgid = RequestMsgId(rq);

  if (!method_name) {
    return ResponseCreateError(msgid, "Method not specified.");
  }
  if (strcmp(method_name, "test/ping") == 0) {
    if (RequestParamCount(rq) == 0) {
      return ResponseCreateString(msgid, "pong!");
    }
    return ResponseCreateError(msgid, "Pong, but those arguments weren't necessary.");

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

  if (strcmp(method_name, "samplelist/import") == 0) {
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

    return ResponseCreateResult(msgid,
                                FnSampleListImportSample(value.str_value));
  }

  if (strcmp(method_name, "sample/play") == 0) {
    if (RequestParamCount(rq) != 1) {
      return ResponseCreateError(msgid, "Expected 1 parameter.");
    }

    // 1. ALOCAÇÃO CORRETA: Cria a struct na memória e zera os campos
    Sample* ephSample = calloc(1, sizeof(Sample));
    if (!ephSample) {
      return ResponseCreateError(msgid, "Out of memory allocating sample.");
    }

    Object* obj = RequestParamGet(rq, 0);
    ObjectValue fileName = ObjectGetValue(obj);
    
    Result res = SampleInitFromFile(ephSample, fileName.str_value);
    
    if (!ResultIsOk(res)) {
      // 2. PREVENÇÃO DE VAZAMENTO: Se der erro ao carregar, libera a memória!
      free(ephSample);
      return ResponseCreateError(msgid, res.msg);
    }
    
    SamplePlay(ephSample);
    
    return ResponseCreateString(msgid, "Ok");
  }

  return ResponseCreateError(msgid, "Method not found.");

  if (strcmp(method_name, "samplelist/remove") == 0) {
    if (RequestParamCount(rq) < 1) {
      return ResponseCreateError(msgid, "Expected 1 parameter");
    }
    Object *obj = RequestParamGet(rq, 0);
    if (!ObjectExpect(obj, OBJ_STR)) {
      return ResponseCreateError(msgid, "Object is not an string");
    }
    const char *str = ObjectGetStr(obj);
    Result res = SampleListDeleteSampleByName(sampleList, str);
    return ResponseCreateResult(msgid, res);
  }
}
