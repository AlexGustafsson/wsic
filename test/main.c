#include "unity/unity.h"

#include "list-test.c"
#include "string-test.c"
#include "url-test.c"

int main() {
  UNITY_BEGIN();

  list_test_run();
  string_test_run();
  url_test_run();

  return UNITY_END();
}
