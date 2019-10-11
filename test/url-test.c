#include "unity/unity.h"

#include "../src/url/url.h"

void url_test_canFormatAnURL() {
  url_t *url = url_create();

  url_setProtocol(url, string_fromBuffer("https"));
  url_setDomainName(url, string_fromBuffer("wsic.axgn.se"));
  url_setPort(url, 80);
  url_setPath(url, string_fromBuffer("/index.html"));
  url_setParameter(url, string_fromBuffer("q"), string_fromBuffer("test"));
  url_setParameter(url, string_fromBuffer("page"), string_fromBuffer("home"));

  string_t *urlString = url_toString(url);
  const char *expected = "https://wsic.axgn.se/index.html?q=test&page=home";

  TEST_ASSERT_EQUAL_STRING(expected, string_getBuffer(urlString));
  string_free(urlString);

  // Testing other port than standard 80
  url_setPort(url, 8443);

  urlString = url_toString(url);
  expected = "https://wsic.axgn.se:8443/index.html?q=test&page=home";

  TEST_ASSERT_EQUAL_STRING(expected, string_getBuffer(urlString));
  string_free(urlString);

  url_free(url);
}

void url_test_canGetAndSetParameters() {
  url_t *url = url_create();
  string_t *key = string_fromBuffer("username");
  string_t *value = string_fromBuffer("xXMasterHackerXx");

  url_setParameter(url, key, value);

  TEST_ASSERT_EQUAL_STRING("xXMasterHackerXx", string_getBuffer(url_getParameter(url, key)));

  url_free(url);
}

void url_test_canGetAndSetDomainName() {
  url_t *url = url_create();

  // Set domain name
  url_setDomainName(url, string_fromBuffer("localhost"));
  // Try to get the domain name
  TEST_ASSERT_EQUAL_STRING("localhost", string_getBuffer(url_getDomainName(url)));

  // Try uppdate the domain name
  url_setDomainName(url, string_fromBuffer("wsic.axgn.se"));
  // Try to get the new domain name
  TEST_ASSERT_EQUAL_STRING("wsic.axgn.se", string_getBuffer(url_getDomainName(url)));

  url_free(url);
}

void url_test_canGetAndSetProtocol() {
  url_t *url = url_create();

  // Set protocol
  url_setProtocol(url, string_fromBuffer("http"));
  // Try to get the protocol
  TEST_ASSERT_EQUAL_STRING("http", string_getBuffer(url_getProtocol(url)));

  // Try uppdate the protocol
  url_setProtocol(url, string_fromBuffer("https"));
  // Try to get the new protocol
  TEST_ASSERT_EQUAL_STRING("https", string_getBuffer(url_getProtocol(url)));

  url_free(url);
}

void url_test_canCreateQueryString() {
  url_t *url = url_create();
  url_setParameter(url, string_fromBuffer("username"), string_fromBuffer("test"));
  url_setParameter(url, string_fromBuffer("page"), string_fromBuffer("home"));

  string_t *result = url_toQueryString(url);
  char *expectedResult = "username=test&page=home";

  TEST_ASSERT_EQUAL_STRING(expectedResult, string_getBuffer(result));

  string_free(result);
  url_free(url);
}

void url_test_run() {
  RUN_TEST(url_test_canFormatAnURL);
  RUN_TEST(url_test_canGetAndSetParameters);
  RUN_TEST(url_test_canGetAndSetDomainName);
  RUN_TEST(url_test_canGetAndSetProtocol);
  RUN_TEST(url_test_canCreateQueryString);
}
