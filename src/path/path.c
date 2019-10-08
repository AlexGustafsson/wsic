#include <stdlib.h>

#include "../logging/logging.h"

#include "path.h"

string_t *path_resolve(string_t *relativePath, string_t *root) {
  string_t *path = string_fromBufferWithLength(string_getBuffer(root), string_getSize(root));
  // Ensure that the root path ends with a '/' (doubles don't hurt)
  string_appendChar(path, '/');
  string_append(path, relativePath);

  char *resolvedPathBuffer = realpath(string_getBuffer(path), NULL);
  string_free(path);
  // The realpath call failed
  if (resolvedPathBuffer == 0)
    return 0;

  string_t *resolvedPath = string_fromBuffer(resolvedPathBuffer);
  free(resolvedPathBuffer);

  // Get the substring of the resolved path that should be equal to the root path
  string_t *resolvedRoot = string_substring(resolvedPath, 0, string_getSize(root));
  // If the substring is 0, it was likely out of bounds
  if (resolvedRoot == 0) {
    string_free(resolvedPath);
    return 0;
  }

  // If the resolved root is not the same as the actual root, the path is not within the root directory
  if (!string_equals(resolvedRoot, root)) {
    string_free(resolvedRoot);
    string_free(resolvedPath);
    return 0;
  }

  string_free(resolvedRoot);
  return resolvedPath;
}

string_t *path_relativeTo(string_t *path, string_t *root) {
  char *resolvedPathBuffer = realpath(string_getBuffer(path), NULL);
  // The realpath call failed
  if (resolvedPathBuffer == 0)
    return 0;

  string_t *resolvedPath = string_fromBuffer(resolvedPathBuffer);

  // Get the substring of the resolved path that should be equal to the root path
  string_t *resolvedRoot = string_substring(resolvedPath, 0, string_getSize(root));
  // If the substring is 0, it was likely out of bounds
  if (resolvedRoot == 0) {
    string_free(resolvedPath);
    return 0;
  }

  // If the resolved root is not the same as the actual root, the path is not within the root directory
  if (!string_equals(resolvedRoot, root)) {
    string_free(resolvedRoot);
    string_free(resolvedPath);
    return 0;
  }

  string_t *relativePath = string_substring(resolvedPath, string_getSize(root), string_getSize(resolvedPath));
  string_free(resolvedPath);
  return relativePath;
}
