#ifndef RESOURCES_H
#define RESOURCES_H

#include "../string/string.h"

#include "resources/www/400.html.h"
#include "resources/www/403.html.h"
#include "resources/www/404.html.h"
#include "resources/www/500.html.h"
#include "resources/www/501.html.h"
#include "resources/www/template.html.h"

#include "resources/config/default-config.toml.h"

#define FILE_READ "r"
#define FILE_WRITE "w"
#define FILE_READ_WRITE "rw"

// Read a file as a string (does not follow symlinks)
string_t *resources_loadFile(string_t *filePath);
// Try to get the MIME type of a file (does not follow symlinks)
string_t *resources_getMIMEType(string_t *filePath);
// Whether or not the user can execute the file path (does not follow symlinks, works for files and directories)
bool resources_isExecutable(string_t *filePath);
// Whether or not a path is a regular file (does not follow symlinks)
bool resources_isFile(string_t *filePath);

#endif
