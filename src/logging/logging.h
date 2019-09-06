#ifndef LOGGING_H
#define LOGGING_H

#include <stdint.h>
#include <stdio.h>
#include <syslog.h>
#include <stdlib.h>

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

// A "private" macro for formatting and logging to console
#define _log_console(level, ...) do {fprintf(stderr, "\x1b[90m[\x1b[%dm%s\x1b[90m][%s@%d][%s]\x1b[0m ", LOG_COLOR_##level, LOG_LABEL_##level, __FILE__, __LINE__, __func__); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n");} while(0)
// Log to all enabled outputs
#define log(level, ...) do {if ((LOGGING_OUTPUT & LOGGING_CONSOLE) > 0) {_log_console(level, __VA_ARGS__);} if ((LOGGING_OUTPUT & LOGGING_SYSLOG) > 0) {syslog(LOG_NOTICE, __VA_ARGS__);}} while(0)

extern uint8_t LOGGING_OUTPUT;

// Automatically start logging
__attribute__((constructor)) void startLogging();
// Automatically deconstruct logging
__attribute__((destructor)) void stopLogging();

#endif
