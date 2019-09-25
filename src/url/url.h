#ifndef URL_H
#define URL_H

#include <stdint.h>
#include <stdio.h>

#include "../datastructures/hash-table/hash-table.h"
#include "../string/string.h"

typedef struct {
  string_t *protocol;
  string_t *domainName;
  uint16_t port;
  string_t *path;
  hash_table_t *parameters;
} url_t;

url_t *url_create();

string_t *url_toString(url_t *url);

void url_setProtocol(url_t *url, string_t *protocol);
string_t *url_getProtocol(url_t *url);

void url_setDomainName(url_t *url, string_t *domainName);
string_t *url_getDomainName(url_t *url);

void url_setPort(url_t *url, uint16_t port);
uint16_t url_getPort(url_t *url);

void url_setPath(url_t *url, string_t *path);
string_t *url_getPath(url_t *url);

void url_setParameter(url_t *url, string_t *key, string_t *value);
string_t *url_getParameter(url_t *url, string_t *key);

void url_free(url_t *url);

#endif
