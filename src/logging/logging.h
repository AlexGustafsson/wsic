#ifndef LOGGING_H
#define LOGGING_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <stdint.h>

#include "../string/string.h"
#include "../http/http.h"
#include "../time/time.h"

// Define clean code comptaitble aliases for syslog's constants
#define LOG_EMERGENCY LOG_EMERG
#define LOG_CRITICAL LOG_CRIT
#define LOG_ERROR LOG_ERR

// Label for each log level
#define LOG_LABEL_0 "EMERGENCY"
#define LOG_LABEL_1 "ALERT"
#define LOG_LABEL_2 "CRITICAL"
#define LOG_LABEL_3 "ERROR"
#define LOG_LABEL_4 "WARNING"
#define LOG_LABEL_5 "NOTICE"
#define LOG_LABEL_6 "INFO"
#define LOG_LABEL_7 "DEBUG"

// ANSI color codes to use for console logging
#define LOG_COLOR_RED 31
#define LOG_COLOR_ORANGE 33
#define LOG_COLOR_GREEN 32
#define LOG_COLOR_BLUE 34

// Color to use for each log level
#define LOG_COLOR_0 LOG_COLOR_RED
#define LOG_COLOR_1 LOG_COLOR_RED
#define LOG_COLOR_2 LOG_COLOR_RED
#define LOG_COLOR_3 LOG_COLOR_RED
#define LOG_COLOR_4 LOG_COLOR_ORANGE
#define LOG_COLOR_5 LOG_COLOR_GREEN
#define LOG_COLOR_6 LOG_COLOR_GREEN
#define LOG_COLOR_7 LOG_COLOR_BLUE

// Define constants for support logging types (bitmasks)
#define LOGGING_CONSOLE 1
#define LOGGING_SYSLOG 2

#define _logging_logToFile(filePointer, level, ...) logging_logToFile(filePointer, LOG_LABEL_##level, LOG_COLOR_##level, __FILE__, __LINE__, __func__, __VA_ARGS__)

// Log to all enabled outputs
#define log(level, ...)                           \
  do {                                            \
    if (level > LOGGING_LEVEL)                    \
      break;                                      \
                                                  \
    if ((LOGGING_OUTPUT & LOGGING_CONSOLE) > 0)   \
      _logging_logToFile(stderr, level, __VA_ARGS__);    \
                                                  \
    if ((LOGGING_OUTPUT & LOGGING_SYSLOG) > 0)    \
      syslog(level, __VA_ARGS__);                 \
                                                  \
    if (LOGGING_OUTPUT_FILE != 0) {               \
      _logging_logToFile(LOGGING_OUTPUT_FILE, level, __VA_ARGS__);    \
    }                                             \
                                                  \
  } while (0)

// Log only the inputs (and a newline) to all enabled outputs
#define logRaw(level, ...)                        \
  do {                                            \
    if (level > LOGGING_LEVEL)                    \
      break;                                      \
                                                  \
    if ((LOGGING_OUTPUT & LOGGING_CONSOLE) > 0) { \
      fprintf(stdout, __VA_ARGS__);               \
      fprintf(stdout, "\n");                      \
    }                                             \
                                                  \
    if ((LOGGING_OUTPUT & LOGGING_SYSLOG) > 0)    \
      syslog(level, __VA_ARGS__);                 \
                                                  \
    if (LOGGING_OUTPUT_FILE != 0) {               \
      fprintf(LOGGING_OUTPUT_FILE, __VA_ARGS__);  \
      fprintf(LOGGING_OUTPUT_FILE, "\n");         \
    }                                             \
  } while (0)

extern uint8_t LOGGING_OUTPUT;
extern uint8_t LOGGING_LEVEL;
extern FILE *LOGGING_OUTPUT_FILE;

// Start logging (should always be called from the main process as soon as possible)
bool logging_start();
// Deconstruct logging (should always be closed right before exit)
void logging_stop();
// A general function that logs to specified file, can be both a path and stderr
void logging_logToFile(FILE *filePointer, const char *label, int color, const char *file, int line, const char *function, const char *format, ...);
// Log the request in format CLF
void logging_request(string_t *remoteHost, enum httpMethod method, string_t *path, string_t *version, uint16_t responseCode, size_t bytesSent);

#endif
