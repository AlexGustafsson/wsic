#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../logging/logging.h"

#include "http.h"

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

enum httpMethod http_getMethod(http_t *http) {
  return http->method;
}

void http_setVersion(http_t *http, string_t *version) {
  if (http->version != 0)
    string_free(http->version);

  http->version = version;
}

string_t *http_getVersion(http_t *http) {
  return http->version;
}

void http_setResponseCode(http_t *http, uint16_t code) {
  http->responseCode = code;
  http->responseCodeText = http_codeToString(code);
}

uint16_t http_getResponseCode(http_t *http) {
  return http->responseCode;
}

void http_setHeader(http_t *http, string_t *key, string_t *value) {
  string_t *oldValue = hash_table_setValue(http->headers, key, value);
  if (oldValue != 0)
    string_free(oldValue);
}

string_t *http_getHeader(http_t *http, string_t *key) {
  return hash_table_getValue(http->headers, key);
}

void http_setBody(http_t *http, string_t *body) {
  if (http->body != 0)
    string_free(http->body);

  http->body = body;
  // Content-Length
  http_setHeader(http, string_fromBuffer("Content-Length"), string_fromInt(string_getSize(http->body)));
}

string_t *http_getBody(http_t *http) {
  return http->body;
}

url_t *http_getUrl(http_t *http) {
  return http->url;
}

string_t *http_toResponseString(http_t *http) {
  string_t *result = string_create();
  // First line is roughly 40 bytes, a header entry is roughly 40 bytes - allows for some speedup
  string_setBufferSize(result, 40 + hash_table_getLength(http->headers) * 40 + (http->body == 0 ? 0 : string_getSize(http->body)));

  if (http->version != 0) {
    string_appendBuffer(result, "HTTP/");
    string_append(result, http->version);
    string_appendChar(result, ' ');
  }

  string_t *responseCode = string_fromInt(http->responseCode);

  string_append(result, responseCode);
  string_free(responseCode);
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
  // Append body to result if it exists
  if (http->body != 0)
    string_append(result, http->body);

  return result;
}

bool http_parseRequestLine(http_t *http, string_t *string) {
  char current = 0;
  string_t *tempString = string_create();
  string_cursor_t *cursor = string_createCursor(string);

  // Parse method
  // Reads untill a space is found or null char at end on line
  while ((current = string_getNextChar(cursor)) != ' ' && current != 0)
    string_appendChar(tempString, current);

  if (current == 0) {
    log(LOG_ERROR, "Could not parse request. Missing path and/or version");
    string_free(tempString);
    string_freeCursor(cursor);
    return false;
  }

  // Convert from string to enum and sets method
  http->method = http_parseMethod(tempString);
  string_free(tempString);

  // Initialize https url struct
  if (http->url == 0)
    http->url = url_create();
  if (http->url == 0) {
    log(LOG_ERROR, "Could not initialize http's url struct");
    string_freeCursor(cursor);
    return false;
  }
  url_setProtocol(http->url, string_fromBuffer("http"));

  // Get the path and parameters
  // Reads untill a space is found or null char at end on line
  string_t *requestTarget = string_create();
  while ((current = string_getNextChar(cursor)) != ' ' && current != 0)
    string_appendChar(requestTarget, current);

  if (current == 0) {
    log(LOG_ERROR, "Could not parse request. Missing version");
    string_freeCursor(cursor);
    string_free(requestTarget);
    return false;
  }

  // Parse path and parameters (request target)
  bool correctlyParsed = http_parseRequestTarget(http, requestTarget);
  if (correctlyParsed == false) {
    log(LOG_ERROR, "Could not parse path and optional parameters");
    string_freeCursor(cursor);
    string_free(requestTarget);
    return false;
  }
  string_free(requestTarget);

  // Parse version
  // Reads untill null char at end on line
  string_t *versionString = string_create();
  http_setVersion(http, string_create());
  while ((current = string_getNextChar(cursor)) != 0)
    string_appendChar(versionString, current);
  // Free cursor, not in use anny more
  string_freeCursor(cursor);

  // Check to see if the version starts with HTTP/
  string_t *compareString = string_substring(versionString, 0, 5);
  if (compareString == 0) {
    log(LOG_ERROR, "Could not parse HTTP version. Unknown value was passed");
    if (compareString != 0)
      string_free(compareString);
    if (versionString != 0)
      string_free(versionString);
    return false;
  }
  // If the version does not start with "HTTP/" then exit
  if (string_equalsBuffer(compareString, "HTTP/") == false) {
    log(LOG_ERROR, "Could not parse request:version. Invalid input missing 'HTTP/''");
    if (compareString != 0)
      string_free(compareString);
    if (versionString != 0)
      string_free(versionString);
    return false;
  }
  string_free(compareString);

  string_t *version = string_substring(versionString, 5, 8);
  if (version == 0) {
    log(LOG_ERROR, "Could not parse request:version. Invalid input missing 'chars after HTTP/'");
    string_free(versionString);
    return false;
  }

  // The three chars after HTTP/ must be in the order: '[0-9]\.[0-9]'
  bool firstCharIsNotInt = string_getCharAt(version, 0) < '0' || string_getCharAt(version, 0) > '9';
  bool secondCharIsNotDot = string_getCharAt(version, 1) != '.';
  bool thirdCharIsNotInt = string_getCharAt(version, 2) < '0' || string_getCharAt(version, 0) > '9';

  if (firstCharIsNotInt || secondCharIsNotDot || thirdCharIsNotInt) {
    log(LOG_ERROR, "Could not parse request:version. Invalid input version nuber vas incorect");
    string_free(version);
    string_free(versionString);
    return false;
  }

  // Set the version
  http_setVersion(http, version);

  string_free(versionString);
  return true;
}

