#include <pthread.h>
#include <stdarg.h>
#include <time.h>

#include "logging.h"

// Enable the console logger by default
uint8_t LOGGING_OUTPUT = LOGGING_CONSOLE;

// Specifies which levels to output to log
uint8_t LOGGING_LEVEL = LOG_NOTICE;

// The specified file for output
FILE *LOGGING_OUTPUT_FILE = 0;

pthread_mutex_t mutex;

bool logging_start() {
  // Create mutex
  if (pthread_mutex_init(&mutex, NULL) != 0) {
    log(LOG_ERROR, "Failed to create mutex for console log");
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

  if (LOGGING_OUTPUT_FILE != 0)
    fclose(LOGGING_OUTPUT_FILE);
}

bool logging_openOutputFile(const char *filePath) {
  LOGGING_OUTPUT_FILE = fopen(filePath, "a");

  return LOGGING_OUTPUT_FILE == 0;
}

void logging_logToFile(FILE *filePointer, const char *label, int color, const char *file, int line, const char *function, const char *format, ...) {
  // Elapsed time
  uint64_t days = 0;
  uint64_t hours = 0;
  uint64_t minutes = 0;
  uint64_t seconds = 0;
  uint64_t milliseconds = 0;
  uint64_t nanoseconds = 0;
  uint64_t secondsSinceStart = 0;
  uint64_t nanosecondsSinceStart = 0;
  time_getTimeSinceStart(&nanosecondsSinceStart, &secondsSinceStart);
  time_getFormattedElapsedTime(nanosecondsSinceStart, secondsSinceStart, &nanoseconds, &milliseconds, &seconds, &minutes, &hours, &days);

  // Current time
  time_t calendarNow = time(NULL);
  struct tm timeInfo;
  localtime_r(&calendarNow, &timeInfo);

  pthread_mutex_lock(&mutex);
  fprintf(filePointer, "\x1b[90m[%02d/%02d/%04d %02d:%02d:0%d]", timeInfo.tm_mday, timeInfo.tm_mon, timeInfo.tm_year, timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);

  if (days != 0)
    fprintf(filePointer, "%llud ", (unsigned long long)days);
  if (hours != 0)
    fprintf(filePointer, "%lluh ", (unsigned long long)hours);
  if (minutes != 0)
    fprintf(filePointer, "%llum ", (unsigned long long)minutes);
  if (seconds != 0)
    fprintf(filePointer, "%llus ", (unsigned long long)seconds);

  fprintf(filePointer, "[%llu.%llums][\x1b[%dm%s\x1b[90m][%s@%d][%s]\n    └──\x1b[0m ", (unsigned long long)milliseconds, (unsigned long long)nanoseconds, color, label, file, line, function);
  // Print the text and arguments that commes from the log(log_lvl, "%d %s %c", a, b, c)
  va_list arguments;
  va_start(arguments, format);
  vfprintf(filePointer, format, arguments);
  va_end(arguments);

  fprintf(filePointer, "\n");
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
