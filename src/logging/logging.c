#include "logging.h"

// Enable the console logger by default
uint8_t LOGGING_OUTPUT = LOGGING_CONSOLE;

void startLogging() {
  // Log notices and up
  setlogmask(LOG_UPTO(LOG_NOTICE));
  // Open a syslog named after the application. Log the PID and connect to the user's log
  openlog("wsic", LOG_PID, LOG_USER);
}

void stopLogging() {
  // Stop syslog if it is turned on
  if ((LOGGING_OUTPUT & LOGGING_SYSLOG) > 0)
    closelog();
}
