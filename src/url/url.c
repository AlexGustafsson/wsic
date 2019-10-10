#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../logging/logging.h"

#include "url.h"

url_t *url_create() {
  url_t *url = malloc(sizeof(url_t));
  if (url == 0)
    return 0;

  memset(url, 0, sizeof(url_t));

  url->parameters = hash_table_create();

  return url;
}

url_t *url_copy(const url_t *url) {
  url_t *newUrl = url_create();
  if (newUrl == 0)
    return 0;

  if (url->protocol != 0)
    newUrl->protocol = string_copy(url->protocol);

  if (url->domainName != 0)
    newUrl->domainName = string_copy(url->domainName);

  newUrl->port = url->port;

  if (url->path != 0)
    newUrl->path = string_copy(url->path);

  size_t parameters = hash_table_getLength(url->parameters);
  for (size_t i = 0; i < parameters; i++) {
    string_t *key = hash_table_getKeyByIndex(url->parameters, i);
    string_t *value = hash_table_getValueByIndex(url->parameters, i);
    url_setParameter(newUrl, string_copy(key), string_copy(value));
  }

  return newUrl;
}

string_t *url_toString(const url_t *url) {
  string_t *result = string_create();

  if (url->protocol != 0) {
    string_append(result, url->protocol);
    string_appendBuffer(result, "://");
  }

  if (url->domainName != 0)
    string_append(result, url->domainName);

  if (url->port != 0 && url->port != 80) {
    string_t *port = string_fromInt(url->port);

    string_appendChar(result, ':');
    string_append(result, port);
    string_free(port);
  }

  if (url->path != 0)
    string_append(result, url->path);

  size_t parameters = hash_table_getLength(url->parameters);
  if (parameters > 0)
    string_appendChar(result, '?');

  for (size_t i = 0; i < parameters; i++) {
    string_t *key = (string_t *)hash_table_getKeyByIndex(url->parameters, i);
    string_t *value = (string_t *)hash_table_getValueByIndex(url->parameters, i);
    string_append(result, key);
    string_appendChar(result, '=');
    string_append(result, value);

    if (i + 1 < parameters)
      string_appendChar(result, '&');
  }

  return result;
}

string_t *url_toQueryString(const url_t *url) {
  if (url->parameters == 0) {
    return 0;
  }

  string_t *result = string_create();
  size_t parameters = hash_table_getLength(url->parameters);

  for (size_t i = 0; i < parameters; i++) {
    string_t *key = (string_t *)hash_table_getKeyByIndex(url->parameters, i);
    string_t *value = (string_t *)hash_table_getValueByIndex(url->parameters, i);

    string_append(result, key);
    string_appendChar(result, '=');
    string_append(result, value);

    if (i + 1 < parameters)
      string_appendChar(result, '&');
  }

  return result;
}

void url_setProtocol(url_t *url, string_t *protocol) {
  if (url->protocol != 0)
    string_free(url->protocol);

  url->protocol = protocol;
}

string_t *url_getProtocol(const url_t *url) {
  return url->protocol;
}

void url_setDomainName(url_t *url, string_t *domainName) {
  if (url->domainName != 0)
    string_free(url->domainName);

  url->domainName = domainName;
}

string_t *url_getDomainName(const url_t *url) {
  return url->domainName;
}

void url_setPort(url_t *url, uint16_t port) {
  url->port = port;
}

uint16_t url_getPort(const url_t *url) {
  return url->port;
}

void url_setPath(url_t *url, string_t *path) {
  if (url->path != 0)
    string_free(url->path);

  url->path = path;
}

string_t *url_getPath(const url_t *url) {
  return url->path;
}

void url_setParameter(url_t *url, string_t *key, string_t *value) {
  string_t *oldValue = hash_table_setValue(url->parameters, key, value);
  if (oldValue != 0)
    string_free(oldValue);
}

string_t *url_getParameter(const url_t *url, const string_t *key) {
  return hash_table_getValue(url->parameters, key);
}

void url_free(url_t *url) {
  if (url->protocol != 0)
    string_free(url->protocol);
  if (url->domainName != 0)
    string_free(url->domainName);
  if (url->path != 0)
    string_free(url->path);
  while (hash_table_getLength(url->parameters) > 0) {
    string_t *key = hash_table_getKeyByIndex(url->parameters, 0);
    string_t *value = hash_table_removeValue(url->parameters, key);
    string_free(value);
  }
  hash_table_free(url->parameters);
  free(url);
}
