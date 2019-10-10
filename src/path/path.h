#ifndef PATH_H
#define PATH_H

#include "../string/string.h"

// Returns the absolute path within a root directory from a relative path within the root directoy
// Returns 0 if the path is outside of root or if the path does not exist
string_t *path_resolve(const string_t *relativePath, const string_t *root) __attribute__((nonnull(1, 2)));

// Returns a path relative to the root directory
// Returns 0 if the path does not exist
string_t *path_relativeTo(const string_t *path, const string_t *root) __attribute__((nonnull(1, 2)));

#endif
