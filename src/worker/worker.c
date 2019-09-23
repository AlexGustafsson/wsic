#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

#include "../string/string.h"

#include "worker.h"

// Private method
string_t *worker_getSharedFileName(int id);

worker_t *worker_spawn(int id) {
  worker_t *worker = malloc(sizeof(worker));
  if (worker == 0)
    return 0;

  // Map the file into memory
  // A NULL address let's the system picks an address for us
  // Anonymous and a -1 file descriptor indicates that there is no backing file (only the child process can use the mapping)
  worker_channel_t *channel = mmap(NULL, sizeof(worker_channel_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (channel == 0) {
    free(worker);
    return 0;
  }
  worker->channel = channel;

  return worker;
}

uint8_t worker_getStatus(worker_t *worker) {
  return 0;
}

void worker_setConnection(worker_t *worker, connection_t *connection) {
  return;
}

void worker_free(worker_t *worker) {
  // Unmap the memory
  munmap(worker->channel, sizeof(worker_channel_t));

  // Free the worker itself
  free(worker);
}
