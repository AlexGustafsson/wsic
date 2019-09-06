#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
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
  // Waiting for the client to send ous a request
  int socketId =
      accept(initialSocketId, (struct sockaddr *)&hostAddress, &addressLength);

  if (socketId < 0) {
    log(LOG_ERROR, "Could not accept connection from %s:%i",
           inet_ntoa(hostAddress.sin_addr), ntohs(hostAddress.sin_port));
    return 0;
  }

  connection_t *connection = malloc(sizeof(connection_t));
  connection->request = 0;
  connection->requestLength = 0;
  connection->socketId = socketId;
  connection->sourcePort = hostAddress.sin_port;

  char *tempSourceAddress = inet_ntoa(hostAddress.sin_addr);
  connection->sourceAddress = malloc(sizeof(char) * (strlen(tempSourceAddress) + 1));
  strlcpy(connection->sourceAddress, tempSourceAddress, strlen(tempSourceAddress));

  log(LOG_DEBUG, "Successfully accepted connection from %s:%i",
         connection->sourceAddress, connection->sourcePort);

  return connection;
}

void request(connection_t *connection) {
  char *buffer = malloc(sizeof(char) * (REQUEST_BUFFER_SIZE + 1));
  memset(buffer, 0, REQUEST_BUFFER_SIZE + 1);
  connection->requestLength =
      recv(connection->socketId, buffer, REQUEST_BUFFER_SIZE, 0);
  buffer[REQUEST_BUFFER_SIZE] = 0;
  connection->request = buffer;

  if (connection->requestLength == 0) {
    log(LOG_ERROR, "Could not receive request from %s:%i", connection->sourceAddress, connection->sourcePort);
  } else {
    log(LOG_DEBUG, "Successfully resieved request:\n\n%s", buffer);
  }
}

void respons(connection_t *connection, const char *header, const char *body) {
  // Send respons to client
  if (send(connection->socketId, header, strlen(header), 0) < 0) {
    log(LOG_ERROR, "Could not send header to %s:%i", connection->sourceAddress, connection->sourcePort);
  } else {
    log(LOG_DEBUG, "Successfully sent header to %s:%i", connection->sourceAddress, connection->sourcePort);
  }
  if (send(connection->socketId, body, strlen(body), 0) < 0) {
    log(LOG_ERROR, "Could not send body to %s:%i", connection->sourceAddress, connection->sourcePort);
  } else {
    log(LOG_DEBUG, "Successfully sent body to %s:%i", connection->sourceAddress, connection->sourcePort);
  }
}

void closeConnection(connection_t *connection) {
  log(LOG_DEBUG, "Closed socket with id: %d\n\n", connection->socketId);
  close(connection->socketId);
  freeConnection(connection);
}

void freeConnection(connection_t *connection) {
  if (connection->request != 0)
    free(connection->request);
  if (connection->sourceAddress != 0)
    free(connection->sourceAddress);
  free(connection);
}
