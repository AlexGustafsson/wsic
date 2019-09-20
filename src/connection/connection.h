#ifndef CONNECTION_H
#define CONNECTION_H

#include <stdint.h>

#include "../string/string.h"

typedef struct {
  int socketId;
  string_t *sourceAddress;
  uint16_t sourcePort;
} connection_t;

connection_t *connection_create();
void connection_parseRequest(connection_t *connection, char *buffer, size_t bufferSize);

void connection_setSocket(connection_t *connection, int socketId);
void connection_setSourceAddress(connection_t *connection, string_t *sourceAddress);
void connection_setSourcePort(connection_t *connection, uint16_t sourcePort);

string_t *connection_read(connection_t *connection, int timeout, size_t maxBytes);
size_t connection_write(connection_t *connection, const char *buffer, size_t bufferSize);

void connection_free(connection_t *connection);

#endif
