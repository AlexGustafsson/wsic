#ifndef SET_H
#define SET_H

#include <stdio.h>

#include "../list/list.h"

typedef list_t set_t;

set_t *set_create();
void set_addValue(set_t *set, void *value);
void *set_removeValue(set_t *set, size_t index);
void *set_getValue(set_t *set, size_t index);
size_t set_getLength(set_t *set);
void set_clear(set_t *set);
void set_free(set_t *set);

#endif
