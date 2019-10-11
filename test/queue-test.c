#include "unity/unity.h"

#include "../src/datastructures/queue/queue.h"

void queue_test_canStoreValues() {
  queue_t *queue = queue_create();

  int a = 4;
  int b = 2;
  int c = 0;

  // Add value a and make sure the length increased
  queue_push(queue, (void *)a);
  TEST_ASSERT_EQUAL_INT(1, queue_getLength(queue));
  // Add value b and make sure the length increased
  queue_push(queue, (void *)b);
  TEST_ASSERT_EQUAL_INT(2, queue_getLength(queue));
  // Add value c and make sure the length increased
  queue_push(queue, (void *)c);
  TEST_ASSERT_EQUAL_INT(3, queue_getLength(queue));

  // Ensure that values are returned FIFO and the length decreased
  TEST_ASSERT_EQUAL_INT(a, (int)queue_pop(queue));
  TEST_ASSERT_EQUAL_INT(2, queue_getLength(queue));
  TEST_ASSERT_EQUAL_INT(b, (int)queue_pop(queue));
  TEST_ASSERT_EQUAL_INT(1, queue_getLength(queue));
  TEST_ASSERT_EQUAL_INT(c, (int)queue_pop(queue));
  TEST_ASSERT_EQUAL_INT(0, queue_getLength(queue));

  // Add another set of value and ensure that peeking does not change order
  queue_push(queue, (void *)a);
  queue_push(queue, (void *)b);
  TEST_ASSERT_EQUAL_INT(a, (int)queue_peek(queue));
  TEST_ASSERT_EQUAL_INT(a, (int)queue_pop(queue));
  TEST_ASSERT_EQUAL_INT(b, (int)queue_pop(queue));

  // Ensure that there are no elements left after clearing
  queue_clear(queue);
  TEST_ASSERT(queue_getLength(queue) == 0);

  queue_free(queue);
}

void queue_test_run() {
  RUN_TEST(queue_test_canStoreValues);
}
