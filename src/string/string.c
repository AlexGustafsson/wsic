#include <stdlib.h>
#include <string.h>

#include "string.h"

string_t *string_create() {
  string_t *string = malloc(sizeof(string_t));
  if (string == 0)
    return 0;

  string->buffer = 0;
  string->bufferSize = 0;
  string->size = 0;

  return string;
}

string_t *string_fromCopy(const char *buffer) {
  string_t *string = string_create();
  if (string == 0)
    return 0;

  string->size = strlen(buffer);
  string->bufferSize = string->size + 1;
  string->buffer = malloc(sizeof(char) * (string->bufferSize));
  if (string->buffer == 0) {
    free(string);
    return 0;
  }

  memcpy(string->buffer, buffer, string->size);
  string->buffer[string->bufferSize - 1] = 0;

  return string;
}

void string_append(string_t *string, const char *buffer) {
  size_t bufferSize = strlen(buffer);
  size_t newSize = string->size + bufferSize;

  // Expand if necessary
  if (newSize >= string->bufferSize) {
    char *expandedBuffer = realloc(string->buffer, sizeof(char) * (newSize + 1));
    if (expandedBuffer == 0)
      return;
    string->buffer = expandedBuffer;
    string->bufferSize = newSize + 1;
  }

  memcpy(string->buffer + string->size, buffer, bufferSize);
  string->buffer[string->bufferSize - 1] = 0;
  string->size = newSize;
}

const char* string_getBuffer(string_t *string) {
  return (const char *)string->buffer;
}

size_t string_getSize(string_t *string) {
  return string->size;
}

void string_free(string_t *string) {
  if (string->buffer != 0)
    free(string->buffer);

  free(string);
}
