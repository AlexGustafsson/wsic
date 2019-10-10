#ifndef HTTP_H
#define HTTP_H

#include <stdio.h>
#include <stdint.h>

#include "../string/string.h"
#include "../url/url.h"
#include "../datastructures/hash-table/hash-table.h"

#include "response-codes.h"

enum httpMethod { HTTP_METHOD_UNKNOWN,
                  HTTP_METHOD_GET,
                  HTTP_METHOD_PUT,
                  HTTP_METHOD_POST,
                  HTTP_METHOD_HEAD,
                  HTTP_METHOD_OPTIONS };

typedef struct {
  enum httpMethod method;
  string_t *version;
  uint16_t responseCode;
  // Only for internal use
  string_t *responseCodeText;
  hash_table_t *headers;
  string_t *body;
  url_t *url;
} http_t;

http_t *http_create();

http_t *http_parseRequest(const string_t *request);
bool http_parseRequestLine(http_t *http, const string_t *string);
bool http_parseHeader(http_t *http, const string_t *string);
void http_parseBody(http_t *http, string_t *string, size_t offset);
bool http_parseRequestTarget(http_t *http, const string_t *requestTarget);
bool http_parseHost(http_t *http);
bool http_parseUrl(http_t *http);
enum httpMethod http_parseMethod(const string_t *method);
string_t *http_methodToString(enum httpMethod method);

void http_setMethod(http_t *http, enum httpMethod method);
enum httpMethod http_getMethod(const http_t *http);

void http_setVersion(http_t *http, string_t *version);
string_t *http_getVersion(const http_t *http);

void http_setResponseCode(http_t *http, uint16_t code);
uint16_t http_getResponseCode(const http_t *http);

void http_setHeader(http_t *http, string_t *key, string_t *value);
string_t *http_getHeader(const http_t *http, const string_t *key);

void http_setVersion(http_t *http, string_t *version);
string_t *http_getVersion(const http_t *http);

void http_setBody(http_t *http, string_t *body);
string_t *http_getBody(const http_t *http);

url_t *http_getUrl(const http_t *http);

string_t *http_toResponseString(const http_t *http);

void http_free(http_t *http);

#endif
