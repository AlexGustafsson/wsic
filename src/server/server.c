#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <string.h>

#include "../cgi/cgi.h"
#include "../datastructures/hash-table/hash-table.h"
#include "../datastructures/list/list.h"
#include "../datastructures/queue/queue.h"
#include "../logging/logging.h"

#include "server.h"

static struct pollfd *socketDescriptors = 0;
static size_t socketDescriptorCount = 0;
static queue_t *connectionQueue = 0;

bool server_setNonBlocking(int socketDescriptor) {
  // Get the current flags set for the socket
  int flags = fcntl(socketDescriptor, F_GETFL, 0);
  if (flags == -1) {
    log(LOG_ERROR, "Unable to get current socket descriptor flags");
    return false;
  }
  // Set the socket to be non-blocking
  flags = fcntl(socketDescriptor, F_SETFL, flags | O_NONBLOCK);
  if (flags == -1) {
    log(LOG_ERROR, "Unable to set socket descriptor flags");
    return false;
  }

  return true;
}

pid_t server_createInstance(set_t *ports) {
  // Fork the process
  pid_t pid = fork();

  if (pid < 0) {
    log(LOG_ERROR, "Could not create server instance");
    return 0;
  } else if (pid == 0) {
    // Start a server in the child process (blocking call)
    int exitCode = server_start(ports);
    // We only get here if there's an error
    log(LOG_ERROR, "The server exited with code %d", exitCode);
    exit(exitCode);
  } else {
    log(LOG_DEBUG, "Started server instance with pid %d", pid);
    return pid;
  }
}

int server_start(set_t *ports) {
  // Set up file structures necessary for polling state of socket queues
  size_t descriptorsSize = sizeof(struct pollfd) * set_getLength(ports);
  socketDescriptors = malloc(descriptorsSize);
  memset(socketDescriptors, 0, descriptorsSize);

  connectionQueue = queue_create();
  if (connectionQueue == 0) {
    log(LOG_ERROR, "Could not create a connection queue");
    return EXIT_FAILURE;
  }

  // Set up listening sockets for each port
  for (size_t i = 0; i < list_getLength(ports); i++) {
    uint16_t port = (uint16_t)list_getValue(ports, i);
    log(LOG_DEBUG, "Setting up port %d for listening", port);
    int socketDescriptor = server_listen(port);
    if (socketDescriptor != 0) {
      socketDescriptors[i].fd = socketDescriptor;
      // Listen for incoming data
      socketDescriptors[i].events = POLLIN;
      socketDescriptorCount++;
    } else {
      log(LOG_ERROR, "Unable to make the server listen on port %d", port);
    }
  }

  if (socketDescriptorCount == 0) {
    log(LOG_ERROR, "No ports bound by the server, closing");
    free(socketDescriptors);
    return EXIT_FAILURE;
  }

  if (socketDescriptorCount != list_getLength(ports))
    log(LOG_WARNING, "Not all required ports could be successfully bound (%zu out of %zu)", socketDescriptorCount, list_getLength(ports));

  // Start accepting connections
  while (true) {
    int acceptedPorts = server_acceptConnections();
    // Try again if the attempt failed
    if (acceptedPorts == 0)
      continue;

    log(LOG_DEBUG, "There were %d port with incoming sockets (currently %zu sockets in total)", acceptedPorts, queue_getLength(connectionQueue));

    connection_t *connection = 0;
    while ((connection = queue_pop(connectionQueue)) != 0) {
      server_handleConnection(connection);
    }
  }

  free(socketDescriptors);
}

int server_listen(uint16_t port) {
  int socketDescriptor = socket(AF_INET, SOCK_STREAM, PROTOCOL);
  // Test to see if socket is created
  if (socketDescriptor < 0) {
    log(LOG_ERROR, "Could not create socket for port %d", port);
    return 0;
  }

  log(LOG_DEBUG, "Successfully created socket for port %d", port);

  int enableSocketReuse = 1;
  if (setsockopt(socketDescriptor, SOL_SOCKET, SO_REUSEADDR, &enableSocketReuse, sizeof(int)) < 0) {
    log(LOG_ERROR, "Could not make socket reuse address for port %d", port);
    return 0;
  }

  bool isNonBlocking = server_setNonBlocking(socketDescriptor);
  if (!isNonBlocking) {
    log(LOG_ERROR, "Failed to make listening socket non-blocking");
    return 0;
  }

  // Host address info
  struct sockaddr_in hostAddress;
  hostAddress.sin_family = AF_INET;
  hostAddress.sin_addr.s_addr = INADDR_ANY;
  hostAddress.sin_port = htons(port);

  // Bind socket to PORT
  ssize_t returnCodeBind = -1;
  returnCodeBind = bind(socketDescriptor, (struct sockaddr *)&hostAddress, sizeof(hostAddress));
  if (returnCodeBind < 0) {
    log(LOG_ERROR, "Could not bind 0.0.0.0:%d - code: %d", port, errno);
    return 0;
  }

  log(LOG_DEBUG, "Successfully bound 0.0.0.0:%d", port);

  // Listen to the socket
  if (listen(socketDescriptor, BACKLOG) < 0) {
    log(LOG_ERROR, "Could not listen on 0.0.0.0:%d", port);
    return 0;
  }

  log(LOG_INFO, "Listening to 0.0.0.0:%d", port);
  return socketDescriptor;
}

