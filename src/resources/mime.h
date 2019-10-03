#ifndef MIME_H
#define MIME_H

/**
* The supported types were parsed from Mozillas list available here:
* https://developer.mozilla.org/en-US/docs/Web/HTTP/Basics_of_HTTP/MIME_types/Complete_list_of_MIME_types
*/

// The number of supported MIME types
#define MIME_TYPES 63

// An array of all supported file extensions
extern const char *mime_extensions[MIME_TYPES];
// An array of all MIME types
extern const char *mime_types[MIME_TYPES];

#endif
