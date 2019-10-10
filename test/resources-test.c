#include <stdlib.h>

#include "unity/unity.h"

#include "../src/resources/resources.h"

void resources_test_canLoadFile() {
  string_t *path = string_fromBuffer(realpath("test/resources-test.c", NULL));
  string_t *result = resources_loadFile(path);
  string_cursor_t *cursor = string_createCursor(result);
  string_t *firstRowInResult = string_getNextLine(cursor);

  TEST_ASSERT_EQUAL_STRING("#include <stdlib.h>", string_getBuffer(firstRowInResult));

  string_free(path);
  string_free(result);
  string_freeCursor(cursor);
  string_free(firstRowInResult);
}

void resources_test_cannotLoadFileThatDoesNotExist() {
  string_t *path = string_fromBuffer("test/filedoesnotexist.c");
  string_t *result = resources_loadFile(path);

  TEST_ASSERT_NULL(result);

  string_free(path);
}

void resources_test_canGetMIMEType() {
  string_t *path = string_fromBuffer("index.html");
  string_t *result = resources_getMIMEType(path);

  TEST_ASSERT_EQUAL_STRING("text/html", string_getBuffer(result));

  string_free(path);
  string_free(result);
}

void resources_test_cannotGetInvalidMIMEType() {
  string_t *path = string_fromBuffer("index");
  string_t *result = resources_getMIMEType(path);
  TEST_ASSERT_NULL(result);
  string_free(path);
  if (result != 0)
    string_free(result);

  string_t *path1 = string_fromBuffer("index.mp4");
  string_t *result1 = resources_getMIMEType(path1);
  TEST_ASSERT_NULL(result1);
  string_free(path1);
  if (result1 != 0)
    string_free(result1);

  string_t *path2 = string_fromBuffer("index.tar.gz");
  string_t *result2 = resources_getMIMEType(path2);
  TEST_ASSERT_NULL(result2);
  string_free(path2);
  if (result2 != 0)
    string_free(result2);
}

void resources_test_canIsExecutable() {
  string_t *path = string_fromBuffer(realpath("build/wsic.test", NULL));
  bool result = resources_isExecutable(path);

  TEST_ASSERT_TRUE(result);

  string_free(path);

  string_t *path1 = string_fromBuffer(realpath("test/resources-test.c", NULL));
  bool result1 = resources_isExecutable(path1);

  TEST_ASSERT_FALSE(result1);

  string_free(path1);
}

void resources_test_canIsFile() {
  string_t *path = string_fromBuffer(realpath("build/wsic.test", NULL));
  bool result = resources_isFile(path);

  TEST_ASSERT_TRUE(result);

  string_free(path);

  string_t *path1 = string_fromBuffer(realpath("test/resources-test.c", NULL));
  bool result1 = resources_isFile(path1);

  TEST_ASSERT_TRUE(result1);

  string_free(path1);
}

void resources_test_run() {
  RUN_TEST(resources_test_canLoadFile);
  RUN_TEST(resources_test_cannotLoadFileThatDoesNotExist);
  RUN_TEST(resources_test_canGetMIMEType);
  RUN_TEST(resources_test_cannotGetInvalidMIMEType);
  RUN_TEST(resources_test_canIsExecutable);
  RUN_TEST(resources_test_canIsFile);
}
