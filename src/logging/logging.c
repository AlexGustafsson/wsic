#include <time.h>
#include <stdarg.h>
#include <pthread.h>

#include "logging.h"

// Enable the console logger by default
uint8_t LOGGING_OUTPUT = LOGGING_CONSOLE;

// Specifies which levels to output to log
uint8_t LOGGING_LEVEL = LOG_DEBUG;

pthread_mutex_t mutex;

bool logging_start() {
  // Create mutex. Mutex will
  if (pthread_mutex_init(&mutex, NULL) != 0) {
    log(LOG_ERROR, "Failed to create mutex for log");
    return false;
  }
  // Log debug and up (handling of level should be done elsewhere)
  setlogmask(LOG_UPTO(LOG_DEBUG));
  // Open a syslog named after the application. Log the PID and connect to the user's log
  openlog("wsic", LOG_PID, LOG_USER);

  return true;
}

void logging_stop() {
  closelog();
  pthread_mutex_unlock(&mutex);
}

void logging_logConsole(const char* label, int color, const char* file, int line, const char* function, const char* format, ...) {
  uint64_t nanoseconds = 0;
  uint64_t milliseconds = 0;
  uint64_t seconds = 0;
  uint64_t minutes = 0;
  uint64_t hours = 0;
  uint64_t days = 0;

  uint64_t nanosecondsSinceStart = 0;
  uint64_t secondsSinceStart = 0;
  time_getTimeSinceStart(&nanosecondsSinceStart, &secondsSinceStart);

  // Print time
  time_formatTime(nanosecondsSinceStart, secondsSinceStart, &nanoseconds, &milliseconds, &seconds, &minutes, &hours, &days);

  pthread_mutex_lock(&mutex);
  if (days != 0) {
    fprintf(stderr, "\x1b[90m[%llud %lluh %llum %llus %llu.%llums]", days, hours, minutes, seconds, milliseconds, nanoseconds);
  } else if (hours != 0){
    fprintf(stderr, "\x1b[90m[%lluh %llum %llus %llu.%llums]", hours, minutes, seconds, milliseconds, nanoseconds);
  } else if (minutes != 0) {
    fprintf(stderr, "\x1b[90m[%llum %llus %llu.%llums]", minutes, seconds, milliseconds, nanoseconds);
  } else if (seconds != 0) {
    fprintf(stderr, "\x1b[90m[%llus %llu.%llums]", seconds, milliseconds, nanoseconds);
  } else {
    fprintf(stderr, "\x1b[90m[%llu.%llums]", milliseconds, nanoseconds);
  }
  // Print log level, path to file where the program currently are, which line the program is at and last the function that was called
  fprintf(stderr, "[\x1b[%dm%s\x1b[90m][%s@%d][%s]\n    └──\x1b[0m ", color, label, file, line, function);

  // Print the text and arguments that commes from the log(log_lvl, "%d %s %c", a, b, c)
  va_list arguments;
  va_start(arguments, format);
  vfprintf(stderr, format, arguments);
  va_end(arguments);

  fprintf(stderr, "\n");
  pthread_mutex_unlock(&mutex);
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
