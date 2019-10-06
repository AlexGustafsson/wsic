#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "datastructures/list/list.h"
#include "datastructures/set/set.h"

#include "compile-time-defines.h"
#include "config/config.h"
#include "daemon/daemon.h"
#include "logging/logging.h"
#include "resources/resources.h"
#include "server/server.h"
#include "string/string.h"
#include "time/time.h"

#include "main.h"

bool main_serverShouldRun = true;
pid_t main_serverInstance = 0;

int main(int argc, char const *argv[]) {
  // Start internal time keeping
  time_reset();

  // Warn if the server is running as root
  if (geteuid() == 0)
    log(LOG_WARNING, "Running as root. I hope you know what you're doing.");

  if (argc == 1) {
    log(LOG_ERROR, "Expected a command");
    main_printHelp();
    return EXIT_FAILURE;
  }

  const char *command = argv[1];

  if (strcmp(command, "help") == 0) {
    main_printHelp();
    return EXIT_SUCCESS;
  } else if (strcmp(command, "version") == 0) {
    main_printVersion();
    return EXIT_SUCCESS;
  } else if (strcmp(command, "start") != 0) {
    log(LOG_ERROR, "Unexpected command '%s'", command);
    return EXIT_FAILURE;
  }

  // Parse arguments
  bool argumentParsingFailed = false;
  int8_t daemon = -1;
  string_t *logfile = 0;
  string_t *configFilePath = 0;
  for (int i = 2; i < argc; i++) {
    if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--daemon") == 0) {
      daemon = 1;
    } else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--logfile") == 0) {
      logfile = string_fromCopy(argv[++i]);
      if (logfile == 0) {
        log(LOG_ERROR, "Unable to set logfile");
        argumentParsingFailed = true;
        break;
      }
    } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--config") == 0) {
      configFilePath = string_fromCopy(argv[++i]);
      if (configFilePath == 0) {
        log(LOG_ERROR, "Unable to set config file");
        argumentParsingFailed = true;
        break;
      }
    }
  }

  if (argumentParsingFailed) {
    if (logfile != 0)
      string_free(logfile);
    if (configFilePath != 0)
      string_free(configFilePath);
  }

  // Parse config - either from specified file or from default
  config_t *config = 0;
  if (configFilePath == 0) {
    log(LOG_WARNING, "No config file specified - using default config");
    config = config_parse((const char *)RESOURCES_CONFIG_DEFAULT_CONFIG_TOML);
  } else {
    string_t *configFile = resources_loadFile(configFilePath);
    if (configFile != 0) {
      config = config_parse(string_getBuffer(configFile));
      string_free(configFile);
    }
    string_free(configFilePath);
  }

  if (config == 0) {
    log(LOG_ERROR, "Failed to setup config file");
    return EXIT_FAILURE;
  }

  // Add parsed arguments to the config
  if (logfile != 0)
    config_setLogfile(config, logfile);
  if (daemon == 1)
    config_setIsDaemon(config, daemon);
  config_setGlobalConfig(config);
  logging_startSyslog();

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

    // Close open file descriptors related to a TTY session
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
  }

  set_t *ports = set_create();
  // Extract unique ports used by the configuration
  for (size_t i = 0; i < list_getLength(config->serverConfigs); i++) {
    server_config_t *serverConfig = config_getServerConfig(config, i);
    set_addValue(ports, (void *)serverConfig->port);
  }

  main_serverInstance = server_createInstance(ports);
  if (main_serverInstance == 0) {
    log(LOG_ERROR, "Unable to create server instance");
    exit(EXIT_FAILURE);
  }

  // Setup signal handling for main process
  signal(SIGINT, handleSignalSIGINT);
  signal(SIGTERM, handleSignalSIGTERM);
  signal(SIGKILL, handleSignalSIGKILL);
  // If a child exits, it will interrupt the sleep and check statuses directly
  signal(SIGCHLD, handleSignalSIGCHLD);

  // Put the main process into a polling sleep
  while (main_serverShouldRun) {
    // If a child exits, or another signal is caught it will interrupt the sleep
    sleep(10);
    int status;
    if (waitpid(main_serverInstance, &status, WNOHANG) != 0) {
      bool exited = WIFEXITED(status);
      log(LOG_DEBUG, "Got interrupted by a signal or sleep timeout, exited: %d", exited);
      if (exited && main_serverShouldRun) {
        // If the server should be running but it exited, restart it
        int exitCode = WEXITSTATUS(status);
        if (exitCode == SERVER_EXIT_FATAL) {
          log(LOG_DEBUG, "Got a fatal exit code from instance, quitting");
          exit(1);
        }

        log(LOG_WARNING, "Server instance has exited with code %d. Restarting", exitCode);
        pid_t newInstance = server_createInstance(ports);
        if (newInstance != 0)
          main_serverInstance = newInstance;
      }
    }
  }

  // Note that the SIGINT will be received by the worker process as well, killing it automatically
  log(LOG_DEBUG, "Waiting for child processes to exit");
  pid_t pid = 0;
  while ((pid = wait(0)) > 0)
    ;

  list_free(ports);

  log(LOG_DEBUG, "Freeing global config");
  config_freeGlobalConfig();

  log(LOG_DEBUG, "Closing syslog");
  logging_stopSyslog();

  log(LOG_DEBUG, "Exiting from main");
  return 0;
}

void main_printHelp() {
  printf("wsic ( \xF0\x9D\x9C\xB6 ) - A Web Server written in C\n");
  printf("\n");
  printf("\x1b[1mVERSION\x1b[0m\n");
  printf("wsic %s\n", WSIC_VERSION);
  printf("\n");
  printf("\x1b[1mUSAGE\x1b[0m\n");
  printf("$ wsic <command> [arguments]\n");
  printf("\n");
  printf("\x1b[1mCOMMANDS\x1b[0m\n");
  printf("start\t\tStart WSIC\n");
  printf("help\t\tShow this help text\n");
  printf("version\t\tShow current version\n");
  printf("\n");
  printf("\x1b[1mARGUMENTS\x1b[0m\n");
  printf("-d\t--daemon\t\tRun the server as a deamon\n");
  printf("-l\t--logfile\t\tSpecify the logfile to write logs to\n");
}

void main_printVersion() {
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
  // Disable handling of SIGCHILD
  signal(SIGCHLD, main_emptySignalHandler);

  log(LOG_INFO, "Got SIGINT - exiting cleanly");
  // Send signal to server
  kill(main_serverInstance, SIGINT);
  main_serverShouldRun = false;
}

// Handle SIGTERM (kill etc.)
void handleSignalSIGTERM(int signalNumber) {
  // Disable handling of SIGCHILD
  signal(SIGCHLD, main_emptySignalHandler);

  log(LOG_INFO, "Got SIGTERM - exiting cleanly");
  // Send signal to server
  kill(main_serverInstance, SIGINT);
  main_serverShouldRun = false;
}

// Handle SIGKILL (unblockable - just used for logging)
void handleSignalSIGKILL(int signalNumber) {
  // Disable other, conflicting signals
  signal(SIGINT, main_emptySignalHandler);
  signal(SIGTERM, main_emptySignalHandler);
  signal(SIGCHLD, main_emptySignalHandler);

  log(LOG_WARNING, "Got SIGKILL - exiting immediately");
  exit(EXIT_SUCCESS);
}

// Handle SIGCHLD (a child exited)
void handleSignalSIGCHLD(int signalNumber) {
  // Log and ignore
  log(LOG_WARNING, "Got SIGCHLD - child exited");
}

void main_emptySignalHandler() {
  // Do nothing
}
