#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../logging/logging.h"

#include "http.h"

// Forward declaration of functions (For internal use Only)
bool http_parseRequestLine(http_t *http, string_t *string);
bool http_parseHeader(http_t *http, string_t *string);
void http_parseBody(http_t *http, string_t *string, size_t offset);

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
  // Append body to result
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
  correctlyParsed = http_parseRequestLine(http, line);
  // Check if the parse was successfull
  if (correctlyParsed == false) {
    log(LOG_ERROR, "Could not parse request");
    return 0;
  }

  // Parse all headers (KEY: VALUES)
  while ((line = string_getNextLine(cursor)) != 0) {
    // if line is null break
    if (string_getSize(line) == 0) {
      string_free(line);
      break;
    }

    // Parse headers
    correctlyParsed = http_parseHeader(http, line);
    if (correctlyParsed == false) {
      log(LOG_ERROR, "Could not parse request");
      return 0;
    }
    string_free(line);
  }

  // Parse body if there is one
  http_parseBody(http, request, string_getOffset(cursor));
  string_freeCursor(cursor);


  // http://level1.level2.axgn.se/index/test.html?page=120&test=5

  // GET /index/test.html?page=120&test=5 HTTP/1.1
  // Host: level1.level2.axgn.se
  // Host: level1.level2.axgn.se:8080
  // Upgrade-Insecure-Requests: 1
  // Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
  // User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_14_6) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/12.1.2 Safari/605.1.15
  // Accept-Language: sv-se
  // Accept-Encoding: gzip, deflate
  // Connection: keep-alive

  // SET URL
  http->url = url_create();
  if (http->url == 0) {
    log(LOG_ERROR, "Could not inizialise https url struct");
    http_free(http);
    return 0;
  }

  url_setProtocol(http->url, string_fromCopy("http"));

  string_cursor_t *requestCursor = string_createCursor(http->requestTarget);
  ssize_t offset = string_findNextChar(requestCursor, '?');
  string_t *path = string_create();
  string_t *key = string_create();
  string_t *value = string_create();
  ssize_t oldOffset = 0;

  if (offset == -1) {
    path = string_substring(http->requestTarget, 0, string_getSize(http->requestTarget));
    url_setPath(http->url, path);
  } else {
    path = string_substring(http->requestTarget, 0, offset);
    url_setPath(http->url, path);
  }

  while (offset != -1) {
    oldOffset = offset + 1;
    offset = string_findNextChar(requestCursor, '=');
    if (offset == -1) { break; }
    key = string_substring(http->requestTarget, oldOffset, offset);

    oldOffset = offset + 1;
    offset = string_findNextChar(requestCursor, '&');
    if (offset == -1) {
      value = string_substring(http->requestTarget, oldOffset, string_getSize(http->requestTarget));
      url_setParameter(http->url, key, value);
      break;
    }
    value = string_substring(http->requestTarget, oldOffset, offset);

    url_setParameter(http->url, key, value);
  }

  string_t *keyHost = string_fromCopy("Host");
  string_t *host = hash_table_getValue(http->headers, keyHost);
  if (host == 0) {
    log(LOG_ERROR, "Can not parse. Host key was not set");
    http_free(http);
    return 0;
  }

  string_cursor_t *hostCursor = string_createCursor(host);
  ssize_t parameter = string_findNextChar(hostCursor, ':');
  string_t *domainName = string_create();

  if (parameter == -1) {
    url_setPort(http->url, 80);
    url_setDomainName(http->url, host);
  } else {
    // plus one to skip colon
    string_t *portString = string_substring(host, parameter + 1, string_getSize(host));
    int port = atoi(string_getBuffer(portString));
    if (port < 0 || port > 1<<16) {
      log(LOG_DEBUG, "The port in request was to large");
      return 0;
    }
    url_setPort(http->url, port);
    domainName = string_substring(host, 0, parameter);
    url_setDomainName(http->url, domainName);
  }

  // från key:HOST ska jag hämta hem SUBDOMAIN; DOMAIN; PORT
  //url_setDomain(url, );
  //url_setPort(url, );

  // ssize_t offset = string_findNextChar(requestCursor, ':');
  // if (offset == -1) {
  //    string_substring(http->port, 0, offset);
  // }

  return http;
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
    log(LOG_ERROR, "Could not parse request:method");
    return false;
  }

  // Convert from string to enum
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

bool http_parseHeader(http_t *http, string_t *string) {
  string_cursor_t *cursor = string_createCursor(string);
  ssize_t offset = string_findNextChar(cursor, ':');

  // Get the offset for the : if it exesists
  if (offset == -1) {
    log(LOG_ERROR, "Could not find : in header");
    string_freeCursor(cursor);
    return false;
  }

  // Making sure there is a space after :
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
  http_setHeader(http, key, value);

  string_freeCursor(cursor);
  return true;
}

void http_parseBody(http_t *http, string_t *string, size_t offset) {
  // After headers is added we put everything that is left in the request into the body variable
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

  while (hash_table_getLength(http->headers) > 0) {
    string_t *key = hash_table_getKeyByIndex(http->headers, 0);
    string_t *value = hash_table_removeValue(http->headers, key);
    string_free(value);
  }
  hash_table_free(http->headers);
  free(http);
}
