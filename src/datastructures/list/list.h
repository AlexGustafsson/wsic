#ifndef LIST_H
#define LIST_H

#include <stdio.h>

typedef struct list_node_t list_node_t;
struct list_node_t {
  list_node_t* next;
  list_node_t* previous;
  void* value;
};

typedef struct {
  list_node_t* current;
  list_node_t* tail;
  size_t currentIndex;
  size_t length;
} list_t;

list_t* list_create();
void list_addValue(list_t* list, void* value);
void list_moveToIndex(list_t *list, size_t index);
void* list_removeValue(list_t* list, size_t index);
void* list_getValue(list_t* list, size_t index);
void* list_setValue(list_t* list, size_t index, void* value);
size_t list_getLength(list_t* list);
ssize_t list_findIndex(list_t* list, void* value);
void list_clear(list_t* list);
void list_free(list_t* list);

#endif
