#include <time.h>
#include <stdarg.h>

#include "logging.h"

// Enable the console logger by default
uint8_t LOGGING_OUTPUT = LOGGING_CONSOLE;

void logging_startSyslog() {
  // Log debug and up (handling of level should be done elsewhere)
  setlogmask(LOG_UPTO(LOG_DEBUG));
  // Open a syslog named after the application. Log the PID and connect to the user's log
  openlog("wsic", LOG_PID, LOG_USER);
}

void logging_stopSyslog() {
  closelog();
}

void logging_logConsole(const char* label, int color, const char* file, int line, const char* function, const char* format, ...) {
  uint64_t rest = 0;
  uint64_t milliseconds = 0;
  uint64_t seconds = 0;
  uint64_t minutes = 0;
  uint64_t hours = 0;
  uint64_t days = 0;

  uint64_t nanosecondsSinceStart = 0;
  uint64_t secondsSinceStart = 0;
  time_getTimeSinceStart(&nanosecondsSinceStart, &secondsSinceStart);

  time_formatTime(nanosecondsSinceStart, secondsSinceStart, &milliseconds, &seconds, &minutes, &hours, &days, &rest);
  if (days != 0) {
    fprintf(stderr, "\x1b[90m[%llud %lluh %llum %llus %llu.%llums]", (uint64_t)days, (uint64_t)hours, (uint64_t)minutes, (uint64_t)seconds, (uint64_t)milliseconds, rest);
  } else if (hours != 0){
    fprintf(stderr, "\x1b[90m[%lluh %llum %llus %llu.%llums]", (uint64_t)hours, (uint64_t)minutes, (uint64_t)seconds, (uint64_t)milliseconds, rest);
  } else if (minutes != 0) {
    fprintf(stderr, "\x1b[90m[%llum %llus %llu.%llums]", (uint64_t)minutes, (uint64_t)seconds, (uint64_t)milliseconds, rest);
  } else if (seconds != 0) {
    fprintf(stderr, "\x1b[90m[%llus %llu.%llums]", (uint64_t)seconds, (uint64_t)milliseconds, rest);
  } else {
    fprintf(stderr, "\x1b[90m[%llu.%llums]", (uint64_t)milliseconds, rest);
  }
  fprintf(stderr, "[\x1b[%dm%s\x1b[90m][%s@%d][%s]\n    └──\x1b[0m ", color, label, file, line, function);

  va_list arguments;
  va_start(arguments, format);
  vfprintf(stderr, format, arguments);
  va_end(arguments);

  fprintf(stderr, "\n");
}

void logging_request(string_t *remoteHost, enum httpMethod method, string_t *path, string_t *version, uint16_t responseCode, size_t bytesSent) {
  // Get current time
  time_t rawTime = time(NULL);
  // Convert time to local time
  struct tm *timeInfo = localtime(&rawTime);

  char timeBuffer[29];
  // Append to buffer the timeformat we want (CLF)
  // The day is 2 characters, the month 3, the year 4, hours 2, minute 2, second 2 and time zone 4
  // This, space character brackets, etc. and null adds up to 29 characters
  strftime(timeBuffer, 29, "[%d/%b/%Y:%H:%M:%S %z]", timeInfo);

  string_t *methodString = http_methodToString(method);
  logRaw(LOG_NOTICE, "%s - - %s \"%s %s HTTP/%s\" %d %zu", remoteHost == 0 ? "-" : string_getBuffer(remoteHost), timeBuffer, methodString == 0 ? "-" : string_getBuffer(methodString), path == 0 ? "-" : string_getBuffer(path), version == 0 ? "-" : string_getBuffer(version), responseCode, bytesSent);

  if (methodString != 0)
    string_free(methodString);
}
