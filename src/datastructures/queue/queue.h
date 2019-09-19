#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>

#include "../list/list.h"

typedef list_t queue_t;

queue_t* queue_create();
void queue_push(queue_t* queue, void* value);
void* queue_pop(queue_t* queue);
void* queue_peek(queue_t* queue);
size_t queue_getLength(queue_t* queue);
void queue_clear(queue_t* queue);
void queue_free(queue_t* queue);

#endif
