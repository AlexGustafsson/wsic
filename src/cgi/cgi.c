#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
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
        string_t *argumentString = list_getValue(arguments, i);
        argumentsBuffer[i] = string_getBuffer(argumentString);
      }
      argumentsBuffer[list_getLength(arguments)] = 0;
      list_free(arguments);
    } else {
      argumentsBuffer = malloc(sizeof(char *) * 1);
      argumentsBuffer[0] = 0;
    }

    char **environmentBuffer = 0;
    if (environment != 0) {
      environmentBuffer = malloc(sizeof(char *) * (hash_table_getLength(environment) + 1));
      for (size_t i = 0; i < hash_table_getLength(environment); i++) {
        string_t *key = hash_table_getKeyByIndex(environment, i);
        string_t *value = hash_table_getValueByIndex(environment, i);

        string_t *keyValuePair = string_fromBufferWithLength(string_getBuffer(key), string_getSize(key));
        string_appendChar(keyValuePair, '=');
        string_append(keyValuePair, value);
        string_free(value);

        environmentBuffer[i] = (char *)string_getBuffer(keyValuePair);
      }
      environmentBuffer[hash_table_getLength(environment)] = 0;
      hash_table_free(environment);
    } else {
      environmentBuffer = malloc(sizeof(char *) * 1);
      environmentBuffer[0] = 0;
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

string_t *cgi_read(const cgi_process_t *process, size_t timeout) {
  string_t *content = string_create();
  if (content == 0) {
    log(LOG_ERROR, "Failed to allocate string for content");
    return 0;
  }

  uint8_t timeouts = 0;
  while (true) {
    // Set up structures necessary for polling
    struct pollfd descriptors[1];
    memset(descriptors, 0, sizeof(struct pollfd));
    descriptors[0].fd = process->stdout[PIPE_READ];
    descriptors[0].events = POLLIN;

    log(LOG_DEBUG, "Waiting for data to be readable");

    // Wait for the pipe to be ready to read
    int status = poll(descriptors, 1, timeout);
    if (status <= -1) {
      log(LOG_ERROR, "Could not wait for CGI pipe to write data");
      string_free(content);
      return 0;
    } else if (status == 0) {
      // Try again if the poll timed out
      if (timeouts++ > 5)
        return 0;
      continue;
    }

    int bytesAvailable = 0;
    if (ioctl(process->stdout[PIPE_READ], FIONREAD, &bytesAvailable) == -1) {
      const char *reason = strerror(errno);
      log(LOG_ERROR, "Could not check number of bytes available for reading. Got code %d (%s)", errno, reason);
      string_free(content);
      return 0;
    }

    if (bytesAvailable == 0) {
      log(LOG_DEBUG, "Read %zu bytes from the CGI process", string_getSize(content));
      return content;
    }

    log(LOG_DEBUG, "There are %d bytes available for reading", bytesAvailable);

    char *buffer = malloc(sizeof(char) * bytesAvailable);
    ssize_t bytesRead = read(process->stdout[PIPE_READ], buffer, bytesAvailable);
    if (bytesRead < 0) {
      log(LOG_ERROR, "Unable to read bytes from CGI process - got code %d", errno);
      free(buffer);
      string_free(content);
      return 0;
    }
    string_appendBufferWithLength(content, buffer, bytesRead);
    free(buffer);
  }

  return content;
}

size_t cgi_write(const cgi_process_t *process, const char *buffer, size_t bufferSize) {
  ssize_t bytesSent = write(process->stdin[PIPE_WRITE], buffer, bufferSize);

  if (bytesSent == -1) {
    log(LOG_ERROR, "Could not write to process");

    return 0;
  }

  log(LOG_DEBUG, "Successfully wrote %zu bytes to process", bufferSize);
  return bytesSent;
}

void cgi_flushStdin(const cgi_process_t *process) {
  const char buffer[1] = {0};
  ssize_t bytesWritten = write(process->stdin[PIPE_WRITE], buffer, 1);
  if (bytesWritten == -1)
    log(LOG_ERROR, "Unable to write to CGI process - got code %d", errno);
  close(process->stdin[PIPE_WRITE]);
}

bool cgi_isAlive(const cgi_process_t *process) {
  int status;
  pid_t pid = waitpid(process->pid, &status, WNOHANG);

  return pid == 0;
}

int8_t cgi_waitForExit(const cgi_process_t *process) {
  int status;
  waitpid(process->pid, &status, 0);

  return WEXITSTATUS(status);
}

void cgi_closeProcess(const cgi_process_t *process) {
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
