#ifndef SET_H
#define SET_H

#include <stdio.h>

#include "../list/list.h"

typedef list_t set_t;

set_t *set_create();
void set_addValue(set_t *set, void *value) __attribute__((nonnull(1)));
void *set_removeValue(set_t *set, size_t index) __attribute__((nonnull(1)));
void *set_getValue(const set_t *set, size_t index) __attribute__((nonnull(1)));
size_t set_getLength(const set_t *set) __attribute__((nonnull(1)));
void set_clear(set_t *set) __attribute__((nonnull(1)));
void set_free(set_t *set) __attribute__((nonnull(1)));

#endif
