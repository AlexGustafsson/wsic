#ifndef LIST_H
#define LIST_H

#include <stdio.h>

typedef struct list_node_t list_node_t;
struct list_node_t {
  list_node_t *next;
  list_node_t *previous;
  void *value;
};

typedef struct {
  list_node_t *current;
  list_node_t *tail;
  size_t currentIndex;
  size_t length;
} list_t;

list_t *list_create();
void list_addValue(list_t *list, void *value) __attribute__((nonnull(1)));
void list_moveToIndex(list_t *list, size_t index) __attribute__((nonnull(1)));
void *list_removeValue(list_t *list, size_t index) __attribute__((nonnull(1)));
void *list_getValue(const list_t *list, size_t index) __attribute__((nonnull(1)));
void *list_setValue(list_t *list, size_t index, void *value) __attribute__((nonnull(1)));
size_t list_getLength(const list_t *list) __attribute__((nonnull(1)));
ssize_t list_findIndex(const list_t *list, void *value) __attribute__((nonnull(1, 2)));
void list_clear(list_t *list) __attribute__((nonnull(1)));
void list_free(list_t *list) __attribute__((nonnull(1)));

#endif
