#include <errno.h>
#include <poll.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <openssl/err.h>

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

void connection_setSocket(connection_t *connection, int socket) {
  connection->socket = socket;
}

int connection_getSocket(connection_t *connection) {
  return connection->socket;
}

void connection_setSSL(connection_t *connection, SSL *ssl) {
  connection->ssl = ssl;
}

void connection_setSourceAddress(connection_t *connection, string_t *sourceAddress) {
  connection->sourceAddress = sourceAddress;
}

string_t *connection_getSourceAddress(connection_t *connection) {
  return connection->sourceAddress;
}

void connection_setSourcePort(connection_t *connection, uint16_t sourcePort) {
  connection->sourcePort = sourcePort;
}

uint16_t connection_getSourcePort(connection_t *connection) {
  return connection->sourcePort;
}

// TODO:
// This will read indefinitely until timeout.
// A HTTP stream is bidirectional and will most likely stay open for a while
// Until it is closed, there's no real specification of how much data there is to read
// Therefore, try to read headers line by line and act on whether or not to read body bytes
// No body bytes specified? Tough luck - don't read.
// THis function is not fully implemented as of now.
string_t *connection_read(connection_t *connection, int timeout, size_t bytesToRead) {
  string_t *message = string_create();

  while (true) {
    bool dataIsAvailable = connection_pollForData(connection, timeout);
    if (!dataIsAvailable)
      continue;

    // Read the entire message
    while (true) {
      ssize_t bytesAvailable = connection_getAvailableBytes(connection);
      if (bytesAvailable == 0) {
        // Nothing to read, wait for next event as specified by the polling above
        break;
      } else if (bytesAvailable < 0) {
        // Check failed
        string_free(message);
        return 0;
      } else if (string_getSize(message) + (size_t)bytesAvailable > bytesToRead) {
        log(LOG_ERROR, "Too many bytes to read");
        string_free(message);
        return 0;
      }

      char *buffer = 0;
      size_t bytesReceived = connection_readBytes(connection, &buffer, bytesAvailable, READ_FLAGS_NONE);
      if (buffer != 0)
        string_appendBufferWithLength(message, buffer, bytesReceived);
      if (string_getSize(message) == bytesToRead)
        return message;
    }
  }
}

string_t *connection_readLine(connection_t *connection, int timeout, size_t maxBytes) {
  size_t offset = 0;
  uint8_t timeouts = 0;
  while (true) {
    bool dataIsAvailable = connection_pollForData(connection, timeout);
    if (timeouts++ > 5)
      return 0;
    if (!dataIsAvailable)
      continue;

    ssize_t bytesAvailable = connection_getAvailableBytes(connection);
    bytesAvailable = 1024;
    if (bytesAvailable == 0) {
      // Nothing to read, wait for next event as specified by the polling above
      continue;
    } else if (bytesAvailable < 0) {
      // Failed to get available bytes
      return 0;
    } else if ((size_t)bytesAvailable >= maxBytes) {
      // Don't process more than max bytes
      bytesAvailable = maxBytes;
    }

    // Read the request without consuming the content
    char *buffer = 0;
    size_t bytesReceived = 0;
    if (connection->ssl == 0)
      bytesReceived = connection_readBytes(connection, &buffer, bytesAvailable, READ_FLAGS_PEEK);
    else
      bytesReceived = connection_readSSLBytes(connection, &buffer, bytesAvailable, READ_FLAGS_PEEK);
    // Nothing read, wait for next event
    if (buffer == 0)
      continue;

    // Create a string from the buffer
    string_t *string = string_fromCopyWithLength(buffer, bytesReceived);
    free(buffer);
    buffer = 0;

    string_cursor_t *cursor = string_createCursor(string);
    // Set the offset to only look through new parts of the message
    string_setOffset(cursor, offset);
    // Try to find the end of a line
    ssize_t lineLength = string_findNextChar(cursor, '\n') + 1;
    offset = string_getOffset(cursor);
    string_freeCursor(cursor);
    string_free(string);

    if (lineLength < 1) {
      // No line found without looking for more than max bytes
      if ((size_t)bytesAvailable >= maxBytes)
        return 0;

      // There's no line available yet, wait for next event
      continue;
    }

    bytesReceived = 0;
    if (connection->ssl == 0)
      bytesReceived = connection_readBytes(connection, &buffer, lineLength, READ_FLAGS_NONE);
    else
      bytesReceived = connection_readSSLBytes(connection, &buffer, lineLength, READ_FLAGS_NONE);
    // Could not read message
    if (buffer == 0)
      return 0;

    string_t *line = string_fromCopyWithLength(buffer, bytesReceived);
    // Remove the trailing newlines
    string_trimEnd(line);
    return line;
  }
}

