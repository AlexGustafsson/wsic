#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdint.h>

#define REQUEST_BUFFER_SIZE 1024

typedef struct {
  int socketId;
  char *request;
  size_t requestLength;
  char *sourceAddress;
  uint16_t sourcePort;
} connection_t;

void serverListen(int port);
connection_t *acceptConnection();
void request(connection_t *connection);
void respons(connection_t *connection, const char *header, const char *body);
void closeConnection(connection_t *connection);
void freeConnection(connection_t *connection);

#endif /* SERVER_H */
