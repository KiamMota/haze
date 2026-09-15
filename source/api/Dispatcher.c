#include "Context.h"
#include "api/functions/FnSampleList.h"
#include "api/functions/FnSession.h"
#include "audio/AudioEngine.h"
#include "msgpack/Object.h"
#include "api/proto/Request.h"
#include "api/proto/Response.h"
#include "audio/SampleList.h"
#include "msgpack/Object.h"
#include "session/Session.h"

#include <stdint.h>
#include <string.h>

Response *DispatchRPCMessage(const Context *ctx, Request *rq) {
  if (!ctx || !rq)
    return ResponseCreateError(0, "Invalid context or request.");

  const AudioEngine* eng = ContextGetAudioEngine(ctx);
  const Session *session = ContextGetSession(ctx);
  const SampleList *sampleList = SessionGetSampleList(session);

  const char *method_name = RequestMethod(rq);
  uint32_t msgid = RequestMsgId(rq);

  if (!method_name)
    return ResponseCreateError(msgid, "Method not specified.");

  if (strcmp(method_name, "test/ping") == 0) {
    if (RequestParamCount(rq) != 0)
      return ResponseCreateError(
          msgid, "Pong, but those arguments weren't necessary.");

    return ResponseCreateString(msgid, "pong!");
  }

  if (strcmp(method_name, "session/create") == 0) {
    if (RequestParamCount(rq) != 1)
      return ResponseCreateError(msgid, "Expected 1 parameter.");

    Object *obj = RequestParamGet(rq, 0);

    if (!ObjectExpect(obj, OBJ_STR))
      return ResponseCreateError(msgid, "Expected a string.");

    return ResponseCreateResult(
        msgid,
        FnSessionCreate(session, eng, ObjectGetStr(obj)));
  }

  if (strcmp(method_name, "session/get_name") == 0) {
    if (RequestParamCount(rq) != 0)
      return ResponseCreateError(msgid, "Expected 0 parameters.");

    return ResponseCreateString(
        msgid,
        FnSessionGetName(session));
  }

  if (strcmp(method_name, "session/get_working_time") == 0) {
    if (RequestParamCount(rq) != 0)
      return ResponseCreateError(msgid, "Expected 0 parameters.");

    return ResponseCreateInt(
        msgid,
        (int64_t)FnSessionGetWorkingTime(session));
  }

  if (strcmp(method_name, "samplelist/import") == 0) {
    if (RequestParamCount(rq) != 1)
      return ResponseCreateError(msgid, "Expected 1 parameter.");

    Object *obj = RequestParamGet(rq, 0);

    if (!ObjectExpect(obj, OBJ_STR))
      return ResponseCreateError(msgid, "Expected a string.");

    return ResponseCreateResultAudio(
        msgid,
        FnSampleListImportSample(sampleList, eng, ObjectGetStr(obj)));
  }

  if (strcmp(method_name, "samplelist/remove") == 0) {
    if (RequestParamCount(rq) != 1)
      return ResponseCreateError(msgid, "Expected 1 parameter.");

    Object *obj = RequestParamGet(rq, 0);

    if (!ObjectExpect(obj, OBJ_STR))
      return ResponseCreateError(msgid, "Expected a string.");

    return ResponseCreateResultAudio(
        msgid,
        FnSampleListRemoveSample(sampleList, ObjectGetStr(obj)));
  }

  if (strcmp(method_name, "samplelist/play") == 0) {
    if (RequestParamCount(rq) != 1)
      return ResponseCreateError(msgid, "Expected 1 parameter.");

    Object *obj = RequestParamGet(rq, 0);

    if (!ObjectExpect(obj, OBJ_STR))
      return ResponseCreateError(msgid, "Expected a string.");

    return ResponseCreateResultAudio(
        msgid,
        FnSamplePlay(sampleList, ObjectGetStr(obj)));
  }

  return ResponseCreateError(msgid, "Method not found.");
}
