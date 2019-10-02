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

string_t *resources_loadFile(string_t *filePath);
string_t *resources_getMIMEType(string_t *filePath);

#endif
