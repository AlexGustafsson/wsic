#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#include "../logging/logging.h"

#include "connection.h"

connection_t *connection_create() {
  connection_t *connection = malloc(sizeof(connection_t));
  memset(connection, 0, sizeof(connection_t));

  return connection;
}

void connection_setSocket(connection_t *connection, int socketId) {
  connection->socketId = socketId;
}

void connection_setSourceAddress(connection_t *connection, string_t *sourceAddress) {
  connection->sourceAddress = sourceAddress;
}

void connection_setSourcePort(connection_t *connection, uint16_t sourcePort) {
  connection->sourcePort = sourcePort;
}

string_t *connection_read(connection_t *connection, size_t bytes) {
  char *buffer = malloc(sizeof(char) * bytes);
  if (buffer == 0)
    return 0;

  ssize_t bytesReceived = read(connection->socketId, buffer, bytes);
  if (bytesReceived == -1) {
    log(LOG_ERROR, "Could not receive request from %s:%i", string_getBuffer(connection->sourceAddress), connection->sourcePort);

    return 0;
  }

  log(LOG_DEBUG, "Successfully received request");
  return string_fromCopyWithLength(buffer, bytesReceived);
}

size_t connection_write(connection_t *connection, const char *buffer, size_t bufferSize) {
  ssize_t bytesSent = write(connection->socketId, buffer, bufferSize);

  const char *sourceAddress = string_getBuffer(connection->sourceAddress);
  uint16_t sourcePort = connection->sourcePort;

  if (bytesSent == -1) {
    log(LOG_ERROR, "Could not write to %s:%i", string_getBuffer(connection->sourceAddress), connection->sourcePort);

    return 0;
  }

  log(LOG_DEBUG, "Successfully wrote %zu bytes to %s:%i", bufferSize, sourceAddress, sourcePort);
  return bytesSent;
}

void connection_parseRequest(connection_t *connection, char *buffer, size_t bufferSize) {
  log(LOG_WARNING, "Not implemented");
}

void connection_free(connection_t *connection) {
  if (connection->sourceAddress != 0)
    string_free(connection->sourceAddress);
  free(connection);
}
