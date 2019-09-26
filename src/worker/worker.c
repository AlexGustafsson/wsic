#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "../cgi/cgi.h"
#include "../http/http.h"
#include "../logging/logging.h"
#include "../string/string.h"

#include "worker.h"

// Private methods
// The main entry point of a worker
int worker_entryPoint();
int worker_handleConnection(connection_t *connection);

worker_t *worker_spawn(connection_t *connection) {
  worker_t *worker = malloc(sizeof(worker_t));
  if (worker == 0)
    return 0;
  memset(worker, 0, sizeof(worker_t));

  if (connection == 0) {
    // Create a condition for when the thread should sleep
    if (pthread_cond_init(&worker->sleepCondition, NULL) != 0) {
      log(LOG_ERROR, "Unable to create sleeping condition");
      free(worker);
      return 0;
    }

    // Create a mutex locked while working
    if (pthread_mutex_init(&worker->sleepMutex, NULL) != 0) {
      log(LOG_ERROR, "Unable to create sleeping mutex");
      pthread_cond_destroy(&worker->sleepCondition);
      free(worker);
      return 0;
    }
  } else {
    worker->connection = connection;
  }

  pthread_t thread;

  if (pthread_create(&thread, NULL, worker_entryPoint, worker) != 0) {
    log(LOG_ERROR, "Unable to start thread for worker");
    free(worker);
    if (connection != 0) {
      pthread_cond_destroy(&worker->sleepCondition);
      pthread_mutex_destroy(&worker->sleepMutex);
    }
    return 0;
  }

  if (connection == 0) {
    // Due to the thread already being started and may have already
    // freed itself, it would be a race condition to add the thread here
    // if in immediate mode - don't do it
    worker->thread = thread;

    log(LOG_DEBUG, "Created thread in pool mode");
    return worker;
  } else {
    log(LOG_DEBUG, "Created thread in immediate mode");
    return worker;
  }
}

uint8_t worker_getStatus(worker_t *worker) {
  return worker->status;
}

void worker_setConnection(worker_t *worker, connection_t *connection) {
  // TODO: Thread safety
  pthread_mutex_lock(&worker->sleepMutex);
  worker->connection = connection;
  pthread_cond_signal(&worker->sleepCondition);
  pthread_mutex_unlock(&worker->sleepMutex);
}

void worker_setStatus(worker_t *worker, uint8_t status) {
  worker->status = status;
}

connection_t *worker_getConnection(worker_t *worker) {
  return worker->connection;
}

int worker_entryPoint(worker_t *worker) {
  // If a connection is already set, handle it directly (immediate mode)
  if (worker->connection != 0) {
    log(LOG_DEBUG, "Handling a connection in immediate mode");
    worker->status = WORKER_STATUS_WORKING;
    int exitCode = worker_handleConnection(worker->connection);
    log(LOG_DEBUG, "Connection handling exited with code %d", exitCode);
    if (exitCode != 0)
      log(LOG_ERROR, "Handling the connection resulted in a non-zero exit code: %d", exitCode);
    worker->status = WORKER_STATUS_IDLE;
    worker_free(worker);
    return exitCode;
  }

  log(LOG_DEBUG, "Initializing worker");

  // Run the thread continously to handle multiple connections (pool mode)
  while (true) {
    log(LOG_DEBUG, "Putting worker to sleep");
    worker->status = WORKER_STATUS_IDLE;

    // Lock the thread until a connection is available
    pthread_mutex_lock(&worker->sleepMutex);
    while (worker->connection == 0)
      pthread_cond_wait(&worker->sleepCondition, &worker->sleepMutex);
    pthread_mutex_unlock(&worker->sleepMutex);

    log(LOG_DEBUG, "Worker process interrupted by parent to handle a connection");

    worker->status = WORKER_STATUS_WORKING;

    worker_handleConnection(worker->connection);
    log(LOG_DEBUG, "Handled connection - closing it");
    // Free the connection as it's of no further use
    connection_free(worker->connection);

    worker->connection = 0;
  }

  return 0;
}

