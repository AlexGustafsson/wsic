#include <string.h>
#include <stdlib.h>

#include "../logging/logging.h"

#include "http.h"

http_t *createHttp() {
  http_t *http = malloc(sizeof(http_t));
  memset(http, 0, sizeof(http_t));

  return http;
}

void setHttpMethod(http_t *http, enum httpMethod method) {
  http->method = method;
}

void setHttpRequestTarget(http_t *http, const char *requestTarget) {
  if (http->requestTarget != 0)
    free(http->requestTarget);

  size_t length = strlen(requestTarget);
  http->requestTarget = malloc(length + 1);
  strlcpy(http->requestTarget, requestTarget, length + 1);
}

void setHttpVersion(http_t *http, const char *version) {
  if (http->version != 0)
    free(http->version);

  size_t length = strlen(version);
  http->version = malloc(length + 1);
  strlcpy(http->version, version, length + 1);
}

http_t *parseHttpRequest(const char *request) {
  http_t *http = createHttp();
  size_t j, x, y, z = 0;
  char buffer[1024] = {0};
  char method[1024] = {0};
  char requestTarget[1024] = {0};
  char version[1024] = {0};

  // Saves the first row of the header in buffer
  for (size_t i = 0; i < strlen(request) && buffer[i] != '\n' && i < 1024; i++)
    buffer[i] = request[i];

  // Filter out the http method
  for (j = 0; j < strlen(buffer) && buffer[j] != ' '; j++)
    method[x++] = buffer[j];
  setHttpMethod(http, parseHttpMethod(method));
  // Filter out the request target
  for (j += 1; j < strlen(buffer) && buffer[j] != ' '; j++)
    requestTarget[y++] = buffer[j];
  setHttpRequestTarget(http, requestTarget);
  // Filter out the http version
  for (j += 1; j < strlen(buffer) && buffer[j] != '\n'; j++)
    version[z++] = buffer[j];
  setHttpVersion(http, version);

  int offset = (j+1);
  char bufferKeys[1024] = {0};
  char bufferValues[1024] = {0};
  for (size_t i = 0; offset < strlen(request); i++) {
    for (size_t y = 0; y < 1024 && request[offset] != ':'; y++) {
      bufferKeys[y] = request[offset++];
    }
    offset += 2;
    for (size_t y = 0; y < 1024 && request[offset] != '\n'; y++) {
      bufferValues[y] = request[offset++];
    }
    offset += 1;
    addHeader(http, bufferKeys, bufferValues);
    for (int i = 0; i < 1024; i++) {
      bufferKeys[i] = 0;
      bufferValues[i] = 0;
    }
  }
  return http;
}

void addHeader(http_t *http, const char *key, const char *value) {
  if (http->headers == 0) {
    http->headerKeys = malloc(sizeof(char *));
    http->headerValues = malloc(sizeof(char *));
    http->headerKeys[0] = 0;
    http->headerValues[0] = 0;
  } else {
    char **reallocatedHeaderKeys = realloc(http->headerKeys, sizeof(char *) * (http->headers + 1));
    if (reallocatedHeaderKeys == 0) {
      log(LOG_ERROR, "Could not expand http keys array");
      return;
    }

    char **reallocatedHeaderValues = realloc(http->headerValues, sizeof(char *) * (http->headers + 1));
    if (reallocatedHeaderValues == 0) {
      log(LOG_ERROR, "Could not expand http value array");
      return;
    }

    http->headerKeys = reallocatedHeaderKeys;
    http->headerValues = reallocatedHeaderValues;
    http->headerKeys[http->headers] = 0;
    http->headerValues[http->headers] = 0;
  }

  size_t keyLength = strlen(key);
  http->headerKeys[http->headers] = malloc(sizeof(char) * (keyLength + 1));
  strlcpy(http->headerKeys[http->headers], key, keyLength + 1);


  size_t valueLength = strlen(value);
  http->headerValues[http->headers] = malloc(sizeof(char) * (valueLength + 1));
  strlcpy(http->headerValues[http->headers], value, valueLength + 1);

  http->headers++;
}

void freeHttp(http_t *http) {
  if (http->headerKeys != 0) {
    for (size_t i = 0; i < http->headers; i++) {
      if (http->headerKeys[i] != 0)
        free(http->headerKeys[i]);
    }
    free(http->headerKeys);
  }
  if (http->headerValues != 0) {
    for (size_t i = 0; i < http->headers; i++) {
      if (http->headerValues[i] != 0)
        free(http->headerValues[i]);
    }
    free(http->headerValues);
  }
  if (http->requestTarget != 0)
    free(http->requestTarget);
  if (http->version != 0)
    free(http->version);
  free(http);
}

enum httpMethod parseHttpMethod(char* method) {
  if (strcmp(method, "GET") == 0)
    return GET;
  if (strcmp(method, "PUT") == 0)
    return PUT;
  if (strcmp(method, "POST") == 0)
    return POST;
  if (strcmp(method, "HEAD") == 0)
    return HEAD;
  if (strcmp(method, "OPTIONS") == 0)
    return OPTIONS;

  log(LOG_ERROR, "Could not parse http method '%s'", method);
  return UNKNOWN;
}
