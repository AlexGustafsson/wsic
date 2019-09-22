#include "unity/unity.h"

#include "hash-table-test.c"
#include "list-test.c"
#include "queue-test.c"
#include "set-test.c"
#include "string-test.c"
#include "url-test.c"

int main() {
  UNITY_BEGIN();

  list_test_run();
  string_test_run();
  url_test_run();
  queue_test_run();
  set_test_run();
  hash_table_test_run();

  return UNITY_END();
}
