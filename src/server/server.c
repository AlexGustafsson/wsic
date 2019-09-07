#include <arpa/inet.h>
#include <unistd.h>

#include "../logging/logging.h"

#include "server.h"

static int initialSocketId;
static struct sockaddr_in hostAddress;

void serverListen(int port) {
  initialSocketId = socket(AF_INET, SOCK_STREAM, 0);
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
  if (bind(initialSocketId, (struct sockaddr *)&hostAddress,
           sizeof(hostAddress)) < 0) {
    log(LOG_ERROR, "Could not bind listening socket to 0.0.0.0:%d", port);
  } else {
    log(LOG_DEBUG, "Successfully bound listening socket to 0.0.0.0:%d", port);
  }

  // Listen to the socket
  if (listen(initialSocketId, 10) < 0) {
    log(LOG_ERROR, "Could not start server on 0.0.0.0:%d", port);
  } else {
    log(LOG_INFO, "Listening to 0.0.0.0:%d", port);
  }
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
  log(LOG_DEBUG, "Closed socket with id: %d\n\n", connection->socketId);
  close(connection->socketId);
  freeConnection(connection);
}

void closeServer() {
  log(LOG_INFO, "Closing server");
}
