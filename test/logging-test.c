#include <string.h>

#include "unity/unity.h"

#include "../src/logging/logging.h"

void logging_test_canLogFile() {
  FILE *logfileWrite = fopen("build/logging.test.file.log", "a");
  logging_logToFile(logfileWrite, LOG_LABEL_7, LOG_COLOR_7, "logging-test.c", 12, "logging_test_canLogFile", "This is a test log");
  fclose(logfileWrite);

  FILE *logfileRead = fopen("build/logging.test.file.log", "r");

  char buffer[1024] = {0};
  char *result = 0;

  while (fgets(buffer, 1024, logfileRead) != 0) {
    result = strstr(buffer, "This is a test log");
    if (result != 0)
      break;
  }

  TEST_ASSERT_NOT_NULL(result);
}

void logging_test_canStartAndStop() {
  bool loggingStarted = logging_start();
  TEST_ASSERT_TRUE(loggingStarted);

  bool loggingFileOpened = logging_openOutputFile("build/logging.test.logfile.log");
  TEST_ASSERT_TRUE(loggingFileOpened);

  int fileClosed = EOF;
  fileClosed = logging_stop();
  TEST_ASSERT_EQUAL_INT(0, fileClosed);
}

void logging_test_canLogRequest() {
  bool loggingStarted = logging_start();
  TEST_ASSERT_TRUE(loggingStarted);

  bool loggingFileOpened = logging_openOutputFile("build/logging.test.request.log");
  TEST_ASSERT_TRUE(loggingFileOpened);

  string_t *remoteHost = string_fromBuffer("localhost");
  string_t *path = string_fromBuffer("/index.html");
  string_t *version = string_fromBuffer("1.1");
  logging_request(remoteHost, HTTP_METHOD_GET, path, version, 200, 700);
  string_free(remoteHost);
  string_free(path);
  string_free(version);

  int fileClosed = EOF;
  fileClosed = logging_stop();
  TEST_ASSERT_EQUAL_INT(0, fileClosed);

  FILE *logfileRead = fopen("build/logging.test.request.log", "r");

  char buffer[1024] = {0};
  char *result = 0;

  while (fgets(buffer, 1024, logfileRead) != 0) {
    result = strstr(buffer, "\"GET /index.html HTTP/1.1\" 200 700");
    if (result != 0)
      break;
  }

  TEST_ASSERT_NOT_NULL(result);

  fclose(logfileRead);
}

void logging_test_run() {
  RUN_TEST(logging_test_canLogFile);
  RUN_TEST(logging_test_canStartAndStop);
  RUN_TEST(logging_test_canLogRequest);
}
