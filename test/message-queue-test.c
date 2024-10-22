#include "unity/unity.h"

#include "../src/datastructures/message-queue/message-queue.h"

void *thread_job(message_queue_t *queue) {
  return message_queue_pop(queue);
}

void message_queue_test_canPushAndPop() {
  message_queue_t *queue = message_queue_create();

  pthread_t thread0;
  if (pthread_create(&thread0, NULL, (void *(*)(void *))thread_job, queue) != 0) {
    TEST_FAIL_MESSAGE("Thread failed to initialize");
    message_queue_free(queue);
    return;
  }

  pthread_t thread1;
  if (pthread_create(&thread1, NULL, (void *(*)(void *))thread_job, queue) != 0) {
    TEST_FAIL_MESSAGE("Thread failed to initialize");
    message_queue_free(queue);
    return;
  }

  int value0 = 10;
  int value1 = 20;

  message_queue_push(queue, &value0);
  message_queue_push(queue, &value1);

  int *result0 = 0;
  int *result1 = 0;

  pthread_join(thread0, (void **)&result0);
  pthread_join(thread1, (void **)&result1);

  TEST_ASSERT_NOT_NULL(result0);
  TEST_ASSERT_NOT_NULL(result1);

  TEST_ASSERT(*result0 != *result1);
  TEST_ASSERT(*result0 == value0 || *result0 == value1);
  TEST_ASSERT(*result1 == value0 || *result1 == value1);

  message_queue_free(queue);
}

void messsage_queue_test_canUnlockQueue() {
  message_queue_t *queue = message_queue_create();

  pthread_t thread0;
  if (pthread_create(&thread0, NULL, (void *(*)(void *))thread_job, queue) != 0) {
    message_queue_free(queue);
    TEST_FAIL_MESSAGE("Thread failed to initialize");
  }

  pthread_t thread1;
  if (pthread_create(&thread1, NULL, (void *(*)(void *))thread_job, queue) != 0) {
    message_queue_free(queue);
    TEST_FAIL_MESSAGE("Thread failed to initialize");
  }

  message_queue_unlock(queue);

  int *result0 = 0;
  int *result1 = 0;

  pthread_join(thread0, (void **)&result0);
  pthread_join(thread1, (void **)&result1);

  TEST_ASSERT_NULL(result0);
  TEST_ASSERT_NULL(result1);

  message_queue_free(queue);
}

void message_queue_test_run() {
  RUN_TEST(message_queue_test_canPushAndPop);
  RUN_TEST(messsage_queue_test_canUnlockQueue);
}