bool connection_pollForData(connection_t *connection, int timeout) {
  if (connection->ssl != 0 && SSL_pending(connection->ssl) > 0)
    return true;

  // Set up structures necessary for polling
  struct pollfd descriptors[1];
  memset(descriptors, 0, sizeof(struct pollfd));
  descriptors[0].fd = connection->socket;
  descriptors[0].events = POLLIN;

  log(LOG_DEBUG, "Waiting for data to be readable");

  // Wait for the connection to be ready to read
  int status = poll(descriptors, 1, timeout);
  if (status < -1) {
    log(LOG_ERROR, "Could not wait for connection to send data");
    return false;
  } else if (status == 0) {
    log(LOG_ERROR, "The connection timed out");
    return false;
  }

  return true;
}

ssize_t connection_getAvailableBytes(connection_t *connection) {
  // Get the number of bytes immediately available for reading
  int bytesAvailable = -1;
  if (connection->ssl == 0) {
    if (ioctl(connection->socket, FIONREAD, &bytesAvailable) == -1) {
      if (errno == EBADF) {
        log(LOG_ERROR, "Could not check number of bytes available for reading. The socket had closed");
      } else {
        const char *reason = strerror(errno);
        log(LOG_ERROR, "Could not check number of bytes available for reading. Got code %d (%s)", errno, reason);
      }
      return -1;
    }

    log(LOG_DEBUG, "There are %d bytes available for reading", bytesAvailable);
  } else {
    bytesAvailable = SSL_pending(connection->ssl);
    if (bytesAvailable < 0) {
      log(LOG_ERROR, "Failed to get number of available bytes from TLS connection");
      return -1;
    }

    log(LOG_DEBUG, "There are %d bytes available for reading from TLS connection", bytesAvailable);
  }

  return (ssize_t)bytesAvailable;
}

size_t connection_readBytes(connection_t *connection, char **buffer, size_t bytesToRead, int flags) {
  (*buffer) = malloc(sizeof(char) * bytesToRead);
  ssize_t bytesReceived = recv(connection->socket, (*buffer), bytesToRead, flags);
  if (bytesReceived == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      log(LOG_ERROR, "Reading %zu bytes would block", bytesToRead);
    } else if (errno == EBADF) {
      log(LOG_ERROR, "Could not read bytes from %s:%i - the connection is closed", string_getBuffer(connection->sourceAddress), connection->sourcePort);
    } else {
      const char *reason = strerror(errno);
      log(LOG_ERROR, "Could not read bytes from %s:%i. Got code %d (%s)", string_getBuffer(connection->sourceAddress), connection->sourcePort, errno, reason);
    }

    free(*buffer);
    *buffer = 0;
    return 0;
  } else if (bytesReceived == 0) {
    log(LOG_DEBUG, "No bytes read");
    free(*buffer);
    *buffer = 0;
    return 0;
  }

  log(LOG_DEBUG, "Read %zu bytes", bytesReceived);
  return bytesReceived;
}

