#ifndef MACR_H
#define MACR_H

#include <stddef.h>
#include <stdint.h>

#define MSGID(rq) \
    uint64_t msgId = RequestMsgId(rq)

#define REQUIRE_REQUEST(rq) \
    do { \
        if (!(rq)) \
            return NULL; \
    } while (0)

#define NEW_RESPONSE() \
    Response *rs = ResponseNew(); \
    if (!rs) \
        return NULL

#define RETURN_SUCCESS \
    return ResponseCreateNilResult(msgId)

#define REQUIRE_ARGS(rq, n) \
    do { \
        size_t count = RequestParamCount(rq); \
        if (count != (size_t)(n)) { \
            char buf[64]; \
            snprintf( \
                buf, \
                sizeof(buf), \
                count < (size_t)(n) \
                    ? "too few args (expected %zu, got %zu)" \
                    : "too many args (expected %zu, got %zu)", \
                (size_t)(n), \
                count \
            ); \
            return ResponseCreateError(msgId, buf); \
        } \
    } while (0)

#endif
