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

// Spawn a CGI process. Owns arguments and environment.
cgi_process_t *cgi_spawn(const char *command, list_t *arguments, hash_table_t *environment) __attribute__((nonnull(1)));

// NOTE: This will the read all available bytes. Will block until the pipe has data to read
string_t *cgi_read(const cgi_process_t *process, size_t timeout) __attribute__((nonnull(1)));
size_t cgi_write(const cgi_process_t *process, const char *buffer, size_t bufferSize) __attribute__((nonnull(1, 2)));
// Flush the input to the process (no more writes can occur after this point)
void cgi_flushStdin(const cgi_process_t *process) __attribute__((nonnull(1)));

// NOTE: This may not be accurate since the PID may be reused
bool cgi_isAlive(const cgi_process_t *process) __attribute__((nonnull(1)));
// NOTE: This may close another process since the PID may be reused
void cgi_closeProcess(const cgi_process_t *process) __attribute__((nonnull(1)));
// Wait for the process to exit. Returns the status code
int8_t cgi_waitForExit(const cgi_process_t *process) __attribute__((nonnull(1)));

void cgi_freeProcess(cgi_process_t *process) __attribute__((nonnull(1)));

#endif
