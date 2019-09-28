#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include <pthread.h>

#include "../list/list.h"

typedef struct {
  list_t *list;
  // Control blocking of the queue
  pthread_cond_t lockCondition;
  pthread_mutex_t mutex;
} message_queue_t;

message_queue_t *message_queue_create();
// Push a value and alert at least one of the waiting threads
void message_queue_push(message_queue_t *queue, void *value);
// Lock the calling thread until a value can be popped
void *message_queue_pop(message_queue_t *queue);
size_t message_queue_getLength(message_queue_t *queue);
// Not thread safe (usually called alongside message_queue_free)
void message_queue_clear(message_queue_t *queue);
// NOTE: The calling thread should ensure that no threads are locked waiting for a state change
// That is, they should all have been destroyed
void message_queue_free(message_queue_t *queue);

#endif
