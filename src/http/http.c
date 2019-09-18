#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../logging/logging.h"

#include "http.h"

// Forward declaration of functions (For internal use Only)
bool parseRequestLine(http_t *http, string_t *string);
bool parseHeader(http_t *http, string_t *string);
void parseBody(http_t *http, string_t *string, size_t offset);

http_t *http_create() {
  http_t *http = malloc(sizeof(http_t));
  if (http == 0)
    return 0;

  memset(http, 0, sizeof(http_t));

  http->headers = hash_table_create();
  if (http->headers == 0) {
    free(http);
    return 0;
  }

  return http;
}

void http_setMethod(http_t *http, enum httpMethod method) {
  http->method = method;
}

void http_setRequestTarget(http_t *http, string_t *requestTarget) {
  if (http->requestTarget != 0)
    string_free(http->requestTarget);

  http->requestTarget = requestTarget;
}

void http_setVersion(http_t *http, string_t *version) {
  if (http->version != 0)
    string_free(http->version);

  http->version = version;
}

void http_setResponsCode(http_t *http, uint16_t code) {
  http->responseCode = code;
  http->responseCodeText = http_codeToString(code);
}

void http_setHeader(http_t *http, string_t *key, string_t *value) {
  hash_table_setValue(http->headers, key, value);
}

string_t *http_ToString(http_t *http) {
  string_t *result = string_create();

  if (http->version != 0) {
    string_appendBuffer(result, "HTTP/");
    string_append(result, http->version);
    string_appendChar(result, ' ');
  }

  char responseCodeBuffer[4] = {0};
  sprintf(responseCodeBuffer, "%d", http->responseCode);
  size_t responseCodeLength = strlen(responseCodeBuffer);

  string_appendBufferWithLength(result, responseCodeBuffer, responseCodeLength);
  string_appendChar(result, ' ');

  if (http->responseCodeText != 0) {
    string_append(result, http->responseCodeText);
  }

  string_appendBuffer(result, "\r\n");

  size_t headers = hash_table_getLength(http->headers);
  for (size_t i = 0; i < headers; i++) {
    string_t *key = (string_t *)hash_table_getKeyByIndex(http->headers, i);
    string_t *value = (string_t *)hash_table_getValueByIndex(http->headers, i);
    string_append(result, key);
    string_appendBuffer(result, ": ");
    string_append(result, value);
    string_appendBuffer(result, "\r\n");
  }

  // End of headers
  string_appendBuffer(result, "\r\n");

  string_append(result, http->body);
  // End of body
  string_appendBuffer(result, "\r\n\r\n");

  return result;
}

http_t *http_parseRequest(string_t *request) {
  bool correctlyParsed = false;
  http_t *http = http_create();
  http->requestTarget = string_create();
  http->version = string_create();
  string_cursor_t *cursor = string_createCursor(request);
  // First line of the request
  string_t *line = string_getNextLine(cursor);
  if (line == 0) {
    log(LOG_ERROR, "Can not parse empty request");
    http_free(http);
    return 0;
  }
  // Parse the first line in the request (METHOD, PATH, VERSION)
  correctlyParsed = parseRequestLine(http, line);
  if (correctlyParsed == false) {
    log(LOG_ERROR, "Could not parse request");
    return 0;
  }
  log(LOG_DEBUG, "%d %s %s", http->method, string_getBuffer(http->requestTarget), string_getBuffer(http->version));
  // Parse all headers (KEY: VALUES)
  while ((line = string_getNextLine(cursor)) != 0) {
    if (string_getSize(line) == 0) {
      string_free(line);
      break;
    }

    correctlyParsed = parseHeader(http, line);
    if (correctlyParsed == false) {
      log(LOG_ERROR, "Could not parse request");
      return 0;
    }
    string_free(line);
  }

  parseBody(http, request, string_getOffset(cursor));
  log(LOG_DEBUG, "%s", string_getBuffer(http->body));

  string_freeCursor(cursor);
  return http;
}

