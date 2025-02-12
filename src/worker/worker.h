#ifndef WORKER_H
#define WORKER_H

#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>

#include "../datastructures/message-queue/message-queue.h"
#include "../connection/connection.h"
#include "../cgi/cgi.h"

// Before a worker has started, it is initializing (right after fork)
#define WORKER_STATUS_INITIALIZING 0
// When the worker has nothing to do and can accept a conncetion, it is idle
#define WORKER_STATUS_IDLE 1
// When the worker is handling a connection, it is working
#define WORKER_STATUS_WORKING 2

// Don't allow headers larger than 1 MB
#define REQUEST_MAX_HEADER_SIZE 1048576
// Don't allow connections to wait for more than one second without sending data when reading
#define REQUEST_READ_TIMEOUT 1000
// Don't allow bodies larger than 1 MB
#define REQUEST_MAX_BODY_SIZE 1048576

// The maximum time allowed for reading the response from a CGI process
#define CGI_READ_TIMEOUT 1000

typedef struct {
  // Always NULL if in immediate mode
  pthread_t thread;
  // The current status of the worker
  // Written to by the worker, consumed by parent
  uint8_t status;
  message_queue_t *queue;
  int id;
  // The current connection (if any)
  connection_t *connection;
  // The current CGI process (if any)
  cgi_process_t *cgi;
  // Whether or not the worker should run (exit condition)
  bool shouldRun;
} worker_t;

// Pass a connection to handle it directly and destroy the thread after use (immediate mode)
// Or pass NULL and a queue in order to make the thread listen for incoming connections (pool mode)
// Returns NULL if ran in immediate mode (the thread takes care of the memory)
// The connection (if passed) is owned
worker_t *worker_spawn(int id, connection_t *connection, message_queue_t *queue);
uint8_t worker_getStatus(const worker_t *worker) __attribute__((nonnull(1)));
// Wait for the worker to exit naturally or after killing (undefined behaviour for immediate mode)
void worker_waitForExit(const worker_t *worker) __attribute__((nonnull(1)));
// Kill the worker (undefined behaviour for immediate mode)
void worker_kill(worker_t *worker) __attribute__((nonnull(1)));
// Mark a worker as dead, letting it exit when ready
void worker_closeGracefully(worker_t *worker) __attribute__((nonnull(1)));
// Undefined behaviour for immediate mode
void worker_free(worker_t *worker) __attribute__((nonnull(1)));

#endif
