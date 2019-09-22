#ifndef HTTP_H
#define HTTP_H

#include <stdio.h>
#include <stdint.h>

#include "../string/string.h"
#include "../url/url.h"
#include "../datastructures/hash-table/hash-table.h"

#include "response-codes.h"

enum httpMethod { GET,
                  PUT,
                  POST,
                  HEAD,
                  OPTIONS,
                  UNKNOWN };

typedef struct {
  enum httpMethod method;
  string_t *requestTarget;
  string_t *version;
  uint16_t responseCode;
  // Only for internal use
  string_t *responseCodeText;
  hash_table_t *headers;
  string_t *body;
  url_t *url;
} http_t;

http_t *http_create();

void http_setMethod(http_t *http, enum httpMethod method);
void http_setRequestTarget(http_t *http, string_t *requestTarget);
void http_setVersion(http_t *http, string_t *version);
void http_setResponsCode(http_t *http, uint16_t code);
void http_setHeader(http_t *http, string_t *key, string_t *value);

string_t *http_toResponseString(http_t *http);
http_t *http_parseRequest(string_t *request);

enum httpMethod http_parseMethod(string_t *method);

void http_free(http_t *http);

#endif