int server_acceptConnections() {
  // Wait indefinitely for any incoming socket
  int status = poll(socketDescriptors, socketDescriptorCount, -1);
  if (status <= 0) {
    log(LOG_ERROR, "An error occured while waiting for an incoming socket");
    return 0;
  }

  if (status == 1)
    log(LOG_DEBUG, "There's an incoming socket ready to be handled");
  else if (status > 1)
    log(LOG_DEBUG, "There are multiple incoming sockets ready to be handled");

  for (size_t i = 0; i < socketDescriptorCount; i++) {
    // Ignore sockets that don't have a connection
    if ((socketDescriptors[i].revents & POLLIN) == 0)
      continue;

    // Accept each waiting socket
    struct sockaddr_in peerAddress;
    socklen_t addressLength = sizeof(peerAddress);
    int socketId = -1;
    while ((socketId = accept(socketDescriptors[i].fd, (struct sockaddr *)&peerAddress, &addressLength)) != -1) {
      bool isNonBlocking = server_setNonBlocking(socketId);
      if (!isNonBlocking) {
        log(LOG_ERROR, "Unable to make socket non-blocking");
        close(socketId);
        continue;
      }

      // Add a connection to the queue
      connection_t *connection = connection_create();
      connection_setSocket(connection, socketId);
      connection_setSourcePort(connection, ntohs(peerAddress.sin_port));
      connection_setSourceAddress(connection, string_fromCopy(inet_ntoa(peerAddress.sin_addr)));
      queue_push(connectionQueue, connection);

      log(LOG_DEBUG, "Successfully accepted connection from %s:%i", string_getBuffer(connection->sourceAddress), connection->sourcePort);
    }
  }

  return status;
}

void server_handleConnection(connection_t *connection) {
  string_t *header = string_create();
  string_t *line = 0;
  while (true) {
    line = connection_readLine(connection, REQUEST_READ_TIMEOUT, REQUEST_MAX_SIZE);
    if (line == 0 || string_getSize(line) == 0)
      break;
    string_append(header, line);
    string_free(line);
    line = 0;
  }

  if (string_getSize(header) == 0) {
    log(LOG_ERROR, "Didn't receive a header from the request");
    string_free(header);
    server_closeConnection(connection);
    return;
  }

  // TODO: Parse HTTP headers here
  log(LOG_DEBUG, "Got headers:\n%s", string_getBuffer(header));

  // TODO: If the headers specify a body, read it here

  // TODO: If the headers specify an "Expect: " then we need to respond
  // (sockets are bidirectional and will stay open for a while)

  // Try with:
  // dd if=/dev/urandom of=test.dat bs=1M count=1
  // curl -v -X POST -d @test.dat localhost:3000

  server_closeConnection(connection);
  return;

  // Read a request (wait for maximum of one second)
  string_t *request = connection_read(connection, REQUEST_READ_TIMEOUT, REQUEST_MAX_SIZE);
  if (request == 0) {
    log(LOG_ERROR, "Could not get request from connection");
    server_closeConnection(connection);
    return;
  }

  log(LOG_DEBUG, "Got request");

  list_t *arguments = 0;
  hash_table_t *environment = hash_table_create();
  hash_table_setValue(environment, string_fromCopy("HTTPS"), string_fromCopy("off"));
  hash_table_setValue(environment, string_fromCopy("SERVER_SOFTWARE"), string_fromCopy("WSIC"));

  log(LOG_DEBUG, "Spawning CGI process");
  cgi_process_t *process = cgi_spawn("/Users/alexgustafsson/Documents/GitHub/wsic/cgi-test.sh", arguments, environment);
  log(LOG_DEBUG, "Spawned process with pid %d", process->pid);

  log(LOG_DEBUG, "Writing request to CGI process");
  cgi_write(process, string_getBuffer(request), string_getSize(request));
  // Make sure the process receives EOF
  cgi_flushStdin(process);

  log(LOG_DEBUG, "Reading response from CGI process");
  // TODO: Read more than 4096 bytes
  char buffer[4096] = {0};
  cgi_read(process, buffer, 4096);
  buffer[4096 - 1] = 0;

  log(LOG_DEBUG, "Got response from CGI process");
  connection_write(connection, buffer, 4096);

  // NOTE: Not necessary, but for debugging it's nice to know
  // that the process is actually exiting (not kept forever)
  // since we don't currently kill spawned processes
  /*log(LOG_DEBUG, "Waiting for process to exit");
  uint8_t exitCode = cgi_waitForExit(process);
  log(LOG_DEBUG, "Process exited with status %d", exitCode);*/

  cgi_freeProcess(process);
  server_closeConnection(connection);
}

void server_closeConnection(connection_t *connection) {
  log(LOG_DEBUG, "Closed socket with id %d", connection->socketId);
  close(connection->socketId);
  connection_free(connection);
}

void server_close() {
  log(LOG_INFO, "Closing server");
}
