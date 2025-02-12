#include "unity/unity.h"

#include "../src/datastructures/list/list.h"

void list_test_canStoreValues() {
  list_t *list = list_create();

  int a = 4;
  int b = 2;
  int c = 0;
  int d = 1337;

  // Add value a and make sure the length increased
  list_addValue(list, (void *)a);
  TEST_ASSERT_EQUAL_INT(1, list_getLength(list));
  // Add value b and make sure the length increased
  list_addValue(list, (void *)b);
  TEST_ASSERT_EQUAL_INT(2, list_getLength(list));
  // Add value c and make sure the length increased
  list_addValue(list, (void *)c);
  TEST_ASSERT_EQUAL_INT(3, list_getLength(list));

  // Read values out of order to ensure actual referencing works
  TEST_ASSERT_EQUAL_INT(b, (int)list_getValue(list, 1));
  TEST_ASSERT_EQUAL_INT(a, (int)list_getValue(list, 0));
  TEST_ASSERT_EQUAL_INT(c, (int)list_getValue(list, 2));

  // Remove b and make sure it is returned
  TEST_ASSERT_EQUAL_INT(b, (int)list_removeValue(list, 1));
  // Make sure the length was decreased
  TEST_ASSERT_EQUAL_INT(2, (int)list_getLength(list));

  // Read values again to ensure the right values are still available
  TEST_ASSERT_EQUAL_INT(a, (int)list_getValue(list, 0));
  TEST_ASSERT_EQUAL_INT(c, (int)list_getValue(list, 1));

  // Try to read out of bounds and make sure it fails
  TEST_ASSERT_NULL(list_getValue(list, 2));

  // Set existing value to a new value 1337
  TEST_ASSERT_EQUAL_INT(a, (int)list_setValue(list, 0, (void *)d));
  // Make sure the value hase changed
  TEST_ASSERT_EQUAL_INT(d, (int)list_getValue(list, 0));

  // Try to set value out of bounds
  TEST_ASSERT_NULL(list_setValue(list, 2, (void *)a));

  // Remove the last elemnt c to make sure tail is updated
  TEST_ASSERT_EQUAL_INT(c, (int)list_removeValue(list, 1));
  // Read the new tail
  TEST_ASSERT_EQUAL_INT(d, (int)list_getValue(list, 0));

  // Ensure that there are no elements left after clearing
  list_clear(list);
  TEST_ASSERT(list_getLength(list) == 0);

  list_free(list);
}

void list_test_run() {
  RUN_TEST(list_test_canStoreValues);
}
