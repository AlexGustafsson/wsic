#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#include "../logging/logging.h"

#include "connection.h"

connection_t *createConnection() {
  connection_t *connection = malloc(sizeof(connection_t));
  memset(connection, 0, sizeof(connection_t));

  return connection;
}

void setConnectionSocket(connection_t *connection, int socketId) {
  connection->socketId = socketId;
}

void setSourceAddress(connection_t *connection, const char *sourceAddress) {
  size_t addressLength = strlen(sourceAddress);
  connection->sourceAddress = malloc(sizeof(char) * (addressLength + 1));
  strlcpy(connection->sourceAddress, sourceAddress, addressLength);
}

void setSourcePort(connection_t *connection, uint16_t sourcePort) {
  connection->sourcePort = sourcePort;
}

size_t readFromConnection(connection_t *connection, char *buffer,
                          size_t bufferSize) {
  ssize_t bytesReceived = read(connection->socketId, buffer, bufferSize);
  if (bytesReceived == -1) {
    log(LOG_ERROR, "Could not receive request from %s:%i",
        connection->sourceAddress, connection->sourcePort);
    return 0;
  }

  log(LOG_DEBUG, "Successfully received request");
  return bytesReceived;
}

size_t writeToConnection(connection_t *connection, const char *buffer,
                         size_t bufferSize) {
  ssize_t bytesSent = write(connection->socketId, buffer, bufferSize);
  if (bytesSent == -1) {
    log(LOG_ERROR, "Could not write to %s:%i", connection->sourceAddress,
        connection->sourcePort);
    return 0;
  }

  log(LOG_DEBUG, "Successfully wrote %d bytes to %s:%i", bufferSize, connection->sourceAddress,
      connection->sourcePort);
  return bytesSent;
}

void parseConnectionRequest(connection_t *connection, char *buffer,
                            size_t bufferSize) {
  log(LOG_WARNING, "Not implemented");
}

void freeConnection(connection_t *connection) {
  if (connection->sourceAddress != 0)
    free(connection->sourceAddress);
  free(connection);
}
