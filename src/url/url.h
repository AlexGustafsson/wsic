#ifndef URL_H
#define URL_H

#include <stdint.h>
#include <stdio.h>

#define URL_VALIDATED 1
#define URL_OUT_OF_ROOT 2

typedef struct {
  char *protocol;
  char *subdomain;
  char *domain;
  uint16_t port;
  char *path;
  size_t parameters;
  char **queryKeys;
  char **queryValues;
  char *fragment;
} url_t;

url_t *createUrl();

size_t urlToString(url_t *url, char *buffer, size_t bufferSize);

void setUrlProtocol(url_t *url, const char *protocol);
void setUrlSubdomain(url_t *url, const char *subdomain);
void setUrlDomain(url_t *url, const char *domain);
void setUrlPort(url_t *url, uint16_t port);
void setUrlPath(url_t *url, const char *path);
void addUrlParameter(url_t *url, const char *key, const char *value);
void setUrlFragment(url_t *url, const char *fragment);

void freeUrl(url_t *url);

#endif
