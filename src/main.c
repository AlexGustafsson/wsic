#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "compile-time-defines.h"
#include "config/config.h"
#include "daemon/daemon.h"
#include "http/http.h"
#include "logging/logging.h"
#include "resources/resources.h"
#include "server/server.h"
#include "string/string.h"
#include "www/www.h"
#include "cgi/cgi.h"

#include "main.h"

int main(int argc, char const *argv[]) {
  signal(SIGINT, handleSignalSIGINT);
  signal(SIGTERM, handleSignalSIGTERM);
  signal(SIGKILL, handleSignalSIGKILL);
  signal(SIGPIPE, handleSignalSIGPIPE);
  signal(SIGCHLD, handleSignalSIGCHLD);

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

  server_start(config_getPort(defaultServerConfig));
  if (!server_getIsRunning()) {
    log(LOG_ERROR, "Could not start the server");
    return EXIT_FAILURE;
  }

  while (true) {
    connection_t *connection = acceptConnection();
    string_t *request = connection_read(connection, 1024);
    log(LOG_DEBUG, "Got request:\n %s", string_getBuffer(request));

    log(LOG_DEBUG, "Spawning CGI process");
    cgi_process_t *process = cgi_spawn("/Users/alexgustafsson/Documents/GitHub/wsic/cgi-test.sh", 0, 0);
    log(LOG_DEBUG, "Spawned process with pid %d", process->pid);

    log(LOG_DEBUG, "Writing request to CGI process");
    cgi_write(process, string_getBuffer(request), string_getSize(request));
    // Make sure the process receives EOF
    cgi_flushStdin(process);

    log(LOG_DEBUG, "Reading response from CGI process");
    char buffer[4096] = {0};
    cgi_read(process, buffer, 4096);
    buffer[4096 - 1] = 0;

    log(LOG_DEBUG, "Got response: \n%s", buffer);
    connection_write(connection, buffer, 4096);

    // NOTE: Not necessary, but for debugging it's nice to know
    // that the process is actually exiting (not kept forever)
    // since we don't currently kill spawned processes
    log(LOG_DEBUG, "Waiting for process to exit");
    uint8_t exitCode = cgi_waitForExit(process);
    log(LOG_DEBUG, "Process exited with status %d", exitCode);

    cgi_freeProcess(process);
    closeConnection(connection);
    string_free(request);
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
