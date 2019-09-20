#ifndef SERVER_H
#define SERVER_H

#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>

#include "../connection/connection.h"
#include "../datastructures/set/set.h"

// The backlog argument defines the maximum length to which the queue of pending connections for sockfd may grow.
#define BACKLOG 10
/* The protocol specifies a particular protocol to be used with the
       socket.  Normally only a single protocol exists to support a
       particular socket type within a given protocol family, in which case
       protocol can be specified as 0
*/
#define PROTOCOL 0
// Don't allow requests larger than 1 MB
#define REQUEST_MAX_SIZE 1048576
// Don't allow connections to wait for more than one second without sending data when reading
#define REQUEST_READ_TIMEOUT 1000

pid_t server_createInstance(set_t *ports);
// Main entrypoint for a server instance
int server_start();
// Start listening on a port. Returns the listening socket or 0 if failed
int server_listen(uint16_t port);
// Block until at least one of the bound ports receives a request. Returns the number of sockets to handle (0 if failed)
int server_acceptConnections();
void server_handleConnection(connection_t *connection);
void server_closeConnection(connection_t *connection);

void server_close();
#endif
