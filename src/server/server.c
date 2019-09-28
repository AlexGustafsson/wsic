#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <string.h>

#include "../cgi/cgi.h"
#include "../datastructures/hash-table/hash-table.h"
#include "../datastructures/list/list.h"
#include "../datastructures/message-queue/message-queue.h"
#include "../http/http.h"
#include "../logging/logging.h"
#include "../worker/worker.h"

#include "server.h"

static struct pollfd *socketDescriptors = 0;
static size_t socketDescriptorCount = 0;
static message_queue_t *connectionQueue = 0;

static worker_t *workerPool[WORKER_POOL_SIZE];

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
  fflush(stdout);
  fflush(stderr);
  pid_t pid = fork();

  if (pid < 0) {
    log(LOG_ERROR, "Could not create server instance");
    return 0;
  } else if (pid == 0) {
    // Start a server in the child process (blocking call)
    int exitCode = server_start(ports);
    // We only get here if there's an error
    log(LOG_ERROR, "The server exited with code %d", exitCode);
    _exit(exitCode);
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

  connectionQueue = message_queue_create();
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
    return SERVER_EXIT_FATAL;
  }

  if (socketDescriptorCount != list_getLength(ports))
    log(LOG_WARNING, "Not all required ports could be successfully bound (%zu out of %zu)", socketDescriptorCount, list_getLength(ports));

  // TODO: https://stackoverflow.com/questions/70773/pthread-cond-wait-versus-semaphore
  // https://www.justsoftwaresolutions.co.uk/threading/implementing-a-thread-safe-queue-using-condition-variables.html
  // Implementd cond wait-based queue (think message queue) with waiting consumers and one producer

  // Setup worker pool
  log(LOG_DEBUG, "Setting up %d workers in the pool", WORKER_POOL_SIZE);
  for (uint8_t i = 0; i < WORKER_POOL_SIZE; i++) {
    worker_t *worker = worker_spawn(i, 0, connectionQueue);
    if (worker == 0) {
      log(LOG_ERROR, "Failed to set up worker for the pool");
      return SERVER_EXIT_FATAL;
    }

    workerPool[i] = worker;
  }
  log(LOG_DEBUG, "Set up %d workers", WORKER_POOL_SIZE);

  // Start accepting connections
  while (true) {
    int acceptedPorts = server_acceptConnections();
    log(LOG_DEBUG, "There were %d port with incoming sockets", acceptedPorts);

    // TODO: We can handle scaling of number of workers here
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
    const char *reason = strerror(errno);
    log(LOG_ERROR, "An error occured while waiting for an incoming socket: %d (%s)", errno, reason);
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
      message_queue_push(connectionQueue, connection);

      log(LOG_DEBUG, "Successfully accepted connection from %s:%i", string_getBuffer(connection->sourceAddress), connection->sourcePort);
    }
  }

  return status;
}

void server_close() {
  log(LOG_INFO, "Closing server");
}
