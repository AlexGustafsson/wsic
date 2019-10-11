#include <unistd.h>

#include "unity/unity.h"

#include "../src/logging/logging.h"

void time_test_canFormatTimeBasedOnSecondsAndNanoseconds() {
  uint64_t nanoseconds = 0;
  uint64_t milliseconds = 0;
  uint64_t seconds = 0;
  uint64_t minutes = 0;
  uint64_t hours = 0;
  uint64_t days = 0;

  uint64_t secondsSinceStart = 258933;
  uint64_t nanosecondsSinceStart = 999440000;

  time_getFormattedElapsedTime(nanosecondsSinceStart, secondsSinceStart, &nanoseconds, &milliseconds, &seconds, &minutes, &hours, &days);

  TEST_ASSERT_EQUAL_UINT64(2, days);
  TEST_ASSERT_EQUAL_UINT64(23, hours);
  TEST_ASSERT_EQUAL_UINT64(55, minutes);
  TEST_ASSERT_EQUAL_UINT64(33, seconds);
  TEST_ASSERT_EQUAL_UINT64(999, milliseconds);
  TEST_ASSERT_EQUAL_UINT64(44, nanoseconds);
}

void time_test_canGetTimeSinceStart() {
  uint64_t seconds = 0;
  uint64_t nanoseconds = 0;

  TEST_ASSERT_TRUE(time_getTimeSinceStart(NULL, &seconds));

  TEST_ASSERT_EQUAL_INT(0, nanoseconds);
  TEST_ASSERT(seconds > 0);

  TEST_ASSERT_TRUE(time_getTimeSinceStart(&nanoseconds, NULL));

  TEST_ASSERT(nanoseconds > 0);
  TEST_ASSERT(seconds > 0);

  uint64_t seconds1 = 0;
  uint64_t nanoseconds1 = 0;
  TEST_ASSERT_TRUE(time_getTimeSinceStart(&nanoseconds1, &seconds1));

  TEST_ASSERT(nanoseconds1 > 0);
  TEST_ASSERT(seconds1 > 0);

  sleep(1);

  uint64_t seconds2 = 0;
  uint64_t nanoseconds2 = 0;
  TEST_ASSERT_TRUE(time_getTimeSinceStart(&nanoseconds2, &seconds2));

  TEST_ASSERT(nanoseconds2 > nanoseconds1);
  TEST_ASSERT(seconds2 > seconds1);

  TEST_ASSERT_FALSE(time_getTimeSinceStart(NULL, NULL));
}

void time_test_canResetTime() {
  time_reset();

  sleep(1);

  struct timespec now;
  time_getTimeSinceStartOfEpoch(&now);

  TEST_ASSERT(now.tv_sec > 1);
}

void time_test_canGetElapsedTime() {
  struct timespec now;
  sleep(1);
  time_getTimeSinceStartOfEpoch(&now);
  uint64_t nanoseconds = time_getElapsedTime(&now);

  TEST_ASSERT(nanoseconds > 1);
}

void time_test_run() {
  RUN_TEST(time_test_canFormatTimeBasedOnSecondsAndNanoseconds);
  RUN_TEST(time_test_canGetTimeSinceStart);
  RUN_TEST(time_test_canResetTime);
  RUN_TEST(time_test_canGetElapsedTime);
}
