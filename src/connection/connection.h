#ifndef CONNECTION_H
#define CONNECTION_H

#include <stdlib.h>

typedef struct {
  int socketId;
  char *sourceAddress;
  uint16_t sourcePort;
} connection_t;

connection_t *createConnection();
void parseConnectionRequest(connection_t *connection, char *buffer, size_t bufferSize);

void setConnectionSocket(connection_t *connection, int socketId);
void setConnectionRequest(connection_t *connection, const char *request);
void setSourceAddress(connection_t *connection, const char *sourceAddress);
void setSourcePort(connection_t *connection, uint16_t sourcePort);

size_t readFromConnection(connection_t *connection, char *buffer, size_t bufferSize);
size_t writeToConnection(connection_t *connection, const char* buffer, size_t bufferSize);

void freeConnection(connection_t *connection);

#endif
