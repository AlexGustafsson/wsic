#include <string.h>

#include "hash-table.h"

// Uses the CRC32 hash algorithm
// (https://en.wikipedia.org/wiki/Cyclic_redundancy_check) Mostly adapted from
// the excellent https://www.hackersdelight.org/hdcodetxt/crc.c.txt License of
// the original code http://www.hackersdelight.org/permissions.htm NOTE: Expects
// values to be null terminated!
static uint32_t hashLookup[256] = {0};
uint32_t hash_table_hash(const char *value) {
  uint32_t crc, mask;

  if (hashLookup[1] == 0) {
    for (uint32_t byte = 0; byte < 256; byte++) {
      crc = byte;
      for (uint32_t bit = 0; bit < 8; bit++) {
        mask = -(crc & 1);
        // The magic number is the polynomial used
        crc = (crc >> 1) ^ (0xEDB88320 & mask);
      }
      hashLookup[byte] = crc;
    }
  }

  crc = 0xFFFFFFFF;
  for (uint32_t byte = 0; value[byte] != 0; byte++)
    crc = (crc >> 8) ^ hashLookup[(crc ^ value[byte]) & 0xFF];

  return ~crc;
}

hash_table_t *hash_table_create() {
  hash_table_t *hashTable = malloc(sizeof(hash_table_t));
  if (hashTable == 0)
    return 0;

  memset(hashTable, 0, sizeof(hash_table_t));

  hashTable->entries = list_create();
  if (hashTable->entries == 0) {
    free(hashTable);

    return 0;
  }

  return hashTable;
}

void *hash_table_setValue(hash_table_t *hashTable, const char *key,
                          void *value) {
  uint32_t keyHash = hash_table_hash(key);
  ssize_t keyIndex = hash_table_findIndex(hashTable, keyHash);

  if (keyIndex == -1) {
    hash_table_entry_t *entry = malloc(sizeof(hash_table_entry_t));
    entry->key = keyHash;
    entry->value = value;
    list_addValue(hashTable->entries, entry);

    return 0;
  }

  hash_table_entry_t *entry = list_getValue(hashTable->entries, keyIndex);
  void *removedValue = entry->value;
  entry->value = value;
  return removedValue;
}

void *hash_table_removeValue(hash_table_t *hashTable, const char *key) {
  uint32_t keyHash = hash_table_hash(key);
  ssize_t keyIndex = hash_table_findIndex(hashTable, keyHash);

  if (keyIndex == -1)
    return 0;

  hash_table_entry_t *entry = list_removeValue(hashTable->entries, keyIndex);
  void *removedValue = entry->value;
  free(entry);

  return removedValue;
}

void *hash_table_getValue(hash_table_t *hashTable, const char *key) {
  uint32_t keyHash = hash_table_hash(key);
  ssize_t keyIndex = hash_table_findIndex(hashTable, keyHash);

  if (keyIndex == -1)
    return 0;

  hash_table_entry_t *entry = list_getValue(hashTable->entries, keyIndex);
  return entry->value;
}

ssize_t hash_table_findIndex(hash_table_t *hashTable, uint32_t keyHash) {
  for (size_t i = 0; i < hashTable->entries->length; i++) {
    hash_table_entry_t *entry =
        (hash_table_entry_t *)hashTable->entries->current->value;

    if (entry->key == keyHash)
      return hashTable->entries->currentIndex;

    list_moveToIndex(hashTable->entries,
                     (hashTable->entries->currentIndex + 1) %
                         hashTable->entries->length);
  }

  return -1;
}

// NOTE: Does not free values
void hash_table_clear(hash_table_t *hashTable) {
  for (size_t i = 0; i < hashTable->entries->length; i++) {
    size_t index =
        (hashTable->entries->currentIndex + i) % hashTable->entries->length;
    hash_table_entry_t *entry = list_removeValue(hashTable->entries, index);

    free(entry);
  }
}

// NOTE: Does not free values
void hash_table_free(hash_table_t *hashTable) {
  hash_table_clear(hashTable);
  list_free(hashTable->entries);
  free(hashTable);
}
