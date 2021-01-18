#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdarg.h>

/* LOG_HEAD is the first part of debug messages */
#ifndef LOG_HEAD
#define LOG_HEAD "APP"
#endif

#define MODULE_LOG_ENABLE 1

void init_logger(void);


/**
 * Log Debug
 * Params: Like printf
 * Ret: None
 */
static void LOGD(const char* fmt, ...){
#ifdef MODULE_LOG_ENABLE
  va_list args;
  va_start (args, fmt);
  printf(" - D %s ", LOG_HEAD);
  vprintf(fmt, args);
  va_end(args);
  printf("\n");
#endif
}

/**
 * Log Error
 * Params: Like printf
 * Ret: None
 */
static void LOGE(const char* fmt, ...){
#ifdef MODULE_LOG_ENABLE
  va_list args;
  va_start (args, fmt);
  printf(" - E %s ", LOG_HEAD);
  vprintf(fmt, args);
  va_end(args);
  printf("\n");
#endif
}

#endif


