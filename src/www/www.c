#include <stdlib.h>
#include <string.h>

#include "../logging/logging.h"
#include "../resources/resources.h"
#include "../compile-time-defines.h"

#include "www.h"

page_t *createPage() {
  page_t *page = malloc(sizeof(page_t));
  memset(page, 0, sizeof(page_t));

  return page;
}

page_t *createPage400(const char *description) {
  page_t *page = createPage();
  setPageSource(page, RESOURCES_WWW_TEMPLATE_HTML);

  addPageTemplate(page, "content", RESOURCES_WWW_400_HTML);
  addPageTemplate(page, "description", description);
  addPageTemplate(page, "version", WSIC_VERSION);

  resolveTemplateStrings(page);

  return page;
}

page_t *createPage403() {
  page_t *page = createPage();
  setPageSource(page, RESOURCES_WWW_TEMPLATE_HTML);

  addPageTemplate(page, "content", RESOURCES_WWW_403_HTML);
  addPageTemplate(page, "version", WSIC_VERSION);

  resolveTemplateStrings(page);

  return page;
}

page_t *createPage404(const char *path) {
  page_t *page = createPage();
  setPageSource(page, RESOURCES_WWW_TEMPLATE_HTML);

  addPageTemplate(page, "content", RESOURCES_WWW_404_HTML);
  addPageTemplate(page, "path", path);
  addPageTemplate(page, "version", WSIC_VERSION);

  resolveTemplateStrings(page);

  return page;
}

page_t *createPage500(const char *description) {
  page_t *page = createPage();
  setPageSource(page, RESOURCES_WWW_TEMPLATE_HTML);

  addPageTemplate(page, "content", RESOURCES_WWW_500_HTML);
  addPageTemplate(page, "description", description);
  addPageTemplate(page, "version", WSIC_VERSION);

  resolveTemplateStrings(page);

  return page;
}

page_t *createPage501() {
  page_t *page = createPage();
  setPageSource(page, RESOURCES_WWW_TEMPLATE_HTML);

  addPageTemplate(page, "content", RESOURCES_WWW_501_HTML);
  addPageTemplate(page, "version", WSIC_VERSION);

  resolveTemplateStrings(page);

  return page;
}

void setPageSource(page_t *page, const char *source) {
  if (page->source != 0)
    free(page->source);

  size_t pageLength = strlen(source);

  page->sourceLength = pageLength;
  page->source = malloc(sizeof(char) * (pageLength + 1));
  strlcpy(page->source, source, pageLength + 1);
}

void addPageTemplate(page_t *page, const char *key, const char *value) {
  if (page->templates == 0) {
    page->templateKeys = malloc(sizeof(char *));
    page->templateValues = malloc(sizeof(char *));
    page->templateKeys[0] = 0;
    page->templateValues[0] = 0;
  } else {
    char **reallocatedTemplateKeys = realloc(page->templateKeys, sizeof(char *) * (page->templates + 1));
    if (reallocatedTemplateKeys == 0) {
      log(LOG_ERROR, "Could not expand page template key array");
      return;
    }

    char **reallocatedTemplateValues = realloc(page->templateValues, sizeof(char *) * (page->templates + 1));
    if (reallocatedTemplateValues == 0) {
      log(LOG_ERROR, "Could not expand page template value array");
      return;
    }

    page->templateKeys = reallocatedTemplateKeys;
    page->templateValues = reallocatedTemplateValues;
    page->templateKeys[page->templates] = 0;
    page->templateValues[page->templates] = 0;
  }

  size_t keyLength = strlen(key);
  page->templateKeys[page->templates] = malloc(sizeof(char) * (keyLength + 1));
  strlcpy(page->templateKeys[page->templates], key, keyLength + 1);

  size_t valueLength = strlen(value);
  page->templateValues[page->templates] = malloc(sizeof(char) * (valueLength + 1));
  strlcpy(page->templateValues[page->templates], value, valueLength + 1);

  page->templates++;
}

ssize_t resolveTemplateString(page_t *page, size_t offset) {
  char templateBuffer[100] = {0};
  size_t templateLength = 0;
  char previous = 0;

  ssize_t templateStart = -1;
  ssize_t templateEnd = -1;
  for (size_t i = offset; i < page->sourceLength; i++) {
    char current = page->source[i];
    if (previous == '{' && current == '{') {
      templateStart = i - 1;
    } else if (templateStart >= 0) {
      if (current >= 'a' && current <= 'z' && templateLength < 100) {
        templateBuffer[templateLength++] = current;
      } else if (current != '}') {
        templateStart = -1;
        memset(templateBuffer, 0, templateLength);
        templateLength = 0;
      } else if (templateStart >= 0 && previous == '}' && current == '}') {
        templateEnd = i;

        char *templateValue = 0;
        for (size_t j = 0; j < page->templates; j++) {
          if (strcmp(templateBuffer, page->templateKeys[j]) == 0) {
            templateValue = page->templateValues[j];
            break;
          }
        }

        size_t templateValueLength = templateValue == 0 ? 0 : strlen(templateValue);
        char *oldSource = page->source;
        page->source = malloc(page->sourceLength - (templateEnd - templateStart) - 1 + templateValueLength);
        // NOTE: Temporary memset. Please remove
        memset(page->source, '-', page->sourceLength - (templateEnd - templateStart) - 1 + templateValueLength);
        memcpy(page->source, oldSource, templateStart);
        if (templateValue != 0)
          memcpy(page->source + templateStart, templateValue, templateValueLength + 1);
        else
          log(LOG_ERROR, "No value for template %s", templateBuffer);
        memcpy(page->source + templateStart + templateValueLength, oldSource + templateEnd + 1, page->sourceLength - templateEnd - 1);
        page->sourceLength = page->sourceLength - (templateEnd - templateStart) - 1 + templateValueLength;
        free(oldSource);

        return templateStart;
      }
    }

    previous = current;
  }

  return -1;
}

void resolveTemplateStrings(page_t *page) {
  ssize_t offset = 0;
  do {
    offset = resolveTemplateString(page, offset);
  } while (offset > -1);
}

void freePage(page_t *page) {
  if (page->source != 0)
    free(page->source);
  if (page->templateKeys != 0) {
    for (size_t i = 0; i < page->templates; i++) {
      if (page->templateKeys[i] != 0)
        free(page->templateKeys[i]);
    }
    free(page->templateKeys);
  }
  if (page->templateValues != 0) {
    for (size_t i = 0; i < page->templates; i++) {
      if (page->templateValues[i] != 0)
        free(page->templateValues[i]);
    }
    free(page->templateValues);
  }
  free(page);
}
