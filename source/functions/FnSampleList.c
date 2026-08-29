#include "Macros.h"
#include "Result.h"
#include "Session.h"
#include "audio/SampleList.h"
#include "proto/Object.h"
#include "proto/Request.h"
#include "proto/Response.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

// expected example request:
// [1, 67, "samplelist/import", ["rock_with_you.mp3"]]
//
Response *FnSampleListImportSample(Request *rq) {
    MSGID(rq);
    REQUIRE_REQUEST(rq);
    REQUIRE_ARGS(rq, 1);

    if (!RequestParamIsType(rq, OBJ_STR, 0))
        return ResponseCreateError(msgId, "Expected a string parameter.");

    Object **p_ptr = RequestParamGet(rq, 0);
    if (!p_ptr || !*p_ptr)
        return ResponseCreateError(msgId, "Invalid parameter object.");

    Object *param = *p_ptr;
    SampleList *list = SessionGetSampleList(SessionInstance);

    Result res = SampleListImportByFile(
        list,
        param->value.str_value
    );

    if (!ResultIsOk(res))
        return ResponseCreateError(msgId, res.msg);

    RETURN_SUCCESS;
}

// expected example request:
// [1, 67, "samplelist/delete", ["rock_with_you.mp3"]]
//
Response *FnSampleListDelete(Request *rq) {
    MSGID(rq);
    REQUIRE_REQUEST(rq);
    REQUIRE_ARGS(rq, 1);

    if (!RequestParamIsType(rq, OBJ_STR, 0))
        return ResponseCreateError(msgId, "Expected a string parameter.");

    Object **p_ptr = RequestParamGet(rq, 0);
    if (!p_ptr || !*p_ptr)
        return ResponseCreateError(msgId, "Invalid parameter object.");

    Object *param = *p_ptr;
    SampleList *list = SessionGetSampleList(SessionInstance);

    Result res = SampleListDeleteSampleByName(
        list,
        param->value.str_value
    );

    if (!ResultIsOk(res))
        return ResponseCreateError(msgId, res.msg);

    RETURN_SUCCESS;
}

// expected example request:
// [1, 67, "samplelist/get", ["rock_with_you.mp3"]]
// example response:
// [0, 67, nil, "rock_with_you.mp3"]

Response *FnSampleListGet(Request *rq) {
    MSGID(rq);
    REQUIRE_REQUEST(rq);
    REQUIRE_ARGS(rq, 1);

    if (!RequestParamIsType(rq, OBJ_STR, 0))
        return ResponseCreateError(msgId, "Expected a string parameter.");

    Object **p_ptr = RequestParamGet(rq, 0);
    if (!p_ptr || !*p_ptr)
        return ResponseCreateError(msgId, "Invalid parameter object.");

    Object *param = *p_ptr;
    SampleList *list = SessionGetSampleList(SessionInstance);

    Sample *sample = SampleListGetSampleByName(
        list,
        param->value.str_value
    );

    if (!sample)
        return ResponseCreateError(msgId, "sample not found");

    return ResponseCreateStrResult(
        msgId,
        SampleGetName(sample)
    );
}

// example request:
// [1, 67, "samplelist/list", []]
// example response:
// [0, 67, nil, ["foo", "foo2", "foo3"]]

Response *FnSampleListList(Request *rq) {
    MSGID(rq);
    REQUIRE_REQUEST(rq);
    REQUIRE_ARGS(rq, 0);

    SampleList *list = SessionGetSampleList(SessionInstance);

    const char **names = SampleListStr(list);

    if (!names)
        return ResponseCreateError(msgId, "failed to create sample list");

    Response *rs = ResponseCreateStrArrayResult(msgId, names);

    for (size_t i = 0; names[i]; i++)
        free((void *)names[i]);

    free(names);
    names = NULL;

    return rs;
}