int worker_handleConnection(connection_t *connection) {
  http_t *http = http_create();

  // Start reading the header from the client
  string_t *currentLine = 0;
  size_t line = 0;
  size_t headerSize = 0;
  while (true) {
    currentLine = connection_readLine(connection, REQUEST_READ_TIMEOUT, REQUEST_MAX_HEADER_SIZE - headerSize);
    log(LOG_DEBUG, "Got line %s", string_getBuffer(currentLine));
    // Stop if there was no line read or the line was empty (all headers were read)
    if (currentLine == 0 || string_getSize(currentLine) == 0)
      break;
    headerSize += string_getSize(currentLine);
    if (line == 0) {
      // Parse the request line
      bool parsed = http_parseRequestLine(http, currentLine);
      if (!parsed) {
        log(LOG_ERROR, "Failed to parse request line '%s'. Closing connection", string_getBuffer(currentLine));
        string_free(currentLine);
        http_free(http);
        return 1;
      }
    } else {
      // Parse the header
      bool parsed = http_parseHeader(http, currentLine);
      if (!parsed) {
        log(LOG_ERROR, "Failed to parse header '%s'. Closing connection", string_getBuffer(currentLine));
        string_free(currentLine);
        http_free(http);
        return 1;
      }
    }
    string_free(currentLine);
    currentLine = 0;
    line++;
  }
  bool parsedHost = http_parseHost(http);
  if (!parsedHost) {
    log(LOG_DEBUG, "Could not parse Host header");
    http_free(http);
    return 1;
  }

  if (http->url == 0) {
    log(LOG_ERROR, "Didn't receive a header from the request");
    http_free(http);
    return 1;
  }

  // Handle expect header (rudimentary support)
  string_t *expectHeader = string_fromCopy("Expect");
  string_t *expects = http_getHeader(http, expectHeader);
  string_free(expectHeader);
  if (expects != 0) {
    log(LOG_DEBUG, "Got expect '%s'", string_getBuffer(expects));
    // TODO: Handle actual expects here
    connection_write(connection, "HTTP/1.1 100 Continue\r\n", 23);
  }

  // Read the body if one exists
  string_t *body = 0;
  string_t *contentLengthHeader = string_fromCopy("Content-Length");
  string_t *contentLengthString = http_getHeader(http, contentLengthHeader);
  string_free(contentLengthHeader);
  if (contentLengthString != 0) {
    log(LOG_DEBUG, "Content-Length: %s", string_getBuffer(contentLengthString));
    int contentLength = atoi(string_getBuffer(contentLengthString));
    log(LOG_DEBUG, "The request has a body size of %d bytes", contentLength);
    if (contentLength > REQUEST_MAX_BODY_SIZE) {
      log(LOG_WARNING, "The client wanted to write %d bytes which is above maximum %d", contentLength, REQUEST_MAX_BODY_SIZE);
      http_free(http);
      return 1;
    } else if (contentLength == 0) {
      log(LOG_WARNING, "Got empty body");
    } else {
      // TODO: Use connection_read when implemented properly - don't use strings this way
      body = string_create();
      connection_readBytes(connection, &body->buffer, contentLength, READ_FLAGS_NONE);
      body->size = contentLength;
      string_setBufferSize(body, contentLength);
    }
  }

  list_t *arguments = 0;
  hash_table_t *environment = hash_table_create();
  hash_table_setValue(environment, string_fromCopy("HTTPS"), string_fromCopy("off"));
  hash_table_setValue(environment, string_fromCopy("SERVER_SOFTWARE"), string_fromCopy("WSIC"));
  uint8_t method = http_getMethod(http);
  if (method == GET)
    hash_table_setValue(environment, string_fromCopy("REQUEST_METHOD"), string_fromCopy("GET"));
  else if (method == POST)
    hash_table_setValue(environment, string_fromCopy("REQUEST_METHOD"), string_fromCopy("POST"));

  log(LOG_DEBUG, "Spawning CGI process");
  cgi_process_t *process = cgi_spawn("/Users/alexgustafsson/Documents/GitHub/wsic/cgi-test.sh", arguments, environment);
  log(LOG_DEBUG, "Spawned process with pid %d", process->pid);

  // Write body to CGI
  if (body != 0) {
    log(LOG_DEBUG, "Writing request to CGI process");
    log(LOG_DEBUG, "Body content is:\n<%s> of size %zu", string_getBuffer(body), string_getSize(body));
    cgi_write(process, string_getBuffer(body), string_getSize(body));
    // Make sure the process receives EOF
    cgi_flushStdin(process);
  } else {
    // Close the input to the CGI process
    cgi_flushStdin(process);
  }

  log(LOG_DEBUG, "Reading response from CGI process");
  // TODO: Read more than 4096 bytes
  char buffer[4096] = {0};
  cgi_read(process, buffer, 4096);
  buffer[4096 - 1] = 0;

  log(LOG_DEBUG, "Got response from CGI process");
  connection_write(connection, buffer, 4096);

  // NOTE: Not necessary, but for debugging it's nice to know
  // that the process is actually exiting (not kept forever)
  // since we don't currently kill spawned processes
  log(LOG_DEBUG, "Waiting for process to exit");
  uint8_t exitCode = cgi_waitForExit(process);
  log(LOG_DEBUG, "Process exited with status %d", exitCode);

  // Close and free up CGI process and connection
  cgi_freeProcess(process);

  return 0;
}

void worker_waitForExit(worker_t *worker) {
  pthread_join(worker->thread, NULL);
}

void worker_kill(worker_t *worker) {
  pthread_cancel(worker->thread);
  // Ensure that the sleep mutex is unlocked
  pthread_mutex_unlock(&worker->sleepMutex);
  // Ensure that the condition is destroyed after the threads has exited
  pthread_cond_destroy(&worker->sleepCondition);
}

void worker_free(worker_t *worker) {
  if (worker->connection != 0)
    connection_free(worker->connection);
  // Free the worker itself
  free(worker);
}
