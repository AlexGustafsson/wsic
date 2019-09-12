#ifndef STRING_H
#define STRING_H

#include <stdio.h>

/**
* Terminology:
* A 'string' is a string_t defined by this library.
* A 'buffer' is a char buffer allocated to hold raw bytes.
* A 'buffer-based string' is a null-terminated, "regular c string" buffer
*/

typedef struct {
  char *buffer;
  // The buffer's total size (for internal use only)
  size_t bufferSize;
  // The current size of the string (content length, not buffer size)
  size_t size;
} string_t;

typedef struct {
  string_t *string;
  size_t offset;
} string_cursor_t;

// Create an empty string - needs to be freed
string_t *string_create();
// Create a cursor for a string - needs to be freed
string_cursor_t *string_createCursor(string_t *string);
// Reset a cursor to point to the start of a string
void string_resetCursor(string_cursor_t *stringCursor);
// Create a string by copying a buffer with a specified length
string_t *string_fromCopyWithLength(const char *buffer, size_t length);
// Create a string by copying a buffer-based string
string_t *string_fromCopy(const char *buffer);
// Append a buffer-based string to a string
void string_append(string_t *string, const char *buffer);
// Get the raw buffer of the string
const char* string_getBuffer(string_t *string);
// Get the actual size of the string (content's length)
size_t string_getSize(string_t *string);
// Get the character at a certain position within the string. Returns 0 if out of bounds or not found
char string_getCharAt(string_t *string, size_t index);
// Set the character at a certain position. Not guaranteed to set the character if out of bounds
void string_setCharAt(string_t *string, size_t index, char character);
// Get a substring of a string with inclusive index - needs to be freed
string_t *string_substring(string_t *string, size_t firstIndex, size_t lastIndex);
// Get the next character in a cursor
char string_getNextChar(string_cursor_t *stringCursor);
// Get the next line (excluding trailing newline) in a cursor
string_t *string_getNextLine(string_cursor_t *stringCursor);
// Free a string
void string_free(string_t *string);
// Free a cursor
void string_freeCursor(string_cursor_t *stringCursor);

#endif
