#include <math.h>
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

void string_setBufferSize(string_t *string, size_t bufferSize) {
  if (bufferSize == string->bufferSize)
    return;

  // Reallocate with null termination included in the buffer size
  char *newBuffer = realloc(string->buffer, bufferSize + 1);
  if (newBuffer == 0)
    return;

  // Update the buffer and size
  string->bufferSize = bufferSize + 1;
  string->buffer = newBuffer;

  // Shrink the string size if larger than the buffer size
  if (string->size + 1 >= string->bufferSize)
    string->size = string->bufferSize - 1;

  // Clear memory beyond content size
  memset(string->buffer + string->size, 0, string->bufferSize - string->size);
}

void string_clear(string_t *string) {
  string->size = 0;
  if (string->bufferSize > 0)
    string->buffer[0] = 0;
}

string_cursor_t *string_createCursor(string_t *string) {
  string_cursor_t *cursor = malloc(sizeof(string_cursor_t));
  if (cursor == 0)
    return 0;

  cursor->string = string;
  cursor->offset = 0;

  return cursor;
}

void string_resetCursor(string_cursor_t *cursor) {
  cursor->offset = 0;
}

// NOTE: Currently trusts the length to be within the bounds of the buffer
string_t *string_fromBufferWithLength(const char *buffer, size_t length) {
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

string_t *string_fromBuffer(const char *buffer) {
  size_t length = strlen(buffer);

  return string_fromBufferWithLength(buffer, length);
}

string_t *string_copy(string_t *string) {
  string_t *copy = string_fromBufferWithLength(string_getBuffer(string), string_getSize(string));
  return copy;
}

string_t *string_fromInt(int number) {
  if (number == 0)
    return string_fromBuffer("0");

  string_t *string = string_create();
  // Get the rough number of digits in the number
  int digits = (int)log10(abs(number)) + 1;
  // Pre-allocate the rough number of necessary digits
  string_setBufferSize(string, digits);

  // Parse sign
  if (number < 0) {
    string_appendChar(string, '-');
    number = abs(number);
  }

  // Parse the integer left to right
  for (int offset = digits; offset > 0; offset--) {
    int digit = (int)(number / pow(10, offset - 1)) % 10;
    string_appendChar(string, (char)('0' + digit));
  }

  return string;
}

void string_append(string_t *string, string_t *string2) {
  string_appendBuffer(string, string2->buffer);
}

void string_appendBuffer(string_t *string, const char *buffer) {
  size_t bufferSize = strlen(buffer);
  string_appendBufferWithLength(string, buffer, bufferSize);
}

void string_appendBufferWithLength(string_t *string, const char *buffer, size_t bufferSize) {
  size_t newSize = string->size + bufferSize;

  // Expand if necessary
  if (newSize >= string->bufferSize) {
    char *expandedBuffer = realloc(string->buffer, sizeof(char) * (newSize + 1));
    if (expandedBuffer == 0)
      return;
    string->buffer = expandedBuffer;
    string->bufferSize = newSize + 1;
  }

  if (bufferSize > 0 && buffer[bufferSize - 1] == 0)
    memcpy(string->buffer + string->size, buffer, bufferSize - 1);
  else
    memcpy(string->buffer + string->size, buffer, bufferSize);
  string->buffer[string->bufferSize - 1] = 0;
  string->size = newSize;
}

void string_appendChar(string_t *string, char character) {
  char buffer[1] = {character};
  string_appendBufferWithLength(string, buffer, 1);
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

void string_trimEnd(string_t *string) {
  if (string->size == 0)
    return;

  for (ssize_t i = string->size - 1; i >= 0; i--) {
    char *character = string->buffer + i;
    if (character == 0)
      break;

    if (*character == '\n' || *character == '\r' || *character == ' ' || *character == '\t') {
      *character = 0;
      string->size--;
    } else {
      break;
    }
  }
}

string_t *string_substring(string_t *string, size_t firstIndex, size_t lastIndex) {
  // Out of bounds
  if (firstIndex >= string->size || lastIndex > string->size)
    return 0;
  // No buffer specified
  if (string->buffer == 0)
    return 0;

  string_t *substring = string_fromBufferWithLength(string->buffer + firstIndex, lastIndex - firstIndex);
  // Unable to allocate substring
  if (substring == 0)
    return 0;

  return substring;
}

char string_getNextChar(string_cursor_t *cursor) {
  if (cursor->offset >= cursor->string->size)
    return 0;

  return string_getCharAt(cursor->string, cursor->offset++);
}

string_t *string_getNextLine(string_cursor_t *cursor) {
  if (cursor->offset >= cursor->string->size)
    return 0;

  size_t start = cursor->offset;
  // The index to the last newline character (\r<\n> or <\n>)
  ssize_t lastLineIndex = string_findNextChar(cursor, '\n');
  // The index to the last character of the content
  ssize_t lastContentIndex = -1;
  if (lastLineIndex > 0) {
    // Content is anything leading up to the newline
    if (string_getCharAt(cursor->string, lastLineIndex - 1) == '\r')
      lastContentIndex = lastLineIndex - 1;
    else
      lastContentIndex = lastLineIndex;
  } else if (lastContentIndex == -1) {
    // No more newlines in the string, content is anything up until EOF
    lastContentIndex = cursor->string->size;
  }

  string_t *line = 0;
  if (lastContentIndex >= (ssize_t)start)
    line = string_substring(cursor->string, start, lastContentIndex);
  else
    line = string_fromBuffer("");

  return line;
}

ssize_t string_findNextChar(string_cursor_t *cursor, char needle) {
  ssize_t index = -1;
  char current = 0;
  while ((current = string_getNextChar(cursor)) != 0) {
    if (current == needle) {
      index = cursor->offset - 1;
      break;
    }
  }

  return index;
}

size_t string_getOffset(string_cursor_t *cursor) {
  return cursor->offset;
}

void string_setOffset(string_cursor_t *cursor, size_t offset) {
  cursor->offset = offset;
}

bool string_equalsBuffer(string_t *string, const char *buffer) {
  return strcmp(string->buffer, buffer) == 0;
}

bool string_equals(string_t *string, string_t *string2) {
  return string_equalsBuffer(string, string2->buffer);
}

void string_free(string_t *string) {
  if (string->buffer != 0)
    free(string->buffer);

  free(string);
}

void string_freeCursor(string_cursor_t *cursor) {
  free(cursor);
}
