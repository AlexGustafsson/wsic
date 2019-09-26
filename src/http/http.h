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

http_t *http_parseRequest(string_t *request);
bool http_parseRequestLine(http_t *http, string_t *string);
bool http_parseHeader(http_t *http, string_t *string);
void http_parseBody(http_t *http, string_t *string, size_t offset);
bool http_parseRequestTarget(http_t *http, string_t *requestTarget);
bool http_parseHost(http_t *http);
bool http_parseUrl(http_t *http);
enum httpMethod http_parseMethod(string_t *method);

void http_setMethod(http_t *http, enum httpMethod method);
enum httpMethod http_getMethod(http_t *http);

void http_setVersion(http_t *http, string_t *version);
string_t *http_getVersion(http_t *http);

void http_setResponseCode(http_t *http, uint16_t code);
uint16_t http_getResponseCode(http_t *http);

void http_setHeader(http_t *http, string_t *key, string_t *value);
string_t *http_getHeader(http_t *http, string_t *key);

void http_setVersion(http_t *http, string_t *version);
string_t *http_getVersion(http_t *http);

void http_setBody(http_t *http, string_t *body);
string_t *http_getBody(http_t *http);

url_t *http_getUrl(http_t *http);

string_t *http_toResponseString(http_t *http);

void http_free(http_t *http);

#endif
