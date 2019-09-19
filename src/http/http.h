#ifndef HTTP_H
#define HTTP_H

#include <stdio.h>

enum httpMethod { GET,
                  PUT,
                  POST,
                  HEAD,
                  OPTIONS,
                  UNKNOWN };

typedef struct {
  enum httpMethod method;
  char *requestTarget;
  char *version;
  size_t headers;
  char **headerKeys;
  char **headerValues;
} http_t;

http_t *createHttp();

void setHttpMethod(http_t *http, enum httpMethod method);
void setHttpRequestTarget(http_t *http, const char *requestTarget);
void setHttpVersion(http_t *http, const char *version);

http_t *parseHttpRequest(const char *request);
void addHeader(http_t *http, const char *key, const char *value);

void freeHttp(http_t *http);

enum httpMethod parseHttpMethod(char *method);

#endif
