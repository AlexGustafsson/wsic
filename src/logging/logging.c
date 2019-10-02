#include <time.h>

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
  logRaw(LOG_NOTICE, "%s - - %s \"%s %s HTTP/%s\" %d %zu", string_getBuffer(remoteHost), timeBuffer, string_getBuffer(methodString), string_getBuffer(path), string_getBuffer(version), responseCode, bytesSent);
  free(timeBuffer);
  string_free(methodString);
}
