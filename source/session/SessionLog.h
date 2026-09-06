#ifndef SESSION_LOG_H
#define SESSION_LOG_H

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

typedef enum {
    SESSION_LOG_DEBUG,
    SESSION_LOG_INFO,
    SESSION_LOG_WARN,
    SESSION_LOG_ERROR,
} SessionLogLevel;

static inline const char *
SessionLogLevelString(SessionLogLevel level)
{
    switch (level) {
    case SESSION_LOG_DEBUG: return "DBG";
    case SESSION_LOG_INFO:  return "INF";
    case SESSION_LOG_WARN:  return "WRN";
    case SESSION_LOG_ERROR: return "ERR";
    default:                return "???";
    }
}

static inline void
SessionLog(
    FILE *output,
    SessionLogLevel level,
    const char *module,
    const char *file,
    int line,
    const char *fmt,
    ...
)
{
    if (!output || !module || !file || !fmt)
        return;

    struct timespec ts;
    timespec_get(&ts, TIME_UTC);

    struct tm tm;
    localtime_r(&ts.tv_sec, &tm);

    char timebuf[16];

    strftime(
        timebuf,
        sizeof(timebuf),
        "%H:%M:%S",
        &tm
    );

    const char *short_file = file;

    for (const char *p = file; *p; ++p) {
        if (*p == '/' || *p == '\\')
            short_file = p + 1;
    }

    fprintf(
        output,
        "[%s.%03ld] [%s] [Session:%s] (%s:%d) ",
        timebuf,
        ts.tv_nsec / 1000000,
        SessionLogLevelString(level),
        module,
        short_file,
        line
    );

    va_list args;

    va_start(args, fmt);
    vfprintf(output, fmt, args);
    va_end(args);

    fputc('\n', output);

    fflush(output);
}

#define SessionLogDebug(output, module, fmt, ...) \
    SessionLog( \
        output, \
        SESSION_LOG_DEBUG, \
        module, \
        __FILE__, \
        __LINE__, \
        fmt, \
        ##__VA_ARGS__ \
    )

#define SessionLogInfo(output, module, fmt, ...) \
    SessionLog( \
        output, \
        SESSION_LOG_INFO, \
        module, \
        __FILE__, \
        __LINE__, \
        fmt, \
        ##__VA_ARGS__ \
    )

#define SessionLogWarn(output, module, fmt, ...) \
    SessionLog( \
        output, \
        SESSION_LOG_WARN, \
        module, \
        __FILE__, \
        __LINE__, \
        fmt, \
        ##__VA_ARGS__ \
    )

#define SessionLogError(output, module, fmt, ...) \
    SessionLog( \
        output, \
        SESSION_LOG_ERROR, \
        module, \
        __FILE__, \
        __LINE__, \
        fmt, \
        ##__VA_ARGS__ \
    )

#endif
