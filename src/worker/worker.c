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
  worker->id = id;

  // Create a shared file which only the user running WSIC can read
  string_t *sharedFileName = worker_getSharedFileName(worker->id);
  int sharedDescriptor = shm_open(string_getBuffer(sharedFileName), O_RDWR | O_CREAT, 0600);
  string_free(sharedFileName);
  if (sharedDescriptor < 0) {
    free(worker);
    return 0;
  }

  // Enlarge the file
  if (ftrucate(sharedDescriptor, sizeof(worker_channel_t)) < 0) {
    close(sharedDescriptor);
    free(worker);
    return 0;
  }
  worker->sharedDescriptor = sharedDescriptor;

  // Map the file into memory
  // A NULL address let's the system picks an address for us
  worker_channel_t *channel = mmap(NULL, sizeof(worker_channel_t), PROT_READ | PROT_WRITE, MAP_SHARED, sharedDescriptor, 0);
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

string_t *worker_getSharedFileName(int id) {
  string_t *sharedFileName = string_fromCopy(WORKER_SHARED_MEMORY_NAME);
  string_appendChar(sharedFileName, '_');
  string_t *idString = string_fromInt(id);
  string_append(sharedFileName, idString);
  string_free(idString);

  return sharedFileName;
}

void worker_free(worker_t *worker) {
  // Unmap the memory
  munmap(worker->channel, sizeof(worker_channel_t));

  // Close the shared file
  close(worker->sharedDescriptor);

  // Unlink the shared file
  string_t *sharedFileName = worker_getSharedFileName(worker->id);
  shm_unlink(string_getBuffer(sharedFileName));
  string_free(sharedFileName);

  // Free the worker itself
  free(worker);
}
