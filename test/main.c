#include "unity/unity.h"

#include "../src/logging/logging.h"

#include "config-test.c"
#include "hash-table-test.c"
#include "http-test.c"
#include "list-test.c"
#include "queue-test.c"
#include "response-codes-test.c"
#include "set-test.c"
#include "string-test.c"
#include "time-test.c"
#include "url-test.c"
#include "www-test.c"

int main() {
  //LOGGING_OUTPUT = 0;

  UNITY_BEGIN();

  list_test_run();
  string_test_run();
  url_test_run();
  queue_test_run();
  set_test_run();
  hash_table_test_run();
  http_test_run();
  response_codes_test_run();
  www_test_run();
  config_test_run();
  time_test_run();

  return UNITY_END();
}
