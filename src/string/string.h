#ifndef STRING_H
#define STRING_H

typedef struct {
  char *buffer;
  // The buffer's total size (for internal use only)
  size_t bufferSize;
  // The current size of the string (content length, not buffer size)
  size_t size;
} string_t;

string_t *string_create();
string_t *string_fromCopy(const char *buffer);
void string_append(string_t *string, const char *buffer);
const char* string_getBuffer(string_t *string);
size_t string_getSize(string_t *string);

void string_free(string_t *string);

#endif
