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

string_cursor_t *string_createCursor(string_t *string) {
  string_cursor_t *stringCursor = malloc(sizeof(string_cursor_t));
  if (stringCursor == 0)
    return 0;

  stringCursor->string = string;
  stringCursor->offset = 0;

  return stringCursor;
}

void string_resetCursor(string_cursor_t *stringCursor) {
  stringCursor->offset = 0;
}

// NOTE: Currently trusts the length to be within the bounds of the buffer
string_t *string_fromCopyWithLength(const char *buffer, size_t length) {
  string_t *string = string_create();
  if (string == 0)
    return 0;

  string->size = length;
  string->bufferSize = length + 1;
  string->buffer = malloc(sizeof(char) * (string->bufferSize));
  if (string->buffer == 0) {
    free(string);
    return 0;
  }

  memcpy(string->buffer, buffer, string->size);
  string->buffer[string->bufferSize - 1] = 0;

  return string;
}

string_t *string_fromCopy(const char *buffer) {
  size_t length = strlen(buffer);

  return string_fromCopyWithLength(buffer, length);
}

void string_append(string_t *string, const char *buffer) {
  size_t bufferSize = strlen(buffer);
  size_t newSize = string->size + bufferSize;

  // Expand if necessary
  if (newSize >= string->bufferSize) {
    char *expandedBuffer =
        realloc(string->buffer, sizeof(char) * (newSize + 1));
    if (expandedBuffer == 0)
      return;
    string->buffer = expandedBuffer;
    string->bufferSize = newSize + 1;
  }

  memcpy(string->buffer + string->size, buffer, bufferSize);
  string->buffer[string->bufferSize - 1] = 0;
  string->size = newSize;
}

const char *string_getBuffer(string_t *string) {
  return (const char *)string->buffer;
}

size_t string_getSize(string_t *string) {
  return string->size;
}

char string_getCharAt(string_t *string, size_t index) {
  // Out of bounds
  if (index >= string->size)
    return 0;

  char *characterAddress = (char *)(string->buffer + index);
  // The characterAddress was not set (null)
  if (characterAddress == 0)
    return 0;

  return (*characterAddress);
}

void string_setCharAt(string_t *string, size_t index, char character) {
  // Out of bounds
  if (index >= string->size)
    return;

  char *characterAddress = (char *)(string->buffer + index);
  // The character was not set (null)
  if (characterAddress == 0)
    return;

  (*characterAddress) = character;
}

string_t *string_substring(string_t *string, size_t firstIndex,
                           size_t lastIndex) {
  // Out of bounds
  if (firstIndex >= string->size || lastIndex > string->size)
    return 0;

  string_t *substring = string_fromCopyWithLength(string->buffer + firstIndex,
                                                  lastIndex - firstIndex);
  // Unable to allocate substring
  if (substring == 0)
    return 0;

  return substring;
}

char string_getNextChar(string_cursor_t *stringCursor) {
  if (stringCursor->offset >= stringCursor->string->size)
    return 0;

  return string_getCharAt(stringCursor->string, stringCursor->offset++);
}

string_t *string_getNextLine(string_cursor_t *stringCursor) {
  if (stringCursor->offset >= stringCursor->string->size)
    return 0;

  size_t stop;
  for (stop = stringCursor->offset; stop < stringCursor->string->size; stop++) {
    if (string_getCharAt(stringCursor->string, stop) == '\n')
      break;
  }

  // Don't include the newline
  string_t *line =
      string_substring(stringCursor->string, stringCursor->offset, stop);
  // Move past the newline
  stop++;
  stringCursor->offset = stop;

  return line;
}

void string_free(string_t *string) {
  if (string->buffer != 0)
    free(string->buffer);

  free(string);
}

void string_freeCursor(string_cursor_t *stringCursor) {
  free(stringCursor);
}
