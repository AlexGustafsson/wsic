#ifndef CONNECTION_H
#define CONNECTION_H

#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <openssl/ssl.h>

#include "../string/string.h"

#define READ_FLAGS_NONE 0
#define READ_FLAGS_PEEK MSG_PEEK

typedef struct {
  SSL *ssl;
  int socket;
  string_t *sourceAddress;
  uint16_t sourcePort;
} connection_t;

connection_t *connection_create();
void connection_parseRequest(connection_t *connection, char *buffer, size_t bufferSize);

void connection_setSocket(connection_t *connection, int socket);
int connection_getSocket(connection_t *connection);

void connection_setSourceAddress(connection_t *connection, string_t *sourceAddress);
string_t *connection_getSourceAddress(connection_t *connection);

void connection_setSourcePort(connection_t *connection, uint16_t sourcePort);
uint16_t connection_getSourcePort(connection_t *connection);

string_t *connection_read(connection_t *connection, int timeout, size_t bytesToRead);
string_t *connection_readLine(connection_t *connection, int timeout, size_t maxBytes);
bool connection_pollForData(connection_t *connection, int timeout);
ssize_t connection_getAvailableBytes(connection_t *connection);
size_t connection_readBytes(connection_t *connection, char **buffer, size_t bytesToRead, int flags);
size_t connection_readSSLBytes(connection_t *connection, char **buffer, size_t bytesToRead, int flags);
size_t connection_write(connection_t *connection, const char *buffer, size_t bufferSize);

// Check if the connection is for TLS (undefined behaviour if not called as the first read)
bool connection_isSSL(connection_t *connection);

void connection_close(connection_t *connection);
void connection_free(connection_t *connection);

#endif
