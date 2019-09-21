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

string_t *url_toString(url_t *url) {
  string_t *result = string_create();

  if (url->protocol != 0) {
    string_append(result, url->protocol);
    string_appendBuffer(result, "://");
  }

  if (url->subdomain != 0) {
    string_append(result, url->subdomain);
    string_appendChar(result, '.');
  }

  if (url->domain != 0)
    string_append(result, url->domain);

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

  if (url->fragment != 0) {
    string_appendChar(result, '#');
    string_append(result, url->fragment);
  }

  return result;
}

void url_setProtocol(url_t *url, string_t *protocol) {
  if (url->protocol != 0)
    string_free(url->protocol);

  url->protocol = protocol;
}

string_t *url_getProtocol(url_t *url) {
  return url->protocol;
}

void url_setSubdomain(url_t *url, string_t *subdomain) {
  if (url->subdomain != 0)
    string_free(url->subdomain);

  url->subdomain = subdomain;
}

void url_setDomain(url_t *url, string_t *domain) {
  if (url->domain != 0)
    string_free(url->domain);

  url->domain = domain;
}

string_t *url_getDomain(url_t *url) {
  return url->domain;
}

void url_setPort(url_t *url, uint16_t port) {
  url->port = port;
}

uint16_t url_getPort(url_t *url) {
  return url->port;
}

void url_setPath(url_t *url, string_t *path) {
  if (url->path != 0)
    string_free(url->path);

  url->path = path;
}

void url_setParameter(url_t *url, string_t *key, string_t *value) {
  hash_table_setValue(url->parameters, key, value);
}

string_t *url_getParameter(url_t *url, string_t *key) {
  return hash_table_getValue(url->parameters, key);
}

void url_setFragment(url_t *url, string_t *fragment) {
  if (url->fragment != 0)
    string_free(url->fragment);

  url->fragment = fragment;
}

string_t *url_getFragment(url_t *url) {
  return url->fragment;
}

void url_free(url_t *url) {
  if (url->protocol != 0)
    string_free(url->protocol);
  if (url->subdomain != 0)
    string_free(url->subdomain);
  if (url->domain != 0)
    string_free(url->domain);
  if (url->path != 0)
    string_free(url->path);
  while (hash_table_getLength(url->parameters) > 0) {
    string_t *key = hash_table_getKeyByIndex(url->parameters, 0);
    string_t *value = hash_table_removeValue(url->parameters, key);
    string_free(value);
  }
  hash_table_free(url->parameters);
  if (url->fragment != 0)
    string_free(url->fragment);
  free(url);
}
