#include "unity/unity.h"

#include "../src/http/http.h"

void http_test_canParseRequestLine() {
  http_t *http = http_create();
  string_t *request = string_fromCopy("GET /index.html HTTP/1.1");

  // Can parse initial request
  TEST_ASSERT(http_parseRequestLine(http, request) == true);
  TEST_ASSERT_EQUAL_INT(http->method, 1);
  TEST_ASSERT_EQUAL_STRING(http->url->path->buffer, "/index.html");
  TEST_ASSERT_EQUAL_STRING(http->version->buffer, "1.1");
  string_free(request);

  // Should not be able to parse only a METHOD
  string_t *request1 = string_fromCopy("POST");
  TEST_ASSERT(http_parseRequestLine(http, request1) == false);
  string_free(request1);

  // Should not be able to only parse method and path
  string_t *request2 = string_fromCopy("HEAD /index.php");
  TEST_ASSERT(http_parseRequestLine(http, request2) == false);
  string_free(request2);

  // Should be able to parse path ending with ?
  string_t *request3 = string_fromCopy("HEAD /index.php? HTTP/1.1");
  TEST_ASSERT(http_parseRequestLine(http, request3) == true);
  string_free(request3);

  // Should not be able to parse incorect url parameters
  string_t *request4 = string_fromCopy("HEAD /index.php?test=10&& HTTP/1.1");
  TEST_ASSERT(http_parseRequestLine(http, request4) == false);
  string_free(request4);

  // Should not be able to parse incorect url parameters
  string_t *request5 = string_fromCopy("HEAD /index.php?test10&& HTTP/1.1");
  TEST_ASSERT(http_parseRequestLine(http, request5) == false);
  string_free(request5);

  // Should not be able to parse incorect url parameters
  string_t *request6 = string_fromCopy("HEAD /index.php?test=10&server=wsic HTTP/1.1");
  TEST_ASSERT(http_parseRequestLine(http, request6) == true);
  string_free(request6);

  // Trying to parse incorect http version
  string_t *request7 = string_fromCopy("HEAD /index.php TTP/1.1");
  TEST_ASSERT(http_parseRequestLine(http, request7) == false);
  string_free(request7);

  // Trying to parse missing chars in version
  string_t *request8 = string_fromCopy("HEAD /index.php HTTP/");
  TEST_ASSERT(http_parseRequestLine(http, request8) == false);
  string_free(request8);

  // Trying to parse wrong chars in version
  string_t *request9 = string_fromCopy("HEAD /index.php HTTP/aaa");
  TEST_ASSERT(http_parseRequestLine(http, request9) == false);
  string_free(request9);

  // Trying to parse wrong chars in version
  string_t *request10 = string_fromCopy("HEAD /index.php HTTP/11111");
  TEST_ASSERT(http_parseRequestLine(http, request10) == false);
  string_free(request10);

  // Trying to parse wrong chars in version
  string_t *request11 = string_fromCopy("HEAD /index.php HTTP/1");
  TEST_ASSERT(http_parseRequestLine(http, request11) == false);
  string_free(request11);

  //request = string_fromCopy("HTTP/1.0 200 OK");
  //TEST_ASSERT(http_parseRequestLine(http, request) == false);
  //string_free(request);

  http_free(http);
}

void http_test_canGetAndSetMethod() {
  http_t *http = http_create();

  // Make sure its empty
  TEST_ASSERT(http_getMethod(http) == 0);
  // Set the method to HEAD 4
  http_setMethod(http, HTTP_METHOD_HEAD);
  // Get the method
  TEST_ASSERT(http_getMethod(http) == 4);

  http_free(http);
}

void http_test_canGetAndSetVersion() {
  http_t *http = http_create();

  // Make sure its empty
  TEST_ASSERT(http_getVersion(http) == 0);
  // Set the method to HEAD 4
  http_setVersion(http, string_fromCopy("1.5"));
  // Get the method
  TEST_ASSERT(string_equalsBuffer(http_getVersion(http), "1.5"));

  http_free(http);
}

void http_test_canGetAndSetResponseCode() {
  http_t *http = http_create();

  // Make sure its empty
  TEST_ASSERT(http_getResponseCode(http) == 0);
  // Set the method to HEAD 4
  http_setResponseCode(http, 404);
  // Get the method
  TEST_ASSERT(http_getResponseCode(http) == 404);

  http_free(http);
}

