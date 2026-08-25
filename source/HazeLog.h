#ifndef HAZE_LOG_DEF
#define HAZE_LOG_DEF

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef enum {
  HAZE_LOG_DEBUG,
  HAZE_LOG_INFO,
  HAZE_LOG_WARN,
  HAZE_LOG_ERROR,
  HAZE_LOG_FATAL,
} HazeLogLevel;

#define HAZE_COLOR_RESET "\033[0m"
#define HAZE_COLOR_GRAY "\033[90m"
#define HAZE_COLOR_CYAN "\033[36m"
#define HAZE_COLOR_YELLOW "\033[33m"
#define HAZE_COLOR_RED "\033[31m"
#define HAZE_COLOR_MAGENTA "\033[35m"

static inline const char *_HazeLog_level_str(HazeLogLevel level) {
  switch (level) {
  case HAZE_LOG_DEBUG:
    return HAZE_COLOR_GRAY "DEBUG" HAZE_COLOR_RESET;
  case HAZE_LOG_INFO:
    return HAZE_COLOR_CYAN "INFO " HAZE_COLOR_RESET;
  case HAZE_LOG_WARN:
    return HAZE_COLOR_YELLOW "WARN " HAZE_COLOR_RESET;
  case HAZE_LOG_ERROR:
    return HAZE_COLOR_RED "ERROR" HAZE_COLOR_RESET;
  case HAZE_LOG_FATAL:
    return HAZE_COLOR_MAGENTA "FATAL" HAZE_COLOR_RESET;
  default:
    return "?????";
  }
}

static inline void _HazeLog(HazeLogLevel level, const char *file, int line,
                            const char *fmt, ...) {
  time_t t = time(NULL);
  struct tm *tm = localtime(&t);
  char timebuf[20];
  strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", tm);

  fprintf(stderr, "%s[%s]%s [%s] %s%s:%d%s | ", HAZE_COLOR_GRAY, timebuf,
          HAZE_COLOR_RESET, _HazeLog_level_str(level), HAZE_COLOR_GRAY, file,
          line, HAZE_COLOR_RESET);

  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);

  fprintf(stderr, "\n");

  if (level == HAZE_LOG_FATAL)
    abort();
}

#define HazeLogDebug(fmt, ...)                                                 \
  _HazeLog(HAZE_LOG_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define HazeLogInfo(fmt, ...)                                                  \
  _HazeLog(HAZE_LOG_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define HazeLogWarn(fmt, ...)                                                  \
  _HazeLog(HAZE_LOG_WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define HazeLogError(fmt, ...)                                                 \
  _HazeLog(HAZE_LOG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define HazeLogFatal(fmt, ...)                                                 \
  _HazeLog(HAZE_LOG_FATAL, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#endif
