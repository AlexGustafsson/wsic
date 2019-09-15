#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "datastructures/hash-table/hash-table.h"
#include "datastructures/list/list.h"

#include "cgi/cgi.h"
#include "compile-time-defines.h"
#include "config/config.h"
#include "daemon/daemon.h"
#include "http/http.h"
#include "logging/logging.h"
#include "resources/resources.h"
#include "server/server.h"
#include "string/string.h"
#include "www/www.h"

#include "main.h"

int main(int argc, char const *argv[]) {
  // Warn if the server is running as root
  if (geteuid() == 0)
    log(LOG_WARNING, "Running as root. I hope you know what you're doing.");

  config_t *config = config_parse((char *)RESOURCES_CONFIG_DEFAULT_CONFIG_TOML);
  server_config_t *defaultServerConfig = config_getServerConfig(config, 0);

  bool showHelp = false;
  bool showVersion = false;

  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0) {
      showHelp = true;
    } else if (strcmp(argv[i], "-v") == 0) {
      showVersion = true;
    } else if (strcmp(argv[i], "-d") == 0) {
      config_setIsDaemon(config, 1);
    } else if (strcmp(argv[i], "-p") == 0) {
      config_setPort(defaultServerConfig, strtol(argv[++i], 0, 10));
    } else if (strcmp(argv[i], "-l") == 0) {
      string_t *logfile = string_fromCopy(argv[++i]);
      config_setLogfile(defaultServerConfig, logfile);
    } else if (strcmp(argv[i], "-s") == 0) {
      const char *mode = argv[++i];
      if (strcmp(mode, "fork")) {
        config_setParallelMode(config, PARALLEL_MODE_FORK);
      } else if (strcmp(mode, "pre-fork")) {
        config_setParallelMode(config, PARALLEL_MODE_FORK);
      } else {
        log(LOG_ERROR, "Unsupported parallel mode '%s'", mode);
        config_free(config);
        return 3;
      }
    }
  }

  if (showHelp) {
    printHelp();
    config_free(config);
    return 0;
  } else if (showVersion) {
    printVersion();
    config_free(config);
    return 0;
  }

  if (config_getIsDaemon(config)) {
    // Daemonize the server
    pid_t sid = daemonize();
    if (sid < 0) {
      log(LOG_ERROR, "Unable to daemonize the server");
      return EXIT_FAILURE;
    }

    // Log the process id
    log(LOG_INFO, "Successfully deamonized the server. PID: %d", sid);

    // Disable console logging (as there'll be no TTY)
    LOGGING_OUTPUT &= ~(0xFF << LOGGING_CONSOLE);
    // Enable syslog logging
    LOGGING_OUTPUT |= LOGGING_SYSLOG;

    // Close open file descriptiors related to a TTY session
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
  }

  list_t *ports = list_create();
  // Extract unique ports used by the configuration
  for (size_t i = 0; i < list_getLength(config->serverConfigs); i++) {
    server_config_t *serverConfig = config_getServerConfig(config, i);
    if (list_findIndex(ports, (void *)serverConfig->port) < 0)
      list_addValue(ports, (void *)serverConfig->port);
  }

  list_t *pids = list_create();
  // Spawn a server instance for each port
  for (size_t i = 0; i < list_getLength(ports); i++) {
    int16_t port = (int16_t)list_getValue(ports, i);

    pid_t pid = server_createInstance(port);
    if (pid != 0)
      list_addValue(pids, (void *)pid);
  }

  // Setup signal handling
  signal(SIGINT, handleSignalSIGINT);
  signal(SIGTERM, handleSignalSIGTERM);
  signal(SIGKILL, handleSignalSIGKILL);
  signal(SIGPIPE, handleSignalSIGPIPE);
  // If a child exits, it will interrupt the sleep and check statuses directly
  signal(SIGCHLD, handleSignalSIGCHLD);

  // Put the main process into a polling sleep
  while (true) {
    // If a child exits, it will interrupt the sleep and check statuses directly, sleep for 10 seconds
    sleep(10);
    for (size_t i = 0; i < list_getLength(pids); i++) {
      int16_t port = (int16_t)list_getValue(ports, i);
      pid_t pid = (pid_t)list_getValue(pids, i);

      int status;
      if (waitpid(pid, &status, WNOHANG) != 0) {
        int exitCode = WEXITSTATUS(status);
        log(LOG_WARNING, "Server instance has exited with code %d", exitCode);
        log(LOG_DEBUG, "Recreating server instance for port %d", port);
        pid_t newPid = server_createInstance(port);
        if (newPid != 0)
          list_setValue(pids, i, (void *)newPid);
      }
    }
  }

  config_free(config);
  return 0;
}

void printHelp() {
  printf("wsic ( \xF0\x9D\x9C\xB6 ) - A Web Server written in C\n");
  printf("\n");
  printf("\x1b[1mVERSION\x1b[0m\n");
  printf("wsic %s\n", WSIC_VERSION);
  printf("\n");
  printf("\x1b[1mUSAGE\x1b[0m\n");
  printf("$ wsic [arguments]\n");
  printf("\n");
  printf("\x1b[1mARGUMENTS\x1b[0m\n");
  printf("-h\t--help\t\t\tShow this help text\n");
  printf("-v\t--version\t\tShow current version\n");
  printf("-p port\t--port port\t\tSpecify the port to listen on. Defaults to 80\n");
  printf("-d\t--daemon\t\tRun the server as a deamon\n");
  printf("-l\t--logfile\t\tSpecify the logfile to write logs to\n");
  printf("-s mode\t--parallel-mode mode\tSpecify the parallel mode to use where mode is one of fork, thread, pre-fork or mux\n");
}

void printVersion() {
  printf("wsic ( \xF0\x9D\x9C\xB6 ) - A Web Server written in C\n");
  printf("\n");
  printf("\x1b[1mVERSION\x1b[0m\n");
  printf("wsic %s\n", WSIC_VERSION);
  printf("compiled %s with %s\n", COMPILE_TIME, COMPILER_VERSION);
  printf("\n");
  printf("\x1b[1mOPEN SOURCE\x1b[0m\n");
  printf("The source code is hosted on https://github.com/AlexGustafsson/wsic/\n");
}

// Handle SIGINT (CTRL + C)
void handleSignalSIGINT(int signalNumber) {
  log(LOG_INFO, "Got SIGINT - exiting cleanly");
  server_close();
  exit(EXIT_SUCCESS);
}

// Handle SIGTERM (kill etc.)
void handleSignalSIGTERM(int signalNumber) {
  log(LOG_INFO, "Got SIGTERM - exiting cleanly");
  server_close();
  exit(EXIT_SUCCESS);
}

// Handle SIGKILL (unblockable - just used for logging)
void handleSignalSIGKILL(int signalNumber) {
  log(LOG_WARNING, "Got SIGKILL - exiting immediately");
  exit(EXIT_SUCCESS);
}

// Handle SIGPIPE (the other end of a pipe broke it)
void handleSignalSIGPIPE(int signalNumber) {
  // Log and ignore
  log(LOG_WARNING, "Got SIGPIPE - broken pipe");
}

// Handle SIGCHLD (a child exited)
void handleSignalSIGCHLD(int signalNumber) {
  // Log and ignore
  log(LOG_WARNING, "Got SIGCHLD - child exited");
}
