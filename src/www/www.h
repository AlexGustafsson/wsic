#ifndef WWW_H
#define WWW_H

#include <stdbool.h>
#include <stdio.h>

#include "../datastructures/hash-table/hash-table.h"
#include "../string/string.h"

typedef struct {
  string_t *source;
  hash_table_t *templates;
} page_t;

page_t *page_create();

page_t *page_create400(string_t *description) __attribute__((nonnull(1)));
page_t *page_create403();
page_t *page_create413();
page_t *page_create417();
page_t *page_create404(string_t *path) __attribute__((nonnull(1)));
page_t *page_create500(string_t *description) __attribute__((nonnull(1)));
page_t *page_create501();

void page_setSource(page_t *page, string_t *source) __attribute__((nonnull(1, 2)));
string_t *page_getSource(page_t *page) __attribute__((nonnull(1)));

void page_setTemplate(page_t *page, string_t *key, string_t *value) __attribute__((nonnull(1, 2)));

ssize_t page_resolveTemplate(page_t *page, size_t offset) __attribute__((nonnull(1)));
void page_resolveTemplates(page_t *page) __attribute__((nonnull(1)));

void page_clearTemplates(page_t *page) __attribute__((nonnull(1)));

void page_free(page_t *page) __attribute__((nonnull(1)));

#endif
