#include "set.h"

set_t *set_create() {
  return list_create();
}

void set_addValue(set_t *set, void *value) {
  ssize_t index = list_findIndex(set, value);
  // Ignore duplicate values
  if (index != -1)
    return;

  list_addValue(set, value);
}

void *set_removeValue(set_t *set, size_t index) {
  return list_removeValue(set, index);
}

void *set_getValue(const set_t *set, size_t index) {
  return list_getValue(set, index);
}

size_t set_getLength(const set_t *set) {
  return list_getLength(set);
}

void set_clear(set_t *set) {
  list_clear(set);
}

void set_free(set_t *set) {
  list_free(set);
}
