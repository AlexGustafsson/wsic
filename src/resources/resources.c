#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../logging/logging.h"

#include "mime.h"
#include "resources.h"

string_t *resources_loadFile(string_t *filePath) {
  // Open the file in read mode and fail if the path is a directory
  FILE *file = fopen(string_getBuffer(filePath), "r+");
  if (file == 0) {
    log(LOG_ERROR, "Could not read file '%s' - got error %d", string_getBuffer(filePath), errno);
    return 0;
  }

  // Seek to the end of file to get file size
  if (fseek(file, 0L, SEEK_END) != 0) {
    log(LOG_ERROR, "Unable to seek to file end");
    fclose(file);
    return 0;
  }

  // Get the actual file size
  ssize_t fileSize = ftell(file);
  if (fileSize == -1) {
    log(LOG_ERROR, "Could not get filesize for '%s'. Got code %d", string_getBuffer(filePath), errno);
    fclose(file);
    return 0;
  }

  // Go back to the start of the file in order to read it
  rewind(file);

  log(LOG_DEBUG, "Reading %zu bytes from '%s'", fileSize, string_getBuffer(filePath));
  // Create a string with a predefined (expandable) buffer
  string_t *content = string_create();
  string_setBufferSize(content, fileSize);

  // Read the file, character by character until EOF
  char current = 0;
  while ((current = fgetc(file)) != EOF)
    string_appendChar(content, current);

  fclose(file);
  return content;
}

string_t *resources_getMIMEType(string_t *filePath) {
  // Get the extension of the file
  string_cursor_t *cursor = string_createCursor(filePath);
  ssize_t dotIndex = string_findNextChar(cursor, '.');
  string_freeCursor(cursor);
  // There's no "extension" in the path
  if (dotIndex == -1) {
    log(LOG_DEBUG, "No extension found in file path '%s'", string_getBuffer(filePath));
    return 0;
  }

  string_t *extension = string_substring(filePath, dotIndex, string_getSize(filePath));
  if (extension == 0) {
    log(LOG_DEBUG, "The extension could not be extracted from '%s' using start index %zd", string_getBuffer(filePath), dotIndex);
    return 0;
  }

  log(LOG_DEBUG, "Extension of '%s' is '%s'", string_getBuffer(filePath), string_getBuffer(extension));

  for (size_t i = 0; i < MIME_TYPES; i++) {
    if (string_equalsBuffer(extension, mime_extensions[i])) {
      string_free(extension);
      log(LOG_DEBUG, "Found mime type '%s'", mime_types[i]);
      return string_fromCopy(mime_types[i]);
    }
  }

  log(LOG_DEBUG, "Unknown MIME type for extension '%s'", string_getBuffer(extension));

  return 0;
}

bool resources_isExecutable(string_t *filePath) {
  struct stat info;
  if (stat(string_getBuffer(filePath), &info) == 0)
    return info.st_mode & S_IXUSR;
  else
    return false;
}

bool resources_isFile(string_t *filePath) {
  struct stat info;
  if (stat(string_getBuffer(filePath), &info) == 0)
    return S_ISREG(info.st_mode);
  else
    return false;
}
