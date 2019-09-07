#ifndef SERVER_H
#define SERVER_H

#include "../connection/connection.h"

#define REQUEST_BUFFER_SIZE 1024

void serverListen(int port);
connection_t *acceptConnection();
void closeConnection(connection_t *connection);

// Automatically deconstruct server
__attribute__((destructor)) void closeServer();
#endif
