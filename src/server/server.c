#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <string.h>

#include <openssl/ssl.h>

#include "../cgi/cgi.h"
#include "../datastructures/hash-table/hash-table.h"
#include "../datastructures/list/list.h"
#include "../datastructures/message-queue/message-queue.h"
#include "../http/http.h"
#include "../logging/logging.h"
#include "../worker/worker.h"
#include "../config/config.h"

#include "server.h"

static struct pollfd *socketDescriptors = 0;
static size_t socketDescriptorCount = 0;
static message_queue_t *server_connectionQueue = 0;

static worker_t *workerPool[WORKER_POOL_SIZE];

void server_handleSignalSIGPIPE();
int server_handleServerNameIdentification(SSL *context, int *al, void *arg);

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
  // Set up signals
  signal(SIGPIPE, server_handleSignalSIGPIPE);

  // Set up file structures necessary for polling state of socket queues
  size_t descriptorsSize = sizeof(struct pollfd) * set_getLength(ports);
  socketDescriptors = malloc(descriptorsSize);
  memset(socketDescriptors, 0, descriptorsSize);

  server_connectionQueue = message_queue_create();
  if (server_connectionQueue == 0) {
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

  // Setup worker pool
  log(LOG_DEBUG, "Setting up %d workers in the pool", WORKER_POOL_SIZE);
  for (uint8_t i = 0; i < WORKER_POOL_SIZE; i++) {
    worker_t *worker = worker_spawn(i, 0, server_connectionQueue);
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
    int socket = -1;
    while ((socket = accept(socketDescriptors[i].fd, (struct sockaddr *)&peerAddress, &addressLength)) != -1) {
      bool isNonBlocking = server_setNonBlocking(socket);
      if (!isNonBlocking) {
        log(LOG_ERROR, "Unable to make socket non-blocking");
        close(socket);
        continue;
      }

      // Setup the connection
      connection_t *connection = connection_create();
      if (connection == 0) {
        log(LOG_ERROR, "Failed to create connection");
        continue;
      }
      connection_setSocket(connection, socket);
      connection_setSourcePort(connection, ntohs(peerAddress.sin_port));
      connection_setSourceAddress(connection, string_fromCopy(inet_ntoa(peerAddress.sin_addr)));

      log(LOG_DEBUG, "Setting up connection for %s:%i", string_getBuffer(connection->sourceAddress), connection->sourcePort);

      // TODO: Polling slows things down
      connection_pollForData(connection, 100);
      bool isSSL = connection_isSSL(connection);
      if (isSSL) {
        log(LOG_DEBUG, "Handling TLS setup for %s:%d", string_getBuffer(connection_getSourceAddress(connection)), connection_getSourcePort(connection));
        connection->ssl = server_handleSSL(connection);
        if (connection->ssl == 0) {
          log(LOG_DEBUG, "Failed to setup TLS");
          connection_free(connection);
          continue;
        }

        log(LOG_DEBUG, "Successfully setup TLS for connection");
      }

      // Add the connection to the worker pool
      message_queue_push(server_connectionQueue, connection);
    }
  }

  return status;
}

int server_handleServerNameIdentification(SSL *ssl, int *alert, void *arg) {
  // TODO: This always returns 0 even though SNI is set
  // TODO: Set alert appropriately when returning 0
  const char *rawDomain = SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name);
  if (rawDomain == 0) {
    log(LOG_ERROR, "No SNI set for client connection");
    return SSL_TLSEXT_ERR_NOACK;
  } else {
    log(LOG_DEBUG, "SNI for TLS socket is '%s'", rawDomain);
  }

  config_t *config = config_getGlobalConfig();
  string_t *domain = string_fromCopy(rawDomain);
  server_config_t *serverConfig = config_getServerConfigBySNI(config, domain);
  string_free(domain);
  if (serverConfig == 0) {
    log(LOG_ERROR, "Got unknown domain '%s' for TLS handshake", rawDomain);
    return SSL_TLSEXT_ERR_NOACK;
  }

  SSL_CTX *sslContext = config_getSSLContext(serverConfig);
  SSL_set_SSL_CTX(ssl, sslContext);

  return SSL_TLSEXT_ERR_OK;
}

SSL *server_handleSSL(connection_t *connection) {
  const SSL_METHOD *method = TLS_method();
  SSL_CTX* context = SSL_CTX_new(method);
  SSL_CTX_set_tlsext_servername_callback(context, server_handleServerNameIdentification);
  SSL *ssl = SSL_new(context);
  SSL_set_fd(ssl, connection->socket);

  // TODO: Actual polling etc. here
  // https://stackoverflow.com/questions/24312228/openssl-nonblocking-socket-accept-and-connect-failed
  // https://stackoverflow.com/questions/13751458/openssl-ssl-error-want-write-never-recovers-during-ssl-write
  // https://github.com/darrenjs/openssl_examples/blob/master/ssl_server_nonblock.c

  // TODO: Move this into the workers
  // Thread safety (comments):
  // https://stackoverflow.com/questions/5113333/how-to-implement-server-name-indication-sni

  // Set up structures necessary for polling
  struct pollfd readDescriptors[1];
  memset(readDescriptors, 0, sizeof(struct pollfd));
  readDescriptors[0].fd = socket;
  readDescriptors[0].events = POLLIN;

  struct pollfd writeDescriptors[1];
  memset(writeDescriptors, 0, sizeof(struct pollfd));
  writeDescriptors[0].fd = socket;
  writeDescriptors[0].events = POLLOUT;

  bool success = false;
  while (true) {
    ERR_clear_error();
    int status = SSL_accept(ssl);
    if (status <= 0) {
      int error = SSL_get_error(ssl, status);
      if (error == SSL_ERROR_WANT_READ) {
        log(LOG_DEBUG, "TLS socket wants READ, waiting");
        // Wait for the connection to be ready to read
        int status = poll(readDescriptors, 1, -1);
        if (status < -1) {
          log(LOG_ERROR, "Could not wait for connection to be readable");
          return 0;
        }
        log(LOG_DEBUG, "Woke up with status %d", status);
      } else if (error == SSL_ERROR_WANT_WRITE) {
        log(LOG_DEBUG, "TLS socket wants WRITE, waiting");
        // Wait for the connection to be ready to write
        int status = poll(writeDescriptors, 1, -1);
        if (status < -1) {
          log(LOG_ERROR, "Could not wait for connection to be writable");
          return 0;
        }
        log(LOG_DEBUG, "Woke up with status %d", status);
      } else if (error == SSL_ERROR_WANT_CLIENT_HELLO_CB) {
        log(LOG_ERROR, "Could not accept client hello");
        return 0;
      } else {
        log(LOG_ERROR, "Unable to accept TLS socket. Got code %d", error);
        return 0;
      }
    } else {
      return ssl;
    }
  }
}

void server_handleSignalSIGPIPE() {
  // Do nothing
}

void server_close() {
  log(LOG_INFO, "Closing server");
  EVP_cleanup();
}
