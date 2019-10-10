#ifndef SERVER_H
#define SERVER_H

#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>

#include "../connection/connection.h"
#include "../datastructures/set/set.h"

/* The protocol specifies a particular protocol to be used with the
       socket.  Normally only a single protocol exists to support a
       particular socket type within a given protocol family, in which case
       protocol can be specified as 0
*/
#define PROTOCOL 0

#define SERVER_EXIT_FATAL 10

pid_t server_createInstance(const set_t *ports);
// Main entrypoint for a server instance
int server_start(const set_t *ports);
// Start listening on a port. Returns the listening socket or 0 if failed
int server_listen(uint16_t port, size_t backlog);
// Block until at least one of the bound ports receives a request. Returns the number of sockets to handle (0 if failed)
int server_acceptConnections();
SSL *server_handleSSL(connection_t *connection);
void server_closeConnection(connection_t *connection);

void server_close();
#endif
