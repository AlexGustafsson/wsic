#ifndef WWW_H
#define WWW_H

#include <stdio.h>
#include <stdbool.h>

#include "../datastructures/hash-table/hash-table.h"
#include "../string/string.h"

typedef struct {
  string_t *source;
  hash_table_t *templates;
} page_t;

page_t *page_create();

page_t *page_create400(const char *description);
page_t *page_create403();
page_t *page_create404(const char *path);
page_t *page_create500(const char *description);
page_t *page_create501();

void page_setSourceBuffer(page_t *page, const char *source);
void page_setSource(page_t *page, string_t *source);
void page_setTemplate(page_t *page, string_t *key, string_t *value);
void page_setTemplateBuffer(page_t *page, string_t *key, const char *value);

ssize_t page_resolveTemplate(page_t *page, size_t offset);
void page_resolveTemplates(page_t *page);

void page_free(page_t *page);

#endif
