#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "../logging/logging.h"
#include "../string/string.h"

#include "worker.h"

// Private methods
// The main entry point of a worker
int worker_entryPoint();
int worker_handleConnection(connection_t *connection);

worker_t *worker_spawn(connection_t *connection) {
  worker_t *worker = malloc(sizeof(worker_t));
  if (worker == 0)
    return 0;
  memset(worker, 0, sizeof(worker_t));

  if (connection == 0) {
    // Create a condition for when the thread should sleep
    if (pthread_cond_init(&worker->sleepCondition, NULL) != 0) {
      log(LOG_ERROR, "Unable to create sleeping condition");
      free(worker);
      return 0;
    }

    // Create a mutex locked while working
    if (pthread_mutex_init(&worker->sleepMutex, NULL) != 0) {
      log(LOG_ERROR, "Unable to create sleeping mutex");
      pthread_cond_destroy(&worker->sleepCondition);
      free(worker);
      return 0;
    }
  } else {
    worker->connection = connection;
  }

  pthread_t thread;

  if (pthread_create(&thread, NULL, worker_entryPoint, worker) != 0) {
    log(LOG_ERROR, "Unable to start thread for worker");
    free(worker);
    if (connection != 0) {
      pthread_cond_destroy(&worker->sleepCondition);
      pthread_mutex_destroy(&worker->sleepMutex);
    }
    return 0;
  }

  if (connection == 0) {
    // Due to the thread already being started and may have already
    // freed itself, it would be a race condition to add the thread here
    // if in immediate mode - don't do it
    worker->thread = thread;

    log(LOG_DEBUG, "Created thread in pool mode");
    return worker;
  } else {
    log(LOG_DEBUG, "Created thread in immediate mode");
    return worker;
  }
}

uint8_t worker_getStatus(worker_t *worker) {
  return worker->status;
}

void worker_setConnection(worker_t *worker, connection_t *connection) {
  // TODO: Thread safety
  pthread_mutex_lock(&worker->sleepMutex);
  worker->connection = connection;
  pthread_cond_signal(&worker->sleepCondition);
  pthread_mutex_unlock(&worker->sleepMutex);
}

void worker_setStatus(worker_t *worker, uint8_t status) {
  worker->status = status;
}

connection_t *worker_getConnection(worker_t *worker) {
  return worker->connection;
}

int worker_entryPoint(worker_t *worker) {
  // If a connection is already set, handle it directly (immediate mode)
  if (worker->connection != 0) {
    log(LOG_DEBUG, "Handling a connection in immediate mode");
    worker->status = WORKER_STATUS_WORKING;
    int exitCode = worker_handleConnection(worker->connection);
    log(LOG_DEBUG, "Connection handling exited with code %d", exitCode);
    if (exitCode != 0)
      log(LOG_ERROR, "Handling the connection resulted in a non-zero exit code: %d", exitCode);
    worker->status = WORKER_STATUS_IDLE;
    worker_free(worker);
    return exitCode;
  }

  log(LOG_DEBUG, "Initializing worker");

  // Run the thread continously to handle multiple connections (pool mode)
  while (true) {
    log(LOG_DEBUG, "Putting worker to sleep");
    worker->status = WORKER_STATUS_IDLE;

    // Lock the thread until a connection is available
    pthread_mutex_lock(&worker->sleepMutex);
    while (worker->connection == 0)
      pthread_cond_wait(&worker->sleepCondition, &worker->sleepMutex);
    pthread_mutex_unlock(&worker->sleepMutex);

    log(LOG_DEBUG, "Worker process interrupted by parent to handle a connection");

    worker->status = WORKER_STATUS_WORKING;

    worker_handleConnection(worker->connection);
    log(LOG_DEBUG, "Handled connection - closing it");
    // Free the connection as it's of no further use
    connection_free(worker->connection);

    worker->connection = 0;
  }

  return 0;
}

int worker_handleConnection(connection_t *connection) {
  // Handle the connection
  connection_write(connection, "Hello World!", 12);

  return 0;
}

void worker_waitForExit(worker_t *worker) {
  pthread_join(worker->thread, NULL);
}

void worker_kill(worker_t *worker) {
  pthread_cancel(worker->thread);
  // Ensure that the sleep mutex is unlocked
  pthread_mutex_unlock(&worker->sleepMutex);
  // Ensure that the condition is destroyed after the threads has exited
  pthread_cond_destroy(&worker->sleepCondition);
}

void worker_free(worker_t *worker) {
  if (worker->connection != 0)
    connection_free(worker->connection);
  // Free the worker itself
  free(worker);
}
