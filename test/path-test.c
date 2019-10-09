#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "unity/unity.h"

#include "../src/path/path.h"

void path_test_canResolvePathThatExists() {
  // The path that we want to GET
  string_t *relativePath = string_fromBuffer("/index.html");
  // Working directory where www is located
  string_t *root = string_fromBuffer(realpath("www", NULL));
  // The result
  string_t *path = path_resolve(relativePath, root);
  string_free(relativePath);
  string_free(root);

  // Get current working direcorty
  char cwd[1024] = {0};
  getcwd(cwd, 1024);
  // Append the file we expect to find to cwd
  strcat(cwd, "/www/index.html");

  TEST_ASSERT_NOT_NULL(path);
  TEST_ASSERT_EQUAL_STRING(cwd, string_getBuffer(path));
  string_free(path);
}

void path_test_cannotResolvePathThatDoesNotExists() {
  // The path that we want to GET
  string_t *relativePath = string_fromBuffer("/howToHackNSA.html");
  // Working directory where www is located
  string_t *root = string_fromBuffer(realpath("www", NULL));
  // The result
  string_t *path = path_resolve(relativePath, root);
  string_free(relativePath);
  string_free(root);

  TEST_ASSERT_NULL(path);

  if (path != 0)
    string_free(path);
}

void path_test_canResolvePathAbsoluteToRelativePath() {
  string_t *path = string_fromBuffer("www/index.html");
  string_t *root = string_fromBuffer(realpath("www", NULL));
  string_t *result = path_relativeTo(path, root);
  string_free(path);
  string_free(root);

  TEST_ASSERT_NOT_NULL(result);
  TEST_ASSERT_EQUAL_STRING("/index.html", string_getBuffer(result));

  if (result != 0)
    string_free(result);
}

void path_test_run() {
  RUN_TEST(path_test_canResolvePathThatExists);
  RUN_TEST(path_test_cannotResolvePathThatDoesNotExists);
  RUN_TEST(path_test_canResolvePathAbsoluteToRelativePath);
}
