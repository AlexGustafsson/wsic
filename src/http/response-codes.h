#ifndef RESPONSE_CODES_H
#define RESPONSE_CODES_H

#include <stdint.h>

#include "../string/string.h"

// Exported from developer.mozilla.org 2019/09/11
#define HTTP_CODE_TEXT_100 "Continue"
#define HTTP_CODE_TEXT_101 "Switching Protocol"
#define HTTP_CODE_TEXT_102 "Processing"
#define HTTP_CODE_TEXT_103 "Early Hints"
#define HTTP_CODE_TEXT_200 "OK"
#define HTTP_CODE_TEXT_201 "Created"
#define HTTP_CODE_TEXT_202 "Accepted"
#define HTTP_CODE_TEXT_203 "Non-Authoritative Information"
#define HTTP_CODE_TEXT_204 "No Content"
#define HTTP_CODE_TEXT_205 "Reset Content"
#define HTTP_CODE_TEXT_206 "Partial Content"
#define HTTP_CODE_TEXT_207 "Multi-Status"
#define HTTP_CODE_TEXT_208 "Multi-Status"
#define HTTP_CODE_TEXT_226 "IM Used"
#define HTTP_CODE_TEXT_300 "Multiple Choice"
#define HTTP_CODE_TEXT_301 "Moved Permanently"
#define HTTP_CODE_TEXT_302 "Found"
#define HTTP_CODE_TEXT_303 "See Other"
#define HTTP_CODE_TEXT_304 "Not Modified"
#define HTTP_CODE_TEXT_305 "Use Proxy"
#define HTTP_CODE_TEXT_306 "unused"
#define HTTP_CODE_TEXT_307 "Temporary Redirect"
#define HTTP_CODE_TEXT_308 "Permanent Redirect"
#define HTTP_CODE_TEXT_400 "Bad Request"
#define HTTP_CODE_TEXT_401 "Unauthorized"
#define HTTP_CODE_TEXT_402 "Payment Required"
#define HTTP_CODE_TEXT_403 "Forbidden"
#define HTTP_CODE_TEXT_404 "Not Found"
#define HTTP_CODE_TEXT_405 "Method Not Allowed"
#define HTTP_CODE_TEXT_406 "Not Acceptable"
#define HTTP_CODE_TEXT_407 "Proxy Authentication Required"
#define HTTP_CODE_TEXT_408 "Request Timeout"
#define HTTP_CODE_TEXT_409 "Conflict"
#define HTTP_CODE_TEXT_410 "Gone"
#define HTTP_CODE_TEXT_411 "Length Required"
#define HTTP_CODE_TEXT_412 "Precondition Failed"
#define HTTP_CODE_TEXT_413 "Payload Too Large"
#define HTTP_CODE_TEXT_414 "URI Too Long"
#define HTTP_CODE_TEXT_415 "Unsupported Media Type"
#define HTTP_CODE_TEXT_416 "Requested Range Not Satisfiable"
#define HTTP_CODE_TEXT_417 "Expectation Failed"
#define HTTP_CODE_TEXT_418 "I'm a teapot"
#define HTTP_CODE_TEXT_421 "Misdirected Request"
#define HTTP_CODE_TEXT_422 "Unprocessable Entity"
#define HTTP_CODE_TEXT_423 "Locked"
#define HTTP_CODE_TEXT_424 "Failed Dependency"
#define HTTP_CODE_TEXT_425 "Too Early"
#define HTTP_CODE_TEXT_426 "Upgrade Required"
#define HTTP_CODE_TEXT_428 "Precondition Required"
#define HTTP_CODE_TEXT_429 "Too Many Requests"
#define HTTP_CODE_TEXT_431 "Request Header Fields Too Large"
#define HTTP_CODE_TEXT_451 "Unavailable For Legal Reasons"
#define HTTP_CODE_TEXT_500 "Internal Server Error"
#define HTTP_CODE_TEXT_501 "Not Implemented"
#define HTTP_CODE_TEXT_502 "Bad Gateway"
#define HTTP_CODE_TEXT_503 "Service Unavailable"
#define HTTP_CODE_TEXT_504 "Gateway Timeout"
#define HTTP_CODE_TEXT_505 "HTTP Version Not Supported"
#define HTTP_CODE_TEXT_506 "Variant Also Negotiates"
#define HTTP_CODE_TEXT_507 "Insufficient Storage"
#define HTTP_CODE_TEXT_508 "Loop Detected"
#define HTTP_CODE_TEXT_510 "Not Extended"
#define HTTP_CODE_TEXT_511 "Network Authentication Required"
#define HTTP_CODE_TEXT_UNKNOWN ""

// NOTE: Returns strings allocated on the stack - no need to malloc or free
string_t *http_codeToString(uint16_t code);

#endif
