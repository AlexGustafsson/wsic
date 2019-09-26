#ifndef SEMAPHORE_H
#define SEMAPHORE_H

/**
  This is a somewhat portable semaphore abstraction targeting macOS and Ubuntu.
  It exposes the same interface for both platforms.

  NOTE: It creates semaphores for use between processes only.

  NOTE: It is losely inspired by:
  https://stackoverflow.com/questions/27736618/why-are-sem-init-sem-getvalue-sem-destroy-deprecated-on-mac-os-x-and-w
  https://github.com/adrienverge/openfortivpn/issues/105
*/

#include <stdbool.h>
#ifdef __APPLE__
#include <dispatch/dispatch.h>
#else
#include <semaphore.h>
#endif

typedef struct {
  #ifdef __APPLE__
  dispatch_semaphore_t semaphore;
  #else
  sem_t semaphore;
  #endif
} semaphore_t;

semaphore_t *semaphore_create(int initialValue);

bool semaphore_lock(semaphore_t *semaphore);
bool semaphore_unlock(semaphore_t *semaphore);

void semaphore_free(semaphore_t *semaphore);

#endif
