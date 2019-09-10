#define DEBUG_SKIP_LOG

#include <string.h>

#include "list.h"

list_t* list_create() {
  list_t* list = malloc(sizeof(list_t));
  if (list == 0)
    return 0;

  memset(list, 0, sizeof(list_t));

  return list;
}

size_t list_addValue(list_t* list, void* value) {
  list_node_t* node = malloc(sizeof(list_node_t));
  if (node == 0)
    return list->length;

  node->value = value;

  if (list->length == 0) {
    node->next = node;
    node->previous = node;

    list->current = node;
    list->tail = node;
  } else {
    node->next = list->tail->next;
    list->tail->next->previous = node;

    node->previous = list->tail;
    list->tail->next = node;

    list->tail = node;
  }

  list->length++;
  return list->length;
}

void list_moveToIndex(list_t* list, size_t index) {
  if (list->currentIndex == index)
    return;

  if (index == 0) {
    list->currentIndex = 0;
    list->current = list->tail->next;

    return;
  } else if (index + 1 == list->length) {
    list->currentIndex = index;
    list->current = list->tail;

    return;
  }

  // Direction to step in in linked list
  int direction = 0;
  if (list->currentIndex < index) {
    size_t leftSteps = 1 + list->currentIndex + list->length - index;
    size_t rightSteps = index - list->currentIndex;
    direction = leftSteps < rightSteps ? -1 : 1;
  } else if (list->currentIndex > index) {
    size_t leftSteps = list->currentIndex - index;
    size_t rightSteps = 1 + list->length - list->currentIndex + index;
    direction = leftSteps < rightSteps ? -1 : 1;
  }

  while (list->currentIndex != index) {
    list->currentIndex = (list->currentIndex + 1 * direction) % list->length;
    list->current = direction == 1 ? list->current->next : list->current->previous;
  }
}

// NOTE: Does not free memory for values held, just internal structure
size_t list_removeValue(list_t* list, size_t index) {
  // Return if index does not exist
  if (list->length <= index || list->length == 0)
    return list->length;

  if (list->length == 1) {
    free(list->current);
    list->current = 0;
    list->tail = 0;
  } else {
    list_moveToIndex(list, index);
    list_node_t* current = list->current;

    current->next->previous = current->previous;
    current->previous->next = current->next;

    if (list->currentIndex + 1 == list->length)
      list->tail = current->previous;

    list->current = current->next;
    free(current);
  }

  list->length--;
  list->currentIndex = list->length > 0 ? list->currentIndex % list->length : 0;

  return list->length;
}

void* list_getValue(list_t* list, size_t index) {
  // Return null if index does not exist
  if (list->length <= index)
    return 0;

  list_moveToIndex(list, index);

  return list->current->value;
}

// NOTE: Does not free already stored value
void list_setValue(list_t* list, size_t index, void* value) {
  // Return if index does not exist
  if (list->length <= index)
    return;

  list_moveToIndex(list, index);

  list->current->value = value;
}

void list_clear(list_t* list) {
  if (list->length == 0)
    return;

  // list_removeValue alters list->length, therefore we need to store it beforehand
  size_t length = list->length;
  for (size_t i = 0; i < length; i++)
    list_removeValue(list, 0);
}

void list_free(list_t *list) {
  list_clear(list);
  free(list);
}
