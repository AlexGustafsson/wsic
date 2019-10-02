#ifndef PATH_H
#define PATH_H

#include "../string/string.h"

// Returns the absolute path within a root directory from a relative path within the root directoy
// Returns 0 if the path is outside of root or if the path does not exist
string_t *path_resolve(string_t *relativePath, string_t *root);

// Returns a path relative to the root directory
// Returns 0 if the path does not exist
string_t *path_relativeTo(string_t *path, string_t *root);

#endif