bool http_parseHeader(http_t *http, string_t *string) {
  string_cursor_t *cursor = string_createCursor(string);
  ssize_t offset = string_findNextChar(cursor, ':');
  string_freeCursor(cursor);

  // Get the offset for the : if it exesists
  if (offset == -1) {
    log(LOG_ERROR, "Could not find : in header");
    return false;
  }

  // Making sure there is a space after :
  if (string_getCharAt(string, offset + 1) != ' ') {
    log(LOG_ERROR, "Expected space after :");
    return false;
  }

  size_t stringLength = string_getSize(string);
  string_t *key = string_substring(string, 0, offset);
  // + 2 to skip the space
  string_t *value = string_substring(string, offset + 2, stringLength);
  if (value == 0) {
    log(LOG_ERROR, "The header's value could not be parsed");
    return false;
  }

  http_setHeader(http, key, value);
  return true;
}

void http_parseBody(http_t *http, string_t *string, size_t offset) {
  if (http->body != 0)
    string_free(http->body);
  // After headers is added we put everything that is left in the request into the body variable
  http->body = string_substring(string, offset, string_getSize(string));
}

bool http_parseRequestTarget(http_t *http, string_t *requestTarget) {
  string_cursor_t *requestCursor = string_createCursor(requestTarget);
  ssize_t firstParameter = string_findNextChar(requestCursor, '?');

  string_t *path = 0;
  if (firstParameter == -1) {
    path = string_substring(requestTarget, 0, string_getSize(requestTarget));
    url_setPath(http->url, path);
  } else {
    path = string_substring(requestTarget, 0, firstParameter);
    url_setPath(http->url, path);

    // If there is nothing more that a ? at the end, we set the path and return as there is no url parameters
    char current = 0;
    if ((current = string_getNextChar(requestCursor)) == 0) {
      string_freeCursor(requestCursor);
      return true;
    }
  }

  ssize_t secondParameter = 0;
  string_t *value = 0;
  string_t *key = 0;
  while (firstParameter != -1) {
    secondParameter = firstParameter + 1;
    firstParameter = string_findNextChar(requestCursor, '=');
    if (firstParameter == -1) {
      string_freeCursor(requestCursor);
      return false;
    }
    key = string_substring(requestTarget, secondParameter, firstParameter);

    secondParameter = firstParameter + 1;
    firstParameter = string_findNextChar(requestCursor, '&');
    if (firstParameter == -1) {
      value = string_substring(requestTarget, secondParameter, string_getSize(requestTarget));
      url_setParameter(http->url, key, value);
      break;
    }

    value = string_substring(requestTarget, secondParameter, firstParameter);
    url_setParameter(http->url, key, value);
  }

  string_freeCursor(requestCursor);
  return true;
}

bool http_parseHost(http_t *http) {
  string_t *keyHost = string_fromBuffer("Host");
  string_t *host = hash_table_getValue(http->headers, keyHost);
  string_free(keyHost);
  if (host == 0) {
    log(LOG_ERROR, "Can not parse. Host key was not set");
    return false;
  }

  string_cursor_t *hostCursor = string_createCursor(host);
  ssize_t parameter = string_findNextChar(hostCursor, ':');
  string_freeCursor(hostCursor);
  if (http->url == 0)
    http->url = url_create();
  if (parameter == -1) {
    url_setPort(http->url, 80);
    url_setDomainName(http->url, string_fromBuffer(string_getBuffer(host)));
  } else {
    // parameter + 1 to skip colon
    string_t *portString = string_substring(host, parameter + 1, string_getSize(host));
    int port = atoi(string_getBuffer(portString));
    string_free(portString);
    if (port < 0 || port > 1 << 16) {
      log(LOG_DEBUG, "The port in the request was to large");
      return false;
    }
    url_setPort(http->url, port);
    string_t *domainName = string_substring(host, 0, parameter);
    url_setDomainName(http->url, domainName);
  }
  return true;
}

enum httpMethod http_parseMethod(string_t *method) {
  if (string_equalsBuffer(method, "GET") == 1)
    return HTTP_METHOD_GET;
  if (string_equalsBuffer(method, "PUT") == 1)
    return HTTP_METHOD_PUT;
  if (string_equalsBuffer(method, "POST") == 1)
    return HTTP_METHOD_POST;
  if (string_equalsBuffer(method, "HEAD") == 1)
    return HTTP_METHOD_HEAD;
  if (string_equalsBuffer(method, "OPTIONS") == 1)
    return HTTP_METHOD_OPTIONS;

  log(LOG_ERROR, "Could not parse http method '%s'", string_getBuffer(method));
  return HTTP_METHOD_UNKNOWN;
}

string_t *http_methodToString(enum httpMethod method) {
  if (method == 1)
    return string_fromBuffer("GET");
  if (method == 2)
    return string_fromBuffer("PUT");
  if (method == 3)
    return string_fromBuffer("POST");
  if (method == 4)
    return string_fromBuffer("HEAD");
  if (method == 5)
    return string_fromBuffer("OPTIONS");

  log(LOG_ERROR, "Cound not go from method to string '%d'", method);
  return 0;
}

void http_free(http_t *http) {
  if (http->version != 0)
    string_free(http->version);
  if (http->responseCodeText != 0)
    string_free(http->responseCodeText);
  if (http->body != 0)
    string_free(http->body);

  while (hash_table_getLength(http->headers) > 0) {
    string_t *key = hash_table_getKeyByIndex(http->headers, 0);
    string_t *value = hash_table_removeValue(http->headers, key);
    string_free(value);
  }
  hash_table_free(http->headers);

  if (http->url != 0)
    url_free(http->url);

  free(http);
}
