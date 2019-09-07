#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../logging/logging.h"

#include "url.h"

url_t *createUrl() {
  url_t *url = malloc(sizeof(url_t));
  memset(url, 0, sizeof(url_t));

  return url;
}

size_t urlToString(url_t *url, char *buffer, size_t bufferSize) {
  memset(buffer, 0, bufferSize);
  size_t offset = 0;

  if (url->protocol != 0) {
    size_t protocolLength = url->protocol == 0 ? 0 : strlen(url->protocol);
    log(LOG_DEBUG, "protocol: %s, length %zu", url->protocol, protocolLength);
    for (size_t i = 0; i < protocolLength && offset < bufferSize; i++)
      buffer[offset++] = url->protocol[i];
    for (size_t i = 0; i < 3 && offset < bufferSize; i++)
      buffer[offset++] = "://"[i];
  }

  if (url->subdomain != 0) {
    size_t subdomainLength = url->subdomain == 0 ? 0 : strlen(url->subdomain);

    for (size_t i = 0; i < subdomainLength && offset < bufferSize; i++)
      buffer[offset++] = url->subdomain[i];
    for (size_t i = 0; i < 1 && offset < bufferSize; i++)
      buffer[offset++] = "."[i];
  }

  if (url->domain != 0) {
    size_t domainLength = url->domain == 0 ? 0 : strlen(url->domain);

    for (size_t i = 0; i < domainLength && offset < bufferSize; i++)
      buffer[offset++] = url->domain[i];
  }

  if (url->port != 0 && url->port != 80) {
    // The max number of a 16 bit unsigned integer is 65356 which has 5
    // characters (one extra for null)
    char portBuffer[6] = {0};
    sprintf(portBuffer, "%d", url->port);
    size_t portLength = strlen(portBuffer);

    for (size_t i = 0; i < portLength && offset < bufferSize; i++)
      buffer[offset++] = portBuffer[i];
  }

  if (url->parameters > 0) {
    if (offset < bufferSize)
      buffer[offset++] = '?';

    for (size_t i = 0; i < url->parameters; i++) {
      size_t keyLength = url->queryKeys[i] == 0 ? 0 : strlen(url->queryKeys[i]);
      size_t valueLength =
          url->queryValues[i] == 0 ? 0 : strlen(url->queryValues[i]);

      for (size_t j = 0; j < keyLength && offset < bufferSize; j++)
        buffer[offset++] = url->queryKeys[i][j];
      if (offset < bufferSize)
        buffer[offset++] = '=';
      for (size_t j = 0; j < valueLength && offset < bufferSize; j++)
        buffer[offset++] = url->queryValues[i][j];

      if (i + 1 < url->parameters && offset < bufferSize)
        buffer[offset++] = '&';
    }
  }

  if (url->fragment != 0) {
    size_t fragmentLength = url->fragment == 0 ? 0 : strlen(url->fragment);

    if (offset < bufferSize)
      buffer[offset++] = '#';
    for (size_t i = 0; i < fragmentLength && offset < bufferSize; i++)
      buffer[offset++] = url->fragment[i];
  }

  buffer[offset] = 0;

  return offset;
}

void setUrlProtocol(url_t *url, const char *protocol) {
  if (url->protocol != 0)
    free(url->protocol);

  size_t length = strlen(protocol);
  url->protocol = malloc(length + 1);
  strlcpy(url->protocol, protocol, length + 1);
}

void setUrlSubdomain(url_t *url, const char *subdomain) {
  if (url->subdomain != 0)
    free(url->subdomain);

  size_t length = strlen(subdomain);
  url->subdomain = malloc(length + 1);
  strlcpy(url->subdomain, subdomain, length + 1);
}

void setUrlDomain(url_t *url, const char *domain) {
  if (url->domain != 0)
    free(url->domain);

  size_t length = strlen(domain);
  url->domain = malloc(length + 1);
  strlcpy(url->domain, domain, length + 1);
}

void setUrlPort(url_t *url, uint16_t port) {
  url->port = port;
}

void setUrlPath(url_t *url, const char *path) {
  if (url->path != 0)
    free(url->path);

  size_t length = strlen(path);
  url->path = malloc(length + 1);
  strlcpy(url->path, path, length + 1);
}

void addUrlParameter(url_t *url, const char *key, const char *value) {
  if (url->parameters == 0) {
    url->queryKeys = malloc(sizeof(char *));
    url->queryValues = malloc(sizeof(char *));
    url->queryKeys[0] = 0;
    url->queryValues[0] = 0;
  } else {
    char **reallocatedQueryKeys = realloc(url->queryKeys, sizeof(char *) * (url->parameters + 1));
    if (reallocatedQueryKeys == 0) {
      log(LOG_ERROR, "Could not expand url parameter array");
      return;
    }

    char **reallocatedQueryValues = realloc(url->queryValues, sizeof(char *) * (url->parameters + 1));
    if (reallocatedQueryValues == 0) {
      log(LOG_ERROR, "Could not expand url value array");
      return;
    }

    url->queryKeys = reallocatedQueryKeys;
    url->queryValues = reallocatedQueryValues;
    url->queryKeys[url->parameters] = 0;
    url->queryValues[url->parameters] = 0;
  }

  size_t keyLength = strlen(key);
  url->queryKeys[url->parameters] = malloc(sizeof(char) * (keyLength + 1));
  strlcpy(url->queryKeys[url->parameters], key, keyLength + 1);
  url->queryKeys[url->parameters][keyLength] = 0;

  size_t valueLength = strlen(value);
  url->queryValues[url->parameters] = malloc(sizeof(char) * (valueLength + 1));
  strlcpy(url->queryValues[url->parameters], value, valueLength + 1);

  url->parameters++;
}

void setUrlFragment(url_t *url, const char *fragment) {
  if (url->fragment != 0)
    free(url->fragment);

  size_t length = strlen(fragment);
  url->fragment = malloc(length + 1);
  strlcpy(url->fragment, fragment, length + 1);
}

void freeUrl(url_t *url) {
  if (url->protocol != 0)
    free(url->protocol);
  if (url->subdomain != 0)
    free(url->subdomain);
  if (url->domain != 0)
    free(url->domain);
  if (url->path != 0)
    free(url->path);
  if (url->queryKeys != 0) {
    for (size_t i = 0; i < url->parameters; i++) {
      if (url->queryKeys[i] != 0)
        free(url->queryKeys[i]);
    }
    free(url->queryKeys);
  }
  if (url->queryValues != 0) {
    for (size_t i = 0; i < url->parameters; i++) {
      if (url->queryValues[i] != 0)
        free(url->queryValues[i]);
    }
    free(url->queryValues);
  }
  if (url->fragment != 0)
    free(url->fragment);
  free(url);
}
