#include <stdlib.h>
#include <string.h>

#include "../compile-time-defines.h"
#include "../logging/logging.h"
#include "../resources/resources.h"

#include "www.h"

page_t *page_create() {
  page_t *page = malloc(sizeof(page_t));
  if (page == 0)
    return 0;

  memset(page, 0, sizeof(page_t));

  page->templates = hash_table_create();
  page_setTemplate(page, string_fromBuffer("version"), string_fromBuffer((const char *)WSIC_VERSION));

  return page;
}

page_t *page_create400(string_t *description) {
  page_t *page = page_create();
  page_setSource(page, string_fromBufferWithLength((const char *)RESOURCES_WWW_TEMPLATE_HTML, RESOURCES_WWW_TEMPLATE_HTML_LENGTH));

  page_setTemplate(page, string_fromBuffer("content"), string_fromBuffer((const char *)RESOURCES_WWW_400_HTML));
  page_setTemplate(page, string_fromBuffer("description"), description);

  page_resolveTemplates(page);

  return page;
}

page_t *page_create403() {
  page_t *page = page_create();
  page_setSource(page, string_fromBufferWithLength((const char *)RESOURCES_WWW_TEMPLATE_HTML, RESOURCES_WWW_TEMPLATE_HTML_LENGTH));

  page_setTemplate(page, string_fromBuffer("content"), string_fromBuffer((const char *)RESOURCES_WWW_403_HTML));

  page_resolveTemplates(page);

  return page;
}

page_t *page_create413() {
  page_t *page = page_create();
  page_setSource(page, string_fromBufferWithLength((const char *)RESOURCES_WWW_TEMPLATE_HTML, RESOURCES_WWW_TEMPLATE_HTML_LENGTH));

  page_setTemplate(page, string_fromBuffer("content"), string_fromBuffer((const char *)RESOURCES_WWW_413_HTML));

  page_resolveTemplates(page);

  return page;
}

page_t *page_create417() {
  page_t *page = page_create();
  page_setSource(page, string_fromBufferWithLength((const char *)RESOURCES_WWW_TEMPLATE_HTML, RESOURCES_WWW_TEMPLATE_HTML_LENGTH));

  page_setTemplate(page, string_fromBuffer("content"), string_fromBuffer((const char *)RESOURCES_WWW_417_HTML));

  page_resolveTemplates(page);

  return page;
}

page_t *page_create404(string_t *path) {
  page_t *page = page_create();
  page_setSource(page, string_fromBufferWithLength((const char *)RESOURCES_WWW_TEMPLATE_HTML, RESOURCES_WWW_TEMPLATE_HTML_LENGTH));

  page_setTemplate(page, string_fromBuffer("content"), string_fromBuffer((const char *)RESOURCES_WWW_404_HTML));
  page_setTemplate(page, string_fromBuffer("path"), path);

  page_resolveTemplates(page);

  return page;
}

page_t *page_create500(string_t *description) {
  page_t *page = page_create();
  page_setSource(page, string_fromBufferWithLength((const char *)RESOURCES_WWW_TEMPLATE_HTML, RESOURCES_WWW_TEMPLATE_HTML_LENGTH));

  page_setTemplate(page, string_fromBuffer("content"), string_fromBuffer((const char *)RESOURCES_WWW_500_HTML));
  page_setTemplate(page, string_fromBuffer("description"), description);

  page_resolveTemplates(page);

  return page;
}

page_t *page_create501() {
  page_t *page = page_create();
  page_setSource(page, string_fromBufferWithLength((const char *)RESOURCES_WWW_TEMPLATE_HTML, RESOURCES_WWW_TEMPLATE_HTML_LENGTH));

  page_setTemplate(page, string_fromBuffer("content"), string_fromBuffer((const char *)RESOURCES_WWW_501_HTML));

  page_resolveTemplates(page);

  return page;
}

void page_setSource(page_t *page, string_t *source) {
  if (page->source != 0)
    string_free(page->source);

  page->source = source;
}

string_t *page_getSource(page_t *page) {
  return page->source;
}

void page_setTemplate(page_t *page, string_t *key, string_t *value) {
  hash_table_setValue(page->templates, key, value);
}

ssize_t page_resolveTemplate(page_t *page, size_t offset) {
  string_t *templateKey = string_create();
  // Pre-allocate 100 bytes (determined to be enough for current use cases)
  string_setBufferSize(templateKey, 100);

  // Inclusive index range for the template key
  ssize_t templateStart = -1;
  ssize_t templateEnd = -1;
  char previous = 0;
  char current = 0;
  // Create a cursor to the current offset
  string_cursor_t *sourceCursor = string_createCursor(page->source);
  string_setOffset(sourceCursor, offset);
  while ((current = string_getNextChar(sourceCursor)) != 0) {
    if (previous == '{' && current == '{') {
      // -1 since the cursor has moved beyond the character
      // -2 since we want to have the first '{' as the template start
      templateStart = string_getOffset(sourceCursor) - 2;
    } else if (templateStart >= 0) {
      if (current >= 'a' && current <= 'z') {
        // Append allowed letters [a-z]
        string_appendChar(templateKey, current);
      } else if (current != '}') {
        // Reset the parsed template if an illegal character was found
        templateStart = -1;
        string_clear(templateKey);
      } else if (templateStart >= 0 && previous == '}' && current == '}') {
        // -1 since the cursor has moved beyond the character
        templateEnd = string_getOffset(sourceCursor) - 1;

        // Retrieve the value and length of the template (which may not exist)
        string_t *templateValue = hash_table_getValue(page->templates, templateKey);
        size_t templateValueLength = templateValue == 0 ? 0 : string_getSize(templateValue);

        string_t *oldSource = page->source;
        // Create a new string with a determined buffer size for some speedup
        page->source = string_create();
        string_setBufferSize(page->source, string_getSize(oldSource) - (templateEnd - templateStart) - 1 + templateValueLength);

        // Append the source before the template
        string_appendBufferWithLength(page->source, string_getBuffer(oldSource), templateStart);

        // Append the template value if it exists
        if (templateValue != 0 && templateValueLength != 0)
          string_appendBufferWithLength(page->source, string_getBuffer(templateValue), templateValueLength);
        else
          log(LOG_WARNING, "No value for template %s", string_getBuffer(templateKey));

        // Append the source after the template
        string_appendBufferWithLength(page->source, string_getBuffer(oldSource) + templateEnd + 1, string_getSize(oldSource) - templateEnd - 1);

        string_free(oldSource);
        string_freeCursor(sourceCursor);
        string_free(templateKey);
        return templateStart;
      }
    }

    previous = current;
  }

  string_free(templateKey);
  string_freeCursor(sourceCursor);
  return -1;
}

void page_resolveTemplates(page_t *page) {
  ssize_t offset = 0;
  do {
    offset = page_resolveTemplate(page, offset);
  } while (offset > -1);

  page_clearTemplates(page);
}

void page_clearTemplates(page_t *page) {
  if (page->templates == 0)
    return;

  while (hash_table_getLength(page->templates) > 0) {
    string_t *key = hash_table_getKeyByIndex(page->templates, 0);
    string_t *value = hash_table_removeValue(page->templates, key);
    string_free(value);
  }

  hash_table_free(page->templates);
  page->templates = 0;
}

void page_free(page_t *page) {
  if (page->source != 0)
    string_free(page->source);

  page_clearTemplates(page);

  free(page);
}
