#include <stdbool.h>
#include <stdlib.h>

#include "../../logging/logging.h"

#include "message-queue.h"

message_queue_t *message_queue_create() {
  message_queue_t *queue = malloc(sizeof(message_queue_t));
  if (queue == 0) {
    log(LOG_ERROR, "Failed to allocate message queue");
    return 0;
  }

  queue->list = list_create();
  if (queue->list == 0) {
    log(LOG_ERROR, "Failed to allocate list");
    free(queue);
    return 0;
  }

  if (pthread_cond_init(&queue->lockCondition, NULL) != 0) {
    log(LOG_ERROR, "Failed to create locking condition for queue");
    free(queue);
    list_free(queue->list);
    return 0;
  }

  // Create a mutex locked while working
  if (pthread_mutex_init(&queue->mutex, NULL) != 0) {
    log(LOG_ERROR, "Failed to create mutex for thread");
    pthread_cond_destroy(&queue->lockCondition);
    free(queue);
    list_free(queue->list);
    return 0;
  }

  return queue;
}

void message_queue_push(message_queue_t *queue, void *value) {
  pthread_mutex_lock(&queue->mutex);
  list_addValue(queue->list, value);
  // Alert at least one of the waiting threads that the state has changed
  pthread_cond_signal(&queue->lockCondition);
  pthread_mutex_unlock(&queue->mutex);
}

void *message_queue_pop(message_queue_t *queue) {
  // Lock the thread until a connection is available
  pthread_mutex_lock(&queue->mutex);
  void *value = 0;
  while (true) {
    value = list_removeValue(queue->list, 0);
    if (value == 0)
      pthread_cond_wait(&queue->lockCondition, &queue->mutex);
    else
      break;
  }
  pthread_mutex_unlock(&queue->mutex);

  return value;
}

size_t message_queue_getLength(message_queue_t *queue) {
  pthread_mutex_lock(&queue->mutex);
  size_t length = list_getLength(queue->list);
  pthread_mutex_unlock(&queue->mutex);

  return length;
}

void message_queue_clear(message_queue_t *queue) {
  list_clear(queue->list);
}

void message_queue_free(message_queue_t *queue) {
  pthread_cond_destroy(&queue->lockCondition);
  pthread_mutex_unlock(&queue->mutex);
  pthread_cond_destroy(&queue->lockCondition);

  list_free(queue->list);
  free(queue);
}