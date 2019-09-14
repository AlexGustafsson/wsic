#ifndef CGI_H
#define CGI_H

#include <stdbool.h>
#include <stdint.h>

/*
typedef struct {
  // The root directory of your server
  const char *DOCUMENT_ROOT;
  // The visitor's cookie; if one is set
  const char *HTTP_COOKIE;
  // The hostname of the page being attempted
  const char *HTTP_HOST;
  // The URL of the page that called your program
  const char *HTTP_REFERER;
  // The browser type of the visitor
  const char *HTTP_USER_AGENT;
  // "on" if the program is being called through a secure server
  const char *HTTPS;
  // The system path your server is running under
  const char *PATH;
  // The query string (see GET; below)
  const char *QUERY_STRING;
  // The IP address of the visitor
  const char *REMOTE_ADDR;
  // The hostname of the visitor (if your server has reverse-name-lookups on; otherwise this is the IP address again)
  const char *REMOTE_HOST;
  // The port the visitor is connected to on the web server
  const char *REMOTE_PORT;
  // The visitor's username (for .htaccess-protected pages)
  const char *REMOTE_USER;
  // GET or POST
  const char *REQUEST_METHOD;
  // The interpreted pathname of the requested document or CGI (relative to the document root)
  const char *REQUEST_URI;
  // The full pathname of the current CGI
  const char *SCRIPT_FILENAME;
  // The interpreted pathname of the current CGI (relative to the document root)
  const char *SCRIPT_NAME;
  // The email address for your server's webmaster
  const char *SERVER_ADMIN;
  // Your server's fully qualified domain name (e.g. www.cgi101.com)
  const char *SERVER_NAME;
  // The port number your server is listening on
  const char *SERVER_PORT;
  // The server software you're using (e.g. Apache 1.3)
  const char *SERVER_SOFTWARE;
} cgi_environment_t;*/

typedef struct {
  pid_t pid;
  int stdin[2];
  int stdout[2];
  int stderr[2];
} cgi_process_t;

cgi_process_t *cgi_spawn(const char *command, char * const arguments[], char * const environment[]);

// NOTE: This is a blocking call
void cgi_read(cgi_process_t *process, char *buffer, size_t bufferSize);
size_t cgi_write(cgi_process_t *process, const char *buffer, size_t bufferSize);
// Flush the input to the process (no more writes can occur after this point)
void cgi_flushStdin(cgi_process_t *process);

// NOTE: This may not be accurate since the PID may be reused
bool cgi_isAlive(cgi_process_t *process);
// NOTE: This may close another process since the PID may be reused
void cgi_closeProcess(cgi_process_t *process);
// Wait for the process to exit. Returns the status code
int8_t cgi_waitForExit(cgi_process_t *process);

void cgi_freeProcess(cgi_process_t *process);

#endif
