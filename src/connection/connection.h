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

// Connection owns the socket.
void connection_setSocket(connection_t *connection, int socket) __attribute__((nonnull(1)));
int connection_getSocket(const connection_t *connection) __attribute__((nonnull(1)));

// Connection owns the source address.
void connection_setSourceAddress(connection_t *connection, string_t *sourceAddress) __attribute__((nonnull(1)));
string_t *connection_getSourceAddress(const connection_t *connection) __attribute__((nonnull(1)));

void connection_setSourcePort(connection_t *connection, uint16_t sourcePort) __attribute__((nonnull(1)));
uint16_t connection_getSourcePort(const connection_t *connection) __attribute__((nonnull(1)));

string_t *connection_read(const connection_t *connection, int timeout, size_t bytesToRead) __attribute__((nonnull(1)));
string_t *connection_readLine(const connection_t *connection, int timeout, size_t maxBytes) __attribute__((nonnull(1)));
bool connection_pollForData(const connection_t *connection, int timeout) __attribute__((nonnull(1)));
ssize_t connection_getAvailableBytes(const connection_t *connection) __attribute__((nonnull(1)));
size_t connection_readBytes(const connection_t *connection, char **buffer, size_t bytesToRead, int flags) __attribute__((nonnull(1, 2)));
size_t connection_readSSLBytes(const connection_t *connection, char **buffer, size_t bytesToRead, int flags) __attribute__((nonnull(1, 2)));
size_t connection_write(const connection_t *connection, const char *buffer, size_t bufferSize) __attribute__((nonnull(1, 2)));

// Check if the connection is for TLS (undefined behaviour if not called as the first read)
bool connection_isSSL(const connection_t *connection) __attribute__((nonnull(1)));

void connection_close(connection_t *connection) __attribute__((nonnull(1)));
void connection_free(connection_t *connection) __attribute__((nonnull(1)));

#endif
