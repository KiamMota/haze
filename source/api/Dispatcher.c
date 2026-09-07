#include "Result.h"
#include "api/Route.h"
#include "api/functions/Funcs.h"
#include "api/proto/Object.h"
#include "api/proto/Request.h"
#include "api/proto/Response.h"
#include "proto/Request.h"

Response* DispatchRPCMessage(Request* rq) {
    if (!rq) {
        return ResponseCreateError(0, "Invalid request.");
    }

    const char* method_name = RequestMethod(rq);
    if (!method_name) {
        return ResponseCreateError(RequestMsgId(rq), "Method not specified.");
    }

    static const Route routes[] = {
        { "session/init",         haze_session_init },
        { "session/create",       haze_session_create },
        { "session/get_name",     haze_session_get_name },
        { "session/get_worktime", haze_session_get_working_time},
        { "sample/import",        haze_sample_list_import_sample },
        { "sample/play",          wrap_haze_sample_play },
        { NULL,                   NULL }
    };

    for (int i = 0; routes[i].name != NULL; i++) {
        if (strcmp(routes[i].name, method_name) == 0) {
            return routes[i].handler(rq);
        }
    }

    return ResponseCreateError(RequestMsgId(rq), "Method not found.");
}
