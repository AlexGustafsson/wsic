#ifndef WWW_H
#define WWW_H

#include <stdio.h>
#include <stdbool.h>

typedef struct {
  char *source;
  size_t sourceLength;
  size_t templates;
  char **templateKeys;
  char **templateValues;
} page_t;

page_t *createPage();

page_t *createPage404(const char *path);
void setPageSource(page_t *page, const char *source);
void addPageTemplate(page_t *page, const char *key, const char *value);

ssize_t resolveTemplateString(page_t *page, size_t offset);
void resolveTemplateStrings(page_t *page);

void freePage(page_t *page);

#endif
