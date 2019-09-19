#include "queue.h"

queue_t *queue_create() {
  return list_create();
}

void queue_push(queue_t *queue, void *value) {
  list_addValue(queue, value);
}

void *queue_pop(queue_t *queue) {
  return list_removeValue(queue, 0);
}

void *queue_peek(queue_t *queue) {
  return list_getValue(queue, 0);
}

size_t queue_getLength(queue_t *queue) {
  return list_getLength(queue);
}

void queue_clear(queue_t *queue) {
  list_clear(queue);
}

void queue_free(queue_t *queue) {
  list_free(queue);
}
