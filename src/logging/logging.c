#include <pthread.h>
#include <stdarg.h>
#include <time.h>

#include "logging.h"

// Enable the console logger by default
uint8_t LOGGING_OUTPUT = LOGGING_CONSOLE;

// Specifies which levels to output to log
uint8_t LOGGING_LEVEL = LOG_DEBUG;

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

  string_t *time = time_getFormattedTime();

  pthread_mutex_lock(&mutex);
  // Print log level, path to file where the program currently are, which line the program is at and last the function that was called
  fprintf(filePointer, "\x1b[90m[%s][\x1b[%dm%s\x1b[90m][%s@%d][%s]\n    └──\x1b[0m ", string_getBuffer(time), color, label, file, line, function);

  // Print the text and arguments that commes from the log(log_lvl, "%d %s %c", a, b, c)
  va_list arguments;
  va_start(arguments, format);
  vfprintf(filePointer, format, arguments);
  va_end(arguments);

  fprintf(filePointer, "\n");
  pthread_mutex_unlock(&mutex);
  
  string_free(time);
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
