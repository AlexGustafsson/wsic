#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#include "../logging/logging.h"

#include "server.h"

static bool server_isRunning = false;

static int initialSocketId;
static struct sockaddr_in hostAddress;

void server_start(int port) {
  initialSocketId = socket(AF_INET, SOCK_STREAM, PROTOCOL);
  // Test to see if socket is created
  if (initialSocketId < 0) {
    log(LOG_ERROR, "Could not create listening socket");
  } else {
    log(LOG_DEBUG, "Successfully created listening socket");
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
  } else {
    log(LOG_DEBUG, "Successfully bound listening socket to 0.0.0.0:%d", port);
  }
  if (returnCodeBind < 0)
    return;

  // Listen to the socket
  if (listen(initialSocketId, BACKLOG) < 0) {
    log(LOG_ERROR, "Could not start server on 0.0.0.0:%d", port);
    return;
  }

  log(LOG_INFO, "Listening to 0.0.0.0:%d", port);
  server_isRunning = true;
}


bool server_getIsRunning() {
  return server_isRunning;
}

connection_t *acceptConnection() {
  socklen_t addressLength = sizeof(hostAddress);
  // Waiting for the client to send us a request
  int socketId =
      accept(initialSocketId, (struct sockaddr *)&hostAddress, &addressLength);

  if (socketId < 0) {
    log(LOG_ERROR, "Could not accept connection from %s:%i",
        inet_ntoa(hostAddress.sin_addr), ntohs(hostAddress.sin_port));
    return 0;
  }

  connection_t *connection = createConnection();
  setConnectionSocket(connection, socketId);
  setSourcePort(connection, ntohs(hostAddress.sin_port));
  setSourceAddress(connection, inet_ntoa(hostAddress.sin_addr));

  log(LOG_DEBUG, "Successfully accepted connection from %s:%i",
      connection->sourceAddress, connection->sourcePort);

  return connection;
}

void closeConnection(connection_t *connection) {
  log(LOG_DEBUG, "Closed socket with id %d", connection->socketId);
  close(connection->socketId);
  freeConnection(connection);
}

void server_close() {
  log(LOG_INFO, "Closing server");
  server_isRunning = false;
}