bool parseRequestLine(http_t *http, string_t *string) {
  char current = 0;
  string_t *tempString = string_create();
  string_cursor_t *cursor = string_createCursor(string);

  // Parse method
  // Reads untill a space is found or null char at end on line
  while ((current = string_getNextChar(cursor)) != ' ' && current != 0)
    string_appendChar(tempString, current);

  if (current == 0) {
    log(LOG_ERROR, "Could not parse request:method");
    return false;
  }

  http->method = http_parseMethod(tempString);
  string_free(tempString);

  // Parse path
  // Reads untill a space is found or null char at end on line
  while ((current = string_getNextChar(cursor)) != ' ' && current != 0)
    string_appendChar(http->requestTarget, current);

  if (current == 0) {
    log(LOG_ERROR, "Could not parse request:target");
    return false;
  }

  // Parse version
  // Reads untill null char at end on line
  while ((current = string_getNextChar(cursor)) != 0)
    string_appendChar(http->version, current);
  // Free cursor, not in use anny more
  string_freeCursor(cursor);

  // Saves the first 5 characters of the version
  string_t *compareString = string_substring(http->version, 0, 4);
  // If the version does not start with "HTTP/" then exit
  if (string_equalsBuffer(compareString, "HTTP/") != 0) {
    log(LOG_ERROR, "Could not parse request:version");
    string_free(compareString);
    return false;
  }
  string_free(compareString);

  // Remove "HTTP/" from the version and saves only version index
  http_setVersion(http, string_substring(http->version, 5, string_getSize(http->version)));

  return true;
}

bool parseHeader(http_t *http, string_t *string) {
  char current = 0;
  string_cursor_t *cursor = string_createCursor(string);
  ssize_t offset = string_findNextChar(cursor, ':');

  if (offset == -1) {
    log(LOG_ERROR, "Could not find : in header");
    string_freeCursor(cursor);
    return false;
  }

  if (string_getCharAt(string, offset + 1) != ' ') {
    log(LOG_ERROR, "Expected space after :");
    string_freeCursor(cursor);
    return false;
  }

  string_t *key = string_create();
  string_t *value = string_create();
  size_t stringLength = string_getSize(string);
  key = string_substring(string, 0, offset);
  // + 2 to skip the space
  // - 1 because arrays are starting at index zero; string length = amount of chars in string
  value = string_substring(string, offset + 2, stringLength);
  log(LOG_DEBUG, "%s: %s", string_getBuffer(key), string_getBuffer(value));
  http_setHeader(http, key, value);

  string_freeCursor(cursor);
  return true;
}

void parseBody(http_t *http, string_t *string, size_t offset) {
  http->body = string_substring(string, offset, string_getSize(string));
}

enum httpMethod http_parseMethod(string_t *method) {
  if (string_equalsBuffer(method, "GET") == 1)
    return GET;
  if (string_equalsBuffer(method, "PUT") == 1)
    return PUT;
  if (string_equalsBuffer(method, "POST") == 1)
    return POST;
  if (string_equalsBuffer(method, "HEAD") == 1)
    return HEAD;
  if (string_equalsBuffer(method, "OPTIONS") == 1)
    return OPTIONS;

  log(LOG_ERROR, "Could not parse http method '%s'", string_getBuffer(method));
  return UNKNOWN;
}

void http_free(http_t *http) {
  if (http->requestTarget != 0)
    string_free(http->requestTarget);
  if (http->version != 0)
    string_free(http->version);
  if (http->responseCodeText != 0)
    string_free(http->responseCodeText);

  for (size_t i = 0; i < hash_table_getLength(http->headers); i++) {
    string_t *key = hash_table_getKeyByIndex(http->headers, i);
    string_t *value = hash_table_removeValue(http->headers, key);
    string_free(value);
  }
  hash_table_free(http->headers);
  free(http);
}
