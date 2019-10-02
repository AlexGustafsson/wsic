#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "../logging/logging.h"

#include "cgi.h"

#define PIPE_READ 0
#define PIPE_WRITE 1

cgi_process_t *cgi_spawn(const char *command, list_t *arguments, hash_table_t *environment) {
  cgi_process_t *process = malloc(sizeof(cgi_process_t));
  if (process == 0)
    return 0;

  if (pipe(process->stdin) < 0) {
    log(LOG_ERROR, "Could not allocate STDIN pipe for child process");

    cgi_freeProcess(process);
    return 0;
  }

  if (pipe(process->stdout) < 0) {
    log(LOG_ERROR, "Could not allocate STDOUT pipe for child process");

    cgi_freeProcess(process);
    return 0;
  }

  if (pipe(process->stderr) < 0) {
    log(LOG_ERROR, "Could not allocate STDERR pipe for child process");

    cgi_freeProcess(process);
    return 0;
  }

  // Fork the process
  fflush(stdout);
  fflush(stderr);
  process->pid = fork();

  if (process->pid < 0) {
    // Process creation failed
    cgi_freeProcess(process);

    return 0;
  } else if (process->pid == 0) {
    // This is run by the new child process

    // Try to redirect STDIN
    if (dup2(process->stdin[PIPE_READ], STDIN_FILENO) < 0)
      exit(errno);
    // Close the old end of the pipes
    close(process->stdin[PIPE_READ]);
    close(process->stdin[PIPE_WRITE]);

    // Try to redirect STDOUT
    if (dup2(process->stdout[PIPE_WRITE], STDOUT_FILENO) < 0)
      exit(errno);
    // Close the old end of the pipes
    close(process->stdout[PIPE_READ]);
    close(process->stdout[PIPE_WRITE]);

    // Try to redirect STDERR
    if (dup2(process->stderr[PIPE_WRITE], STDERR_FILENO) < 0)
      exit(errno);
    // Close the old end of the pipes
    close(process->stderr[PIPE_READ]);
    close(process->stderr[PIPE_WRITE]);

    const char **argumentsBuffer = 0;
    if (arguments != 0) {
      argumentsBuffer = malloc(sizeof(char *) * (list_getLength(arguments) + 1));
      for (size_t i = 0; i < list_getLength(arguments); i++) {
        string_t *argumentString = list_removeValue(arguments, i);
        argumentsBuffer[i] = string_getBuffer(argumentString);
      }
      list_free(arguments);
      argumentsBuffer[list_getLength(arguments)] = 0;
    }

    char **environmentBuffer = 0;
    if (environment != 0) {
      environmentBuffer = malloc(sizeof(char *) * (hash_table_getLength(environment) + 1));
      for (size_t i = 0; i < hash_table_getLength(environment); i++) {
        string_t *key = hash_table_getKeyByIndex(environment, i);
        string_t *value = hash_table_getValueByIndex(environment, i);

        string_t *keyValuePair = string_fromCopyWithLength(string_getBuffer(key), string_getSize(key));
        string_appendChar(keyValuePair, '=');
        string_append(keyValuePair, value);
        string_free(value);

        environmentBuffer[i] = string_getBuffer(keyValuePair);
      }
      environmentBuffer[hash_table_getLength(environment)] = 0;
      hash_table_free(environment);
    }

    // Run the CGI command
    int exitCode = execve(command, (char *const *)argumentsBuffer, environmentBuffer);

    if (exitCode == -1) {
      if (errno == ENOENT)
        log(LOG_ERROR, "No such file or directory: %s", command);
    }

    exit(exitCode);
  }

  // Close pipes meant for the child's use only
  close(process->stdin[PIPE_READ]);
  close(process->stdout[PIPE_WRITE]);

  // The child creation succeeded, return the process struct
  return process;
}

void cgi_read(cgi_process_t *process, char *buffer, size_t bufferSize) {
  char current = 0;
  size_t offset = 0;
  while (read(process->stdout[PIPE_READ], &current, 1) && offset < bufferSize)
    buffer[offset++] = current;
}

size_t cgi_write(cgi_process_t *process, const char *buffer, size_t bufferSize) {
  ssize_t bytesSent = write(process->stdin[PIPE_WRITE], buffer, bufferSize);

  if (bytesSent == -1) {
    log(LOG_ERROR, "Could not write to process");

    return 0;
  }

  log(LOG_DEBUG, "Successfully wrote %zu bytes to process", bufferSize);
  return bytesSent;
}

void cgi_flushStdin(cgi_process_t *process) {
  const char buffer[1] = {0};
  write(process->stdin[PIPE_WRITE], buffer, 1);
  close(process->stdin[PIPE_WRITE]);
}

bool cgi_isAlive(cgi_process_t *process) {
  int status;
  pid_t pid = waitpid(process->pid, &status, WNOHANG);

  return pid == 0;
}

int8_t cgi_waitForExit(cgi_process_t *process) {
  int status;
  waitpid(process->pid, &status, 0);

  return WEXITSTATUS(status);
}

void cgi_closeProcess(cgi_process_t *process) {
  kill(process->pid, SIGKILL);

  // Close stdin
  close(process->stdin[PIPE_READ]);
  close(process->stdin[PIPE_WRITE]);

  // Close stdout
  close(process->stdout[PIPE_READ]);
  close(process->stdout[PIPE_WRITE]);

  // Close stderr
  close(process->stderr[PIPE_READ]);
  close(process->stderr[PIPE_WRITE]);
}

void cgi_freeProcess(cgi_process_t *process) {
  cgi_closeProcess(process);
  free(process);
}