void http_test_canGetAndSetHeader() {
  http_t *http = http_create();
  string_t *key = string_fromCopy("Host");
  string_t *value = string_fromCopy("localhost:8080");

  // Try get on a key that does not exist
  //TEST_ASSERT(http_getHeader(http, key) == 0);

  // Try to set a header
  http_setHeader(http, key, value);

  // Try to get the set key
  TEST_ASSERT(string_equals(http_getHeader(http, key), value));

  // Try update a key with new value
  string_t *sameKeyAsBefore = string_fromCopy("Host");
  string_t *newValue = string_fromCopy("wsic.axgn.se:8443");
  http_setHeader(http, sameKeyAsBefore, newValue);
  TEST_ASSERT(string_equals(http_getHeader(http, sameKeyAsBefore), newValue));

  http_free(http);
}

void http_test_canGetUrl() {
  http_t *http = http_create();
  http->url = url_create();

  TEST_ASSERT(http->url == http_getUrl(http));

  http_free(http);
}

void http_test_canCreateResponseString() {
  http_t *http = http_create();
  http_setVersion(http, string_fromCopy("1.0"));
  http_setResponseCode(http, 200);
  http_setHeader(http, string_fromCopy("Host"), string_fromCopy("wsic.axgn.se:8443"));
  http_setHeader(http, string_fromCopy("Server"), string_fromCopy("wsic"));
  http_setBody(http, string_fromCopy("This is a test response!"));
  string_t *responsString = http_toResponseString(http);

  string_t *expectedResult = string_fromCopy("HTTP/1.0 200 OK\r\nHost: wsic.axgn.se:8443\r\nServer: wsic\r\nContent-Length: 24\r\n\r\nThis is a test response!");

  TEST_ASSERT_EQUAL_STRING(expectedResult->buffer, responsString->buffer);

  string_free(responsString);
  string_free(expectedResult);
  http_free(http);
}

void http_test_canParseHttpMethod() {
  string_t *methodGet = string_fromCopy("GET");
  string_t *methodPut = string_fromCopy("PUT");
  string_t *methodPost = string_fromCopy("POST");
  string_t *methodHead = string_fromCopy("HEAD");
  string_t *methodOptions = string_fromCopy("OPTIONS");
  string_t *methodUnknown = string_fromCopy("CAKE");

  TEST_ASSERT_EQUAL_UINT8(1, http_parseMethod(methodGet));
  TEST_ASSERT_EQUAL_UINT8(2, http_parseMethod(methodPut));
  TEST_ASSERT_EQUAL_UINT8(3, http_parseMethod(methodPost));
  TEST_ASSERT_EQUAL_UINT8(4, http_parseMethod(methodHead));
  TEST_ASSERT_EQUAL_UINT8(5, http_parseMethod(methodOptions));
  TEST_ASSERT_EQUAL_UINT8(0, http_parseMethod(methodUnknown));

  string_free(methodGet);
  string_free(methodPut);
  string_free(methodPost);
  string_free(methodHead);
  string_free(methodOptions);
  string_free(methodUnknown);
}

void http_test_canParseHttpRequestBody() {
  http_t *http = http_create();
  string_t *body = string_fromCopy("HTTP/1.0 200 OK\r\nHost: wsic.axgn.se:8443\r\nServer: wsic\r\n\r\nThis is a test body!");
  ssize_t offset = 58;

  http_parseBody(http, body, offset);

  TEST_ASSERT_EQUAL_STRING("This is a test body!", string_getBuffer(http->body));

  http_free(http);
  string_free(body);
}

void http_test_canParseHttpHeaders() {
  http_t *http = http_create();

  // Normal header that sound parse just fine
  string_t *header1 = string_fromCopy("Host: wsic.axgn.se:8443");

  // Try to parse the headers, should work
  TEST_ASSERT(http_parseHeader(http, header1) == true);
  string_free(header1);

  // Trying to find a value:'wsic.axgn.se:8443' with the key: 'Host', should work
  string_t *keyHost = string_fromCopy("Host");
  TEST_ASSERT_EQUAL_STRING("wsic.axgn.se:8443", string_getBuffer(http_getHeader(http, keyHost)));
  string_free(keyHost);

  // Trying to parse a header missing :, should not work
  string_t *header2 = string_fromCopy("Server wsic");
  TEST_ASSERT(http_parseHeader(http, header2) == false);

  if (header2 != 0)
    string_free(header2);

  // Trying to parse a header missing the space after :, should not work
  string_t *header3 = string_fromCopy("Date:1878-12-18");
  TEST_ASSERT(http_parseHeader(http, header3) == false);
  if (header3 != 0)
    string_free(header3);

  http_free(http);
}

