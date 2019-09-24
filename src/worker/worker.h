#ifndef WORKER_H
#define WORKER_H

#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>

#include "../connection/connection.h"
#include "../semaphore/semaphore.h"

// Before a worker has started, it is initializing (right after fork)
#define WORKER_STATUS_INITIALIZING 0
// When the worker has nothing to do and can accept a conncetion, it is idle
#define WORKER_STATUS_IDLE 1
// When the worker is handling a connection, it is working
#define WORKER_STATUS_WORKING 2

typedef struct {
  pthread_t thread;
  // The current status of the worker
  // Written to by the worker, consumed by parent
  uint8_t status;
  // The current connection the worker is handling
  // Written to by parent, read by the worker, set to null by worker when done
  connection_t *connection;
  // Control the sleep of the thread
  pthread_cond_t sleepCondition;
  pthread_mutex_t sleepMutex;
} worker_t;

worker_t *worker_spawn();

uint8_t worker_getStatus(worker_t *worker);
void worker_setConnection(worker_t *worker, connection_t *connection);
void worker_waitForExit(worker_t *worker);
// Kill the worker
void worker_kill(worker_t *worker);

void worker_free(worker_t *worker);

#endif
