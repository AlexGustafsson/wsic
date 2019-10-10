#ifndef HASH_TABLE_H
#define HASH_TABLE_H

/**
* Currently this is NOT a proper hash table. It has the interface of a hash table,
* but is simply implemented as a list of key-value pairs which are looped over
* to find the matching key.
*/

#include <stdint.h>
#include <stdlib.h>

#include "../../string/string.h"
#include "../list/list.h"

typedef struct {
  uint32_t keyHash;
  string_t *key;
  void *value;
} hash_table_entry_t;

typedef struct {
  list_t *entries;
} hash_table_t;

uint32_t hash_table_hash(const char *value) __attribute__((nonnull(1)));

hash_table_t *hash_table_create();
void *hash_table_setValue(hash_table_t *hashTable, string_t *key, void *value) __attribute__((nonnull(1, 2)));
void *hash_table_removeValue(hash_table_t *hashTable, const string_t *key) __attribute__((nonnull(1, 2)));
void *hash_table_getValue(const hash_table_t *hashTable, const string_t *key) __attribute__((nonnull(1, 2)));
hash_table_entry_t *hash_table_getEntryByIndex(const hash_table_t *hashTable, size_t index) __attribute__((nonnull(1)));
string_t *hash_table_getKeyByIndex(const hash_table_t *hashTable, size_t index) __attribute__((nonnull(1)));
void *hash_table_getValueByIndex(const hash_table_t *hashTable, size_t index) __attribute__((nonnull(1)));
ssize_t hash_table_findIndex(const hash_table_t *hashTable, uint32_t keyHash) __attribute__((nonnull(1)));
size_t hash_table_getLength(const hash_table_t *hashTable) __attribute__((nonnull(1)));
void hash_table_clear(hash_table_t *hashTable) __attribute__((nonnull(1)));
void hash_table_free(hash_table_t *hashTable) __attribute__((nonnull(1)));

#endif
