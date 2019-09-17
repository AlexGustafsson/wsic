#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include "../logging/logging.h"

#include "resources.h"

string_t *resources_loadFile(string_t *filePath) {
  FILE *file = fopen(string_getBuffer(filePath), FILE_READ);
  if (file == 0) {
    log(LOG_ERROR, "Could not read file '%s' - got error %d", string_getBuffer(filePath), errno);
    return 0;
  }

  // Seek to the end of file to get file size
  fseek(file, 0L, SEEK_END);
  size_t fileSize = lseek((int)file, 0, SEEK_END) + 1;
  rewind(file);
  log(LOG_DEBUG, "Reading %zu bytes from '%s'", fileSize, string_getBuffer(filePath));
  // Create a string with a predefined (expandable) buffer
  string_t *content = string_create();
  string_setBufferSize(content, fileSize);

  // Read the file, character by character until EOF
  char current = 0;
  while((current = fgetc(file)) != EOF)
    string_appendChar(content, current);

  fclose(file);
  return content;
}
