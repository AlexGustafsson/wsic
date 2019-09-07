#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "compile-time-defines.h"
#include "logging/logging.h"
#include "server/server.h"
#include "www/www.h"

#include "main.h"

int main(int argc, char const *argv[]) {
  bool showHelp = false;
  bool showVersion = false;
  uint16_t port = 80;
  bool runAsDaemon = false;
  const char *logfile = 0;
  const char *parallelMode = 0;

  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0) {
      showHelp = true;
    } else if (strcmp(argv[i], "-v") == 0) {
      showVersion = true;
    } else if (strcmp(argv[i], "-d") == 0) {
      runAsDaemon = true;
    } else if (strcmp(argv[i], "-p") == 0) {
      port = strtol(argv[++i], 0, 10);
    } else if (strcmp(argv[i], "-l") == 0) {
      logfile = argv[++i];
    } else if (strcmp(argv[i], "-s") == 0) {
      parallelMode = argv[++i];
    }
  }

  if (showHelp) {
    printHelp();
    return 0;
  } else if (showVersion) {
    printVersion();
    return 0;
  }

  if (!serverListen(port)) {
    log(LOG_ERROR, "Could not start the server");
    return 1;
  }

  while (true) {
    connection_t *connection = acceptConnection();
    char requestBuffer[1024] = {0};
    readFromConnection(connection, requestBuffer, 1024);

    writeToConnection(connection, "HTTP/1.1 200 OK\nContent-Type: text/html\n\n", 41);
    page_t *page = createPage404("/index.html");
    writeToConnection(connection, page->source, page->sourceLength);
    freePage(page);
    closeConnection(connection);
  }

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
