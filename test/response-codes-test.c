#include <stdint.h>

#include "unity/unity.h"

#include "../src/http/response-codes.h"

void response_codes_test_canParseCodeToString() {
  // All response codes in order
  uint16_t responseCodes[] = {100,101,102,103,200,201,202,203,204,205,206,207,208,226,300,301,302,303,304,305,306,307,308,400,401,402,403,404,405,406,407,408,409,410,411,412,413,414,415,416,417,418,421,422,423,424,425,426,428,429,431,451,500,501,502,503,504,505,506,507,508,510,511,666,0};
  // All matching response texts in order
  char *expectedResponseCodeTexts[] = {"Continue", "Switching Protocol", "Processing", "Early Hints", "OK", "Created", "Accepted", "Non-Authoritative Information", "No Content", "Reset Content", "Partial Content", "Multi-Status", "Multi-Status", "IM Used", "Multiple Choice", "Moved Permanently", "Found", "See Other", "Not Modified", "Use Proxy", "unused", "Temporary Redirect", "Permanent Redirect", "Bad Request", "Unauthorized", "Payment Required", "Forbidden", "Not Found", "Method Not Allowed", "Not Acceptable", "Proxy Authentication Required", "Request Timeout", "Conflict", "Gone", "Length Required", "Precondition Failed", "Payload Too Large", "URI Too Long", "Unsupported Media Type", "Requested Range Not Satisfiable", "Expectation Failed", "I'm a teapot", "Misdirected Request", "Unprocessable Entity", "Locked", "Failed Dependency", "Too Early", "Upgrade Required", "Precondition Required", "Too Many Requests", "Request Header Fields Too Large", "Unavailable For Legal Reasons", "Internal Server Error", "Not Implemented", "Bad Gateway", "Service Unavailable", "Gateway Timeout", "HTTP Version Not Supported", "Variant Also Negotiates", "Insufficient Storage", "Loop Detected", "Not Extended", "Network Authentication Required", "", 0};
  string_t *responseCodeText = string_create();
  size_t iterator = 0;

  // Try for each element in responseCodes to see if it matches expectedResponseCodeTexts
  while (expectedResponseCodeTexts[iterator] != 0) {
    // Calculate response
    responseCodeText = http_codeToString(responseCodes[iterator]);
    // Test calculate response against expected value
    TEST_ASSERT_EQUAL_STRING(expectedResponseCodeTexts[iterator], string_getBuffer(responseCodeText));
    string_free(responseCodeText);
    iterator++;
  }
}

void response_codes_test_run() {
  RUN_TEST(response_codes_test_canParseCodeToString);
}
