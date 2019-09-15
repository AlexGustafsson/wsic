#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

#include "../cgi/cgi.h"
#include "../datastructures/hash-table/hash-table.h"
#include "../datastructures/list/list.h"
#include "../logging/logging.h"

#include "server.h"

static int initialSocketId;
static struct sockaddr_in hostAddress;

pid_t server_createInstance(int port) {
  // Fork the process
  pid_t pid = fork();

  if (pid < 0) {
    log(LOG_ERROR, "Could not create server instance on port %d", port);
    return 0;
  } else if (pid == 0) {
    // Start a server in the child process (blocking call)
    int exitCode = server_start(port);
    // We only get here if there's an error
    log(LOG_ERROR, "The server exited with code %d", exitCode);
    exit(exitCode);
  } else {
    log(LOG_DEBUG, "Started server instance with pid %d", pid);
    return pid;
  }
}

int server_start(int port) {
  bool bound = server_listen(port);
  if (!bound) {
    log(LOG_ERROR, "Unable to start the server - socket binding failed");
    return EXIT_FAILURE;
  }

  while (true) {
    connection_t *connection = server_acceptConnection();
    string_t *request = connection_read(connection, 1024);
    log(LOG_DEBUG, "Got request:\n %s", string_getBuffer(request));

    list_t *arguments = 0;
    hash_table_t *environment = hash_table_create();
    hash_table_setValue(environment, string_fromCopy("HTTPS"), string_fromCopy("off"));
    hash_table_setValue(environment, string_fromCopy("SERVER_SOFTWARE"), string_fromCopy("WSIC"));

    log(LOG_DEBUG, "Spawning CGI process");
    cgi_process_t *process = cgi_spawn("/Users/alexgustafsson/Documents/GitHub/wsic/cgi-test.sh", arguments, environment);
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
    server_closeConnection(connection);
  }
}

bool server_listen(int port) {
  initialSocketId = socket(AF_INET, SOCK_STREAM, PROTOCOL);
  // Test to see if socket is created
  if (initialSocketId < 0) {
    log(LOG_ERROR, "Could not create listening socket");
    return false;
  }

  log(LOG_DEBUG, "Successfully created listening socket");

  int enableSocketReuse = 1;
  if (setsockopt(initialSocketId, SOL_SOCKET, SO_REUSEADDR, &enableSocketReuse, sizeof(int)) < 0) {
    log(LOG_ERROR, "Could not make socket reuse address");
    return false;
  }

  // Host address info
  hostAddress.sin_family = AF_INET;
  hostAddress.sin_addr.s_addr = INADDR_ANY;
  hostAddress.sin_port = htons(port);

  // Bind socket to PORT
  ssize_t returnCodeBind = -1;
  returnCodeBind = bind(initialSocketId, (struct sockaddr *)&hostAddress, sizeof(hostAddress));
  if (returnCodeBind < 0) {
    log(LOG_ERROR, "Could not bind listening socket to 0.0.0.0:%d - code: %d", port, errno);
    return false;
  }

  log(LOG_DEBUG, "Successfully bound listening socket to 0.0.0.0:%d", port);

  // Listen to the socket
  if (listen(initialSocketId, BACKLOG) < 0) {
    log(LOG_ERROR, "Could not start server on 0.0.0.0:%d", port);
    return false;
  }

  log(LOG_INFO, "Listening to 0.0.0.0:%d", port);
  return true;
}

connection_t *server_acceptConnection() {
  socklen_t addressLength = sizeof(hostAddress);
  // Waiting for the client to send us a request
  int socketId =
      accept(initialSocketId, (struct sockaddr *)&hostAddress, &addressLength);

  if (socketId < 0) {
    log(LOG_ERROR, "Could not accept connection from %s:%i", inet_ntoa(hostAddress.sin_addr), ntohs(hostAddress.sin_port));
    return 0;
  }

  connection_t *connection = connection_create();
  connection_setSocket(connection, socketId);
  connection_setSourcePort(connection, ntohs(hostAddress.sin_port));
  connection_setSourceAddress(connection, string_fromCopy(inet_ntoa(hostAddress.sin_addr)));

  log(LOG_DEBUG, "Successfully accepted connection from %s:%i", string_getBuffer(connection->sourceAddress), connection->sourcePort);

  return connection;
}

void server_closeConnection(connection_t *connection) {
  log(LOG_DEBUG, "Closed socket with id %d", connection->socketId);
  close(connection->socketId);
  connection_free(connection);
}

void server_close() {
  log(LOG_INFO, "Closing server");
}
