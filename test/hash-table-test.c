#include "unity/unity.h"

#include "../src/datastructures/hash-table/hash-table.h"

void hash_table_test_canStoreValues() {
  hash_table_t *hashTable = hash_table_create();

  string_t *key1 = string_fromCopy("Foo");
  int value1 = 5;
  string_t *key2 = string_fromCopy("Bar");
  int value2 = 7;
  string_t *key3 = string_fromCopy("Foo");
  int value3 = 2;

  // Add pair 1 and make sure the length increased
  hash_table_setValue(hashTable, key1, (void *)value1);
  TEST_ASSERT_EQUAL_INT(1, hash_table_getLength(hashTable));
  // Add pair 2 and make sure the length increased
  hash_table_setValue(hashTable, key2, (void *)value2);
  TEST_ASSERT_EQUAL_INT(2, hash_table_getLength(hashTable));
  // Add pair 3 and make sure the length remains the same (key3 and key1 are the same)
  hash_table_setValue(hashTable, key3, (void *)value3);
  TEST_ASSERT_EQUAL_INT(2, hash_table_getLength(hashTable));

  // Ensure that values are all available (and that value3 replaced value1)
  TEST_ASSERT_EQUAL_INT(value3, (int)hash_table_getValue(hashTable, key3));
  TEST_ASSERT_EQUAL_INT(value2, (int)hash_table_getValue(hashTable, key2));

  // Remove a pair and ensure that the value is returned
  TEST_ASSERT_EQUAL_INT(value3, (int)hash_table_removeValue(hashTable, key3));

  // Ensure that one can receive the correct key and value by index
  TEST_ASSERT_EQUAL(key2, hash_table_getKeyByIndex(hashTable, 0));
  TEST_ASSERT_EQUAL_INT(value2, (int)hash_table_getValueByIndex(hashTable, 0));

  // Ensure that out of bounds access returns null
  TEST_ASSERT_NULL(hash_table_getValueByIndex(hashTable, 4));
  TEST_ASSERT_NULL(hash_table_getKeyByIndex(hashTable, 4));

  // Ensure that there are no elements left after clearing
  hash_table_clear(hashTable);
  TEST_ASSERT(hash_table_getLength(hashTable) == 0);

  hash_table_free(hashTable);
}

void hash_table_test_run() {
  RUN_TEST(hash_table_test_canStoreValues);
}
