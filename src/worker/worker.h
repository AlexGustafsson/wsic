#ifndef WORKER_H
#define WORKER_H

#include <unistd.h>

#include "../connection/connection.h"

#define WORKER_STATUS_IDLE 0
#define WORKER_STATUS_WORKING 1

// Shared memory between parent and worker
typedef struct {
  // The current status of the worker
  // Written to by the worker, consumed by parent
  uint8_t status;
  // The current connection the worker is handling
  // Written to by parent, read by the worker, set to null by worker when done
  connection_t *connection;
} worker_channel_t;

typedef struct {
  pid_t pid;
  worker_channel_t *channel;
} worker_t;

worker_t *worker_spawn(int id);

uint8_t worker_getStatus(worker_t *worker);
void worker_setConnection(worker_t *worker, connection_t *connection);

void worker_free(worker_t *worker);

#endif
