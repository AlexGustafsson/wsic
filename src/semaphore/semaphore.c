#include <stdlib.h>
#include <sys/mman.h>

#include "../logging/logging.h"

#include "semaphore.h"

semaphore_t *semaphore_create(int initialValue) {
  // Map a shared memory address for the semaphore
  semaphore_t *semaphore = mmap(NULL, sizeof(semaphore_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (semaphore == 0) {
    log(LOG_ERROR, "Failed to allocate shared memory for semaphore");
    return 0;
  }

#ifdef __APPLE__
  semaphore->semaphore = dispatch_semaphore_create(initialValue);
  if (semaphore->semaphore == 0) {
    log(LOG_ERROR, "Failed to create semaphore");
    munmap(semaphore->semaphore, sizeof(dispatch_semaphore_t));
    free(semaphore);
    return 0;
  }
#else
  if (sem_init(&semaphore->semaphore, 1, initialValue) == -1) {
    log(LOG_ERROR, "Failed to create semaphore");
    munmap(semaphore, sizeof(sem_t));
    free(semaphore);
    return 0;
  }
#endif

  return semaphore;
}

bool semaphore_lock(semaphore_t *semaphore) {
#ifdef __APPLE__
  if (dispatch_semaphore_wait(semaphore->semaphore, DISPATCH_TIME_FOREVER) != 0) {
    log(LOG_ERROR, "Could not lock semaphore");
    return false;
  }
#else
  if (sem_wait(&semaphore->semaphore) == -1) {
    log(LOG_ERROR, "Could not lock semaphore");
    return false;
  }
#endif

  return true;
}

bool semaphore_unlock(semaphore_t *semaphore) {
#ifdef __APPLE__
  if (dispatch_semaphore_signal(semaphore->semaphore) != 0) {
    log(LOG_ERROR, "Could not lock semaphore");
    return false;
  }
#else
  if (sem_post(&semaphore->semaphore) == -1) {
    log(LOG_ERROR, "Could not lock semaphore");
    return false;
  }
#endif

  return true;
}

void semaphore_free(semaphore_t *semaphore) {
#ifdef __APPLE__
  dispatch_release(semaphore->semaphore);
#else
  sem_destroy(&semaphore->semaphore);
#endif
  free(semaphore);
}
