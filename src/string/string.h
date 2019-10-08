#ifndef STRING_H
#define STRING_H

#include <stdbool.h>
#include <stdio.h>

/**
* Terminology:
* A 'string' is a string_t defined by this library
* A 'buffer' is a char buffer allocated to hold raw bytes
* A 'buffer-based string' is a null-terminated, "regular c string" buffer
* Notes:
* The buffer size is the full size of the buffer - including null byte
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
// Set a strings buffer size - allocates and frees as needed. Useful for preallocating space
void string_setBufferSize(string_t *string, size_t bufferSize) __attribute__((nonnull(1)));
// Clear the contents of the string
void string_clear(string_t *string) __attribute__((nonnull(1)));
// Create a cursor for a string - needs to be freed
string_cursor_t *string_createCursor(string_t *string) __attribute__((nonnull(1)));
// Reset a cursor to point to the start of a string
void string_resetCursor(string_cursor_t *cursor) __attribute__((nonnull(1)));
// Create a string by copying a buffer with a specified length
string_t *string_fromBufferWithLength(const char *buffer, size_t length)  __attribute__((nonnull(1)));
// Create a string by copying a buffer-based string
string_t *string_fromBuffer(const char *buffer) __attribute__((nonnull(1)));
// Create a string by duplicating another string;
string_t *string_copy(string_t *string) __attribute__((nonnull(1)));
// Create a string by converting from a base 10 integer
string_t *string_fromInt(int number);
// Append a string to a string
void string_append(string_t *string, string_t *string2) __attribute__((nonnull(1, 2)));
// Append a buffer-based string to a string
void string_appendBuffer(string_t *string, const char *buffer) __attribute__((nonnull(1, 2)));
// Append a buffer-based string to a string with a specific length
void string_appendBufferWithLength(string_t *string, const char *buffer, size_t bufferSize) __attribute__((nonnull(1, 2)));
// Append a char to a string
void string_appendChar(string_t *string, char character) __attribute__((nonnull(1)));
// Get the raw buffer of the string
const char *string_getBuffer(string_t *string) __attribute__((nonnull(1)));
// Get the actual size of the string (content's length)
size_t string_getSize(string_t *string) __attribute__((nonnull(1)));
// Get the character at a certain position within the string. Returns 0 if out of bounds or not found
char string_getCharAt(string_t *string, size_t index) __attribute__((nonnull(1)));
// Set the character at a certain position. Not guaranteed to set the character if out of bounds
void string_setCharAt(string_t *string, size_t index, char character) __attribute__((nonnull(1)));
// Removes whitespace at the end of a string
void string_trimEnd(string_t *string) __attribute__((nonnull(1)));
// Get a substring of a string with inclusive first index and exclusive last index
string_t *string_substring(string_t *string, size_t firstIndex, size_t lastIndex) __attribute__((nonnull(1)));
// Get the next character in a cursor
char string_getNextChar(string_cursor_t *cursor) __attribute__((nonnull(1)));
// Get the next line (excluding trailing newline (\r\n or \n)) in a cursor
string_t *string_getNextLine(string_cursor_t *cursor) __attribute__((nonnull(1)));
// Get the next index of a character in a cursor (-1 if not found)
ssize_t string_findNextChar(string_cursor_t *cursor, char needle) __attribute__((nonnull(1)));
// Get the current offset in a cursor
size_t string_getOffset(string_cursor_t *cursor) __attribute__((nonnull(1)));
// Set the current offset in a cursor (trusted to be within bounds)
void string_setOffset(string_cursor_t *cursor, size_t offset) __attribute__((nonnull(1)));
// Compare a string to a buffer-based string
bool string_equalsBuffer(string_t *string, const char *buffer) __attribute__((nonnull(1, 2)));
// Compare a string to another
bool string_equals(string_t *string, string_t *string2) __attribute__((nonnull(1, 2)));
// Free a string
void string_free(string_t *string) __attribute__((nonnull(1)));
// Free a cursor
void string_freeCursor(string_cursor_t *cursor) __attribute__((nonnull(1))); 

#endif
