#include "unity/unity.h"

#include "../src/url/url.h"

void url_test_canFormatAnURL() {
  url_t *url = url_create();

  url_setProtocol(url, string_fromCopy("https"));
  url_setDomainName(url, string_fromCopy("wsic.axgn.se"));
  url_setPort(url, 80);
  url_setPath(url, string_fromCopy("/index.html"));
  url_setParameter(url, string_fromCopy("q"), string_fromCopy("test"));
  url_setParameter(url, string_fromCopy("page"), string_fromCopy("home"));
  url_setFragment(url, string_fromCopy("test"));

  string_t *urlString = url_toString(url);
  const char *expected = "https://wsic.axgn.se/index.html?q=test&page=home#test";

  TEST_ASSERT_EQUAL_STRING(expected, string_getBuffer(urlString));

  url_free(url);
}

void url_test_run() {
  RUN_TEST(url_test_canFormatAnURL);
}
