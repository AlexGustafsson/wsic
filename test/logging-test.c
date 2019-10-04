#include "unity/unity.h"

#include "../src/logging/logging.h"

void logging_test_canFormatTimeBasedOnSecondsAndNanoseconds() {
  uint64_t nanoseconds = 0;
  uint64_t milliseconds = 0;
  uint64_t seconds = 0;
  uint64_t minutes = 0;
  uint64_t hours = 0;
  uint64_t days = 0;

  uint64_t secondsSinceStart = 258933;
  uint64_t nanosecondsSinceStart = 999440000;

  time_formatTime(nanosecondsSinceStart, secondsSinceStart, &nanoseconds, &milliseconds, &seconds, &minutes, &hours, &days);

  TEST_ASSERT_EQUAL_UINT64(2, days);
  TEST_ASSERT_EQUAL_UINT64(23, hours);
  TEST_ASSERT_EQUAL_UINT64(55, minutes);
  TEST_ASSERT_EQUAL_UINT64(33, seconds);
  TEST_ASSERT_EQUAL_UINT64(999, milliseconds);
  TEST_ASSERT_EQUAL_UINT64(44, nanoseconds);
}

void logging_test_run() {
  RUN_TEST(logging_test_canFormatTimeBasedOnSecondsAndNanoseconds);
}
