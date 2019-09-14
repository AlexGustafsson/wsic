#ifndef HASH_TABLE_H
#define HASH_TABLE_H

/**
* Currently this is NOT a proper hash table. It has the interface of a hash table,
* but is simply implemented as a list of key-value pairs which are looped over
* to find the matching key.
*/

#include <stdlib.h>
#include <stdint.h>

#include "../list/list.h"
#include "../../string/string.h"

typedef struct {
  uint32_t keyHash;
  string_t *key;
  void *value;
} hash_table_entry_t;

typedef struct {
  list_t *entries;
} hash_table_t;

uint32_t hash_table_hash(const char* value);

hash_table_t* hash_table_create();
void* hash_table_setValue(hash_table_t* hashTable, string_t *key, void* value);
void* hash_table_removeValue(hash_table_t* hashTable, string_t *key);
void* hash_table_getValue(hash_table_t* hashTable, string_t *key);
hash_table_entry_t *hash_table_getEntryByIndex(hash_table_t* hashTable, size_t index);
string_t *hash_table_getKeyByIndex(hash_table_t* hashTable, size_t index);
void* hash_table_getValueByIndex(hash_table_t* hashTable, size_t index);
ssize_t hash_table_findIndex(hash_table_t* hashTable, uint32_t keyHash);
size_t hash_table_getLength(hash_table_t* hashTable);
void hash_table_clear(hash_table_t* hashTable);
void hash_table_free(hash_table_t* hashTable);

#endif
