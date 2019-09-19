#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>

#include "../logging/logging.h"

#include "connection.h"

// MSG_NOSIGNAL is in POSIX, but usually not defined on macOS
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

connection_t *connection_create() {
  connection_t *connection = malloc(sizeof(connection_t));
  if (connection == 0)
    return 0;
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

string_t *connection_read(connection_t *connection, int timeout) {
  // Set up structures necessary for polling
  struct pollfd descriptors[1];
  memset(descriptors, 0, sizeof(struct pollfd));
  descriptors[0].fd = connection->socketId;
  descriptors[0].events = POLLIN;

  // Wait for the connection to be ready to read
  int status = poll(descriptors, 1, timeout);
  if (status < -1) {
    log(LOG_ERROR, "Could not wait for connection to send data");
    return 0;
  } else if (status == 0) {
    log(LOG_ERROR, "The connection timed out");
    return 0;
  }

  log(LOG_DEBUG, "Successfully waited for the data to be readable");

  string_t *message = string_create();
  // Allocate some memory to start with for some speedup
  string_setBufferSize(message, 1024);

  // Read the entire message
  // TODO: Possible DoS attack here where we read the entirety of what is sent to us
  // The client may send data indefinitely
  while (true) {
    char character = 0;
    ssize_t bytesReceived = read(connection->socketId, &character, 1);
    if (bytesReceived == -1) {
      // If the request to read one byte would block, we've read everything
      int error = errno;
      if (error == EAGAIN || error == EWOULDBLOCK) {
        log(LOG_DEBUG, "Successfully received request of %zu bytes", string_getSize(message));
        return message;
      } else {
        log(LOG_ERROR, "Could not receive request from %s:%i", string_getBuffer(connection->sourceAddress), connection->sourcePort);
        string_free(message);
      }
    } else if (bytesReceived == 0) {
      log(LOG_DEBUG, "Successfully received request of %zu bytes", string_getSize(message));
      return message;
    }

    string_appendChar(message, character);
    log(LOG_DEBUG, "%c = %d", character, (int)character);
    if (character == 0) {
      log(LOG_DEBUG, "Successfully received request of %zu bytes", string_getSize(message));
      return message;
    }
  }
}

size_t connection_write(connection_t *connection, const char *buffer, size_t bufferSize) {
  // Use the flag MSG_NOSIGNAL to try to stop SIGPIPE on supported platforms
  ssize_t bytesSent = send(connection->socketId, buffer, bufferSize, MSG_NOSIGNAL);

  const char *sourceAddress = string_getBuffer(connection->sourceAddress);
  uint16_t sourcePort = connection->sourcePort;

  if (bytesSent == -1) {
    log(LOG_ERROR, "Could not write to %s:%i", sourceAddress, sourcePort);

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
