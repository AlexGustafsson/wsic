#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include <pthread.h>
#include <stdbool.h>

#include "../list/list.h"

typedef struct {
  list_t *list;
  // Control blocking of the queue
  pthread_cond_t lockCondition;
  pthread_mutex_t mutex;
  // Whether or not the queue is unlocked (see message_queue_unlock)
  bool unlocked;
} message_queue_t;

message_queue_t *message_queue_create();
// Push a value and alert at least one of the waiting threads
void message_queue_push(message_queue_t *queue, void *value) __attribute__((nonnull(1)));
// Lock the calling thread until a value can be popped
void *message_queue_pop(message_queue_t *queue) __attribute__((nonnull(1)));
size_t message_queue_getLength(message_queue_t *queue) __attribute__((nonnull(1)));
// Not thread safe (usually called alongside message_queue_free)
void message_queue_clear(message_queue_t *queue) __attribute__((nonnull(1)));
// Unlock all waiting threads - useful after pthread_cancel to ensure no thread is deadlocked
// The message queue is expected to be freed after this call - other methods are undefined behaviour
void message_queue_unlock(message_queue_t *queue) __attribute__((nonnull(1)));
// NOTE: The calling thread should ensure that no threads are locked waiting for a state change
// That is, they should all have been destroyed
void message_queue_free(message_queue_t *queue) __attribute__((nonnull(1)));

#endif