void http_test_canParseHost() {
  /*** PARSE NORMAL HOST ***/
  http_t *http = http_create();

  // Should not parse when host header is not set
  TEST_ASSERT(http_parseHost(http) == false);

  // Values to apped to header
  string_t *key = string_fromCopy("Host");
  string_t *value = string_fromCopy("localhost");

  // Set the Host header
  http_setHeader(http, key, value);
  // Should be able to parse host
  TEST_ASSERT(http_parseHost(http) == true);
  // Should find that domainname is set to localhost
  TEST_ASSERT_EQUAL_STRING("localhost", string_getBuffer(url_getDomainName(http->url)));
  // Should find that port is 80
  TEST_ASSERT_EQUAL_UINT16(80, url_getPort(http->url));
  http_free(http);

  /*** PARSE HOST WITH PORT OTHER THAN 80 ***/
  http_t *http1 = http_create();
  // Values to apped to header
  string_t *key1 = string_fromCopy("Host");
  string_t *value1 = string_fromCopy("localhost:8443");

  // Set the Host header
  http_setHeader(http1, key1, value1);
  // Should be able to parse host
  TEST_ASSERT(http_parseHost(http1) == true);
  // Should find that domainname is set to localhost
  TEST_ASSERT_EQUAL_STRING("localhost", string_getBuffer(url_getDomainName(http1->url)));
  // Should find that port is 80
  TEST_ASSERT_EQUAL_UINT16(8443, url_getPort(http1->url));
  http_free(http1);

  /*** PARSE HOST WITH INVALID PORT ***/
  http_t *http2 = http_create();
  // Values to apped to header
  string_t *key2 = string_fromCopy("Host");
  string_t *value2 = string_fromCopy("localhost:123456789");

  // Set the Host header
  http_setHeader(http2, key2, value2);
  // Should not parse when port is out of range
  TEST_ASSERT(http_parseHost(http2) == false);
  http_free(http2);
}

void http_test_canGetAndSetBody() {
  http_t *http = http_create();
  string_t *key = string_fromCopy("Content-Length");
  string_t *body = string_fromCopy("Lorem ipsum dolor sit amet, consectetur adipiscing elit. In rutrum metus eget metus molestie cursus. Quisque tristique sed erat ac suscipit. Ut viverra, dui eu porta porttitor, lectus nisl gravida lectus, non tristique diam mauris vitae ex. Praesent porta posuere augue, eu elementum mauris. Sed tempor urna sed justo viverra, sit amet hendrerit odio dictum. Suspendisse molestie eu tellus a hendrerit. Vestibulum ultricies enim ut est molestie semper. Pellentesque accumsan sit amet enim a vulputate.");
  string_t *expectedContentLength = string_fromCopy("501");

  http_setBody(http, body);
  TEST_ASSERT_EQUAL_STRING(string_getBuffer(body), string_getBuffer(http_getBody(http)));
  TEST_ASSERT_EQUAL_STRING(string_getBuffer(expectedContentLength), string_getBuffer(http_getHeader(http, key)));
  string_free(expectedContentLength);

  string_t *body2 = string_fromCopy("A hard problem and the associated back door for the NTRU Public Key Cryptosystem is described and compared/contrasted with the hard problems and back doors associated to other common public key cryptosystems.");
  string_t *expectedContentLength2 = string_fromCopy("208");
  http_setBody(http, body2);
  TEST_ASSERT_EQUAL_STRING(string_getBuffer(body2), string_getBuffer(http_getBody(http)));
  TEST_ASSERT_EQUAL_STRING(string_getBuffer(expectedContentLength2), string_getBuffer(http_getHeader(http, key)));
  string_free(expectedContentLength2);
  
  string_free(key);
  http_free(http);
}

void http_test_run() {
  RUN_TEST(http_test_canParseRequestLine);
  RUN_TEST(http_test_canGetAndSetMethod);
  RUN_TEST(http_test_canGetAndSetVersion);
  RUN_TEST(http_test_canGetAndSetResponseCode);
  RUN_TEST(http_test_canGetAndSetHeader);
  RUN_TEST(http_test_canGetUrl);
  RUN_TEST(http_test_canCreateResponseString);
  RUN_TEST(http_test_canParseHttpMethod);
  RUN_TEST(http_test_canParseHttpRequestBody);
  RUN_TEST(http_test_canParseHttpHeaders);
  RUN_TEST(http_test_canParseHost);
  RUN_TEST(http_test_canGetAndSetBody);
}
