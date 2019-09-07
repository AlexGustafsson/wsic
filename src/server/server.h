#ifndef SERVER_H
#define SERVER_H

#include <stdbool.h>

#include "../connection/connection.h"

// The backlog argument defines the maximum length to which the queue of pending connections for sockfd may grow.
#define BACKLOG 10
/* The protocol specifies a particular protocol to be used with the
       socket.  Normally only a single protocol exists to support a
       particular socket type within a given protocol family, in which case
       protocol can be specified as 0
*/
#define PROTOCOL 0
#define REQUEST_BUFFER_SIZE 1024

bool serverListen(int port);
connection_t *acceptConnection();
void closeConnection(connection_t *connection);

// Automatically deconstruct server
__attribute__((destructor)) void closeServer();
#endif
