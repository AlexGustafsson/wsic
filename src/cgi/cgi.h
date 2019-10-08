#ifndef CGI_H
#define CGI_H

#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#include "../datastructures/hash-table/hash-table.h"

typedef struct {
  pid_t pid;
  int stdin[2];
  int stdout[2];
  int stderr[2];
} cgi_process_t;

cgi_process_t *cgi_spawn(const char *command, list_t *arguments, hash_table_t *environment);

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
