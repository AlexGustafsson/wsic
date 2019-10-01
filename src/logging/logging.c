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
  log(LOG_DEBUG, "Successfully logged request in format CLF to syslog");

  // Get current time
  time_t rawTime = time(NULL);
  // Convert time to local time
  struct tm *timeInfo = localtime(&rawTime);

  char timeBuffer[80];
  // Append to buffer the timeformat we want (CLF)
  strftime(timeBuffer, 80, "[%d/%b/%Y:%H:%M:%S %z]", timeInfo);

  logRaw(LOG_NOTICE, "%s - - %s \"%s %s HTTP/%s\" %d %zu", string_getBuffer(remoteHost), timeBuffer, string_getBuffer(http_methodToString(method)), string_getBuffer(path), string_getBuffer(version), responseCode, bytesSent);
}
