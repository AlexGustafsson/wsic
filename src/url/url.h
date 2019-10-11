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

url_t *url_copy(const url_t *url) __attribute__((nonnull(1)));

string_t *url_toString(const url_t *url) __attribute__((nonnull(1)));
string_t *url_toQueryString(const url_t *url) __attribute__((nonnull(1)));

// The protocol is owned
void url_setProtocol(url_t *url, string_t *protocol) __attribute__((nonnull(1)));
string_t *url_getProtocol(const url_t *url) __attribute__((nonnull(1)));

// The domain name is owned
void url_setDomainName(url_t *url, string_t *domainName) __attribute__((nonnull(1)));
string_t *url_getDomainName(const url_t *url) __attribute__((nonnull(1)));

void url_setPort(url_t *url, uint16_t port) __attribute__((nonnull(1)));
uint16_t url_getPort(const url_t *url) __attribute__((nonnull(1)));

// The path is owned
void url_setPath(url_t *url, string_t *path) __attribute__((nonnull(1)));
string_t *url_getPath(const url_t *url) __attribute__((nonnull(1)));

// The key and value is owned
void url_setParameter(url_t *url, string_t *key, string_t *value) __attribute__((nonnull(1, 2)));
// The returned value is owned
string_t *url_getParameter(const url_t *url, const string_t *key) __attribute__((nonnull(1, 2)));

void url_free(url_t *url) __attribute__((nonnull(1)));

#endif
