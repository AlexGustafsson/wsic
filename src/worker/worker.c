#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#include "../logging/logging.h"
#include "../string/string.h"

#include "worker.h"

// Not all versions of OS X or macOS seem to have MAP_ANONYMOUS
// They do however have MAP_ANON
#ifdef __APPLE__
#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif
#endif

// Private methods
// The main entry point of a worker
int worker_entryPoint();
void worker_handleConnection(connection_t *connection);
void worker_handleSignalSIGKILL();

// Locking methods
void worker_setStatus(worker_t* worker, uint8_t status);
connection_t *worker_getConnection(worker_t* worker);

// The worker object. Set only in a worker process
static worker_t *self = 0;

worker_t *worker_spawn() {
  worker_t *worker = malloc(sizeof(worker_t));
  if (worker == 0)
    return 0;

  // Map a shared memory address
  // A NULL address let's the system picks an address for us
  // Anonymous and a -1 file descriptor indicates that there is no backing file (only the child process can use the mapping)
  worker->channel = mmap(NULL, sizeof(worker_channel_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (worker->channel == 0) {
    log(LOG_ERROR, "Failed to allocate shared memory for worker");
    free(worker);
    return 0;
  }

  // Create a semaphore
  worker->channelSemaphore = semaphore_create(1);
  if (worker->channelSemaphore == 0) {
    log(LOG_ERROR, "Failed to create semaphore for worker channel");
    semaphore_free(worker->channelSemaphore);
    munmap(worker->channel, sizeof(worker_channel_t));
    free(worker);
    return 0;
  }

  // Fork the process
  worker->pid = fork();

  if (worker->pid < 0) {
    log(LOG_ERROR, "Failed to spawn worker");
    worker_free(worker);

    return 0;
  } else if (worker->pid == 0) {
    // Set the worker object of the process
    self = worker;
    log(LOG_DEBUG, "Entering worker entry point");
    // Put the worker into the entry point
    int exitCode = worker_entryPoint();
    // If this is ever returned, then the worker process failed and will exit
    exit(exitCode);
  }

  log(LOG_DEBUG, "Spawned worker with pid %d", worker->pid);
  return worker;
}

uint8_t worker_getStatus(worker_t *worker) {
  semaphore_lock(worker->channelSemaphore);
  uint8_t status = worker->channel->status;
  semaphore_unlock(worker->channelSemaphore);
  return status;
}

void worker_setConnection(worker_t *worker, connection_t *connection) {
  semaphore_lock(worker->channelSemaphore);
  worker->channel->connection = connection;
  // Wait for the memory to synchronize
  msync(worker->channel, sizeof(worker_channel_t), MS_SYNC);
  semaphore_unlock(worker->channelSemaphore);
}

void worker_setStatus(worker_t* worker, uint8_t status) {
  semaphore_lock(worker->channelSemaphore);
  worker->channel->status = status;
  // Wait for the memory to synchronize
  msync(worker->channel, sizeof(worker_channel_t), MS_SYNC);
  semaphore_unlock(worker->channelSemaphore);
}

connection_t *worker_getConnection(worker_t* worker) {
  semaphore_lock(worker->channelSemaphore);
  connection_t *connection = worker->channel->connection;
  semaphore_unlock(worker->channelSemaphore);
  return connection;
}

int worker_entryPoint() {
  // Setup signal handling
  signal(SIGKILL, worker_handleSignalSIGKILL);

  // Setup mask of signals able to wake the process up
  sigset_t mask;
  sigemptyset(&mask);
  // The parent is trying to interrupt worker
  sigaddset(&mask, SIGUSR1);
  sigaddset(&mask, SIGINT);
  sigaddset(&mask, SIGTERM);
  // Block the above signals (as we want them to be handled by sigwait)
  if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
    log(LOG_ERROR, "Could not set signal mask for worker process");
    return 1;
  }

  worker_setStatus(self, WORKER_STATUS_IDLE);

  while (true) {
    // Wait for a signal whitelisted above
    int signal;
    if (sigwait(&mask, &signal) != 0) {
      log(LOG_ERROR, "Could not wait for signal");
      return 1;
    }

    log(LOG_DEBUG, "Worker process interrupted by parent (%d)", signal);

    if (signal == SIGUSR1) {
      log(LOG_DEBUG, "Worker was interrupted to handle a connection");
      connection_t *connection = worker_getConnection(self);
      // If the channel was interrupted without a connection assigned, wait again
      if (connection == 0) {
        log(LOG_WARNING, "Worker interrupted without receiving a connection");
        continue;
      }

      worker_setStatus(self, WORKER_STATUS_WORKING);
      connection_write(connection, "Hello World!", 12);
      connection_free(connection);
      worker_setConnection(self, 0);
      worker_setStatus(self, WORKER_STATUS_WORKING);
    } else {
      log(LOG_DEBUG, "Worker was interrupted to shut down");
      exit(0);
    }
  }

  return 0;
}

void worker_handleSignalSIGKILL() {
  log(LOG_DEBUG, "Worker got SIGKILL - exiting immediately");
  exit(0);
}

uint8_t worker_waitForExit(worker_t *worker) {
  int status;
  waitpid(worker->pid, &status, 0);

  return WEXITSTATUS(status);
}

bool worker_isAlive(worker_t *worker) {
  int status;
  pid_t pid = waitpid(worker->pid, &status, WNOHANG);

  return pid == 0;
}

void worker_interrupt(worker_t *worker) {
  kill(worker->pid, SIGUSR1);
}

void worker_close(worker_t *worker) {
  kill(worker->pid, SIGTERM);
}

void worker_kill(worker_t *worker) {
  kill(worker->pid, SIGKILL);
}

void worker_free(worker_t *worker) {
  // Kill the process
  worker_kill(worker);

  // Unmap the memory
  munmap(worker->channel, sizeof(worker_channel_t));

  // Free the worker itself
  free(worker);
}
