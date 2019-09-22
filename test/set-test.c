#include "unity/unity.h"

#include "../src/datastructures/set/set.h"

void set_test_canStoreValues() {
  set_t *set = set_create();

  int a = 4;
  int b = 2;
  int c = 2;
  int d = 0;
  int e = 4;

  // Add value a and make sure the length increased
  set_addValue(set, (void *)a);
  TEST_ASSERT_EQUAL_INT(1, set_getLength(set));
  // Add value b and make sure the length increased
  set_addValue(set, (void *)b);
  TEST_ASSERT_EQUAL_INT(2, set_getLength(set));
  // Add value c and make sure the length remains unchanged (c and b are the same)
  set_addValue(set, (void *)c);
  TEST_ASSERT_EQUAL_INT(2, set_getLength(set));
  // Add value d and make sure the length increased
  set_addValue(set, (void *)d);
  TEST_ASSERT_EQUAL_INT(3, set_getLength(set));
  // Add value c and make sure the length remains unchanged (e and a are the same)
  set_addValue(set, (void *)e);
  TEST_ASSERT_EQUAL_INT(3, set_getLength(set));

  // Read values out of order to ensure actual referencing works
  TEST_ASSERT_EQUAL_INT(b, (int)set_getValue(set, 1));
  TEST_ASSERT_EQUAL_INT(a, (int)set_getValue(set, 0));
  TEST_ASSERT_EQUAL_INT(d, (int)set_getValue(set, 2));

  // Remove b and make sure it is returned
  TEST_ASSERT_EQUAL_INT(b, (int)set_removeValue(set, 1));
  // Make sure the length was decreased
  TEST_ASSERT_EQUAL_INT(2, (int)set_getLength(set));

  // Read values again to ensure the right values are still available
  TEST_ASSERT_EQUAL_INT(a, (int)set_getValue(set, 0));
  TEST_ASSERT_EQUAL_INT(d, (int)set_getValue(set, 1));

  // Try to read out of bounds and make sure it fails
  TEST_ASSERT_NULL(set_getValue(set, 2));

  // Ensure that there are no elements left after clearing
  set_clear(set);
  TEST_ASSERT(set_getLength(set) == 0);

  set_free(set);
}

void set_test_run() {
  RUN_TEST(set_test_canStoreValues);
}