size_t connection_readSSLBytes(connection_t *connection, char **buffer, size_t bytesToRead, int flags) {
  (*buffer) = malloc(sizeof(char) * bytesToRead);
  size_t bytesReceived = 0;
  if (flags == READ_FLAGS_PEEK) {
    ERR_clear_error();
    if (SSL_peek_ex(connection->ssl, (*buffer), bytesToRead, &bytesReceived) <= 0) {
      log(LOG_ERROR, "Could not read bytes from %s:%i (TLS)", string_getBuffer(connection->sourceAddress), connection->sourcePort);
      free(*buffer);
      *buffer = 0;
      return 0;
    }

    log(LOG_DEBUG, "Peeked %zu bytes (TLS)", bytesReceived);
  } else {
    ERR_clear_error();
    if (SSL_read_ex(connection->ssl, (*buffer), bytesToRead, &bytesReceived) <= 0) {
      log(LOG_ERROR, "Could not read bytes from %s:%i (TLS)", string_getBuffer(connection->sourceAddress), connection->sourcePort);
      free(*buffer);
      *buffer = 0;
      return 0;
    }

    log(LOG_DEBUG, "Read %zu bytes (TLS)", bytesReceived);
  }

  return bytesReceived;
}

size_t connection_write(connection_t *connection, const char *buffer, size_t bufferSize) {
  const char *sourceAddress = string_getBuffer(connection->sourceAddress);
  uint16_t sourcePort = connection->sourcePort;

  ssize_t bytesSent = -1;
  if (connection->ssl == 0) {
    // Use the flag MSG_NOSIGNAL to try to stop SIGPIPE on supported platforms (there is a signal handler catching other cases)
    ssize_t bytesSent = send(connection->socket, buffer, strlen(buffer), MSG_NOSIGNAL);
    if (bytesSent == -1) {
      if (errno == EBADF) {
        log(LOG_ERROR, "Could not write to %s:%i. The connection had already closed", sourceAddress, sourcePort);
      } else {
        const char *reason = strerror(errno);
        log(LOG_ERROR, "Could not write to %s:%i. Got error %d (%s)", sourceAddress, sourcePort, errno, reason);
      }
      return 0;
    }
  } else {
    ERR_clear_error();
    if (SSL_write_ex(connection->ssl, buffer, strlen(buffer), (size_t *)&bytesSent) < 0) {
      log(LOG_ERROR, "Could not write to %s:%i (TLS)", sourceAddress, sourcePort);
      return 0;
    }
  }

  if (bytesSent != -1)
    log(LOG_DEBUG, "Successfully wrote %zu (out of %zu) bytes to %s:%i", bytesSent, bufferSize, sourceAddress, sourcePort);
  return bytesSent;
}

void connection_parseRequest(connection_t *connection, char *buffer, size_t bufferSize) {
  log(LOG_WARNING, "Not implemented");
}

void connection_close(connection_t *connection) {
  if (connection->ssl != 0)
    SSL_free(connection->ssl);

  if (shutdown(connection->socket, SHUT_RDWR) == -1) {
    if (errno != ENOTCONN && errno != EINVAL) {
      if (errno == ENOTSOCK || errno == EBADF) {
        log(LOG_ERROR, "Failed to shutdown connection. It was likely already closed");
      } else {
        const char *reason = strerror(errno);
        log(LOG_ERROR, "Failed to shutdown connection. Got error %d (%s)", errno, reason);
      }
    }
  } else {
    if (close(connection->socket) == -1) {
      const char *reason = strerror(errno);
      log(LOG_ERROR, "Unable to close connection. Got error %d (%s)", errno, reason);
    }
  }
}

// Detect if TLS was used
bool connection_isSSL(connection_t *connection) {
  char *buffer = 0;
  connection_readBytes(connection, &buffer, 6, READ_FLAGS_PEEK);
  if (buffer == 0)
    return 0;

  bool isHandshakeRecord = buffer[0] == 0x16;
  bool isClientHello = buffer[5] == 0x01;

  return isHandshakeRecord && isClientHello;
}

void connection_free(connection_t *connection) {
  connection_close(connection);
  if (connection->sourceAddress != 0)
    string_free(connection->sourceAddress);
  free(connection);
}
