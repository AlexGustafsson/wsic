#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "../http/http.h"
#include "../logging/logging.h"
#include "../string/string.h"
#include "../www/www.h"

#include "worker.h"

// Private methods
// The main entry point of a worker
int worker_entryPoint();
int worker_handleConnection(connection_t *connection);

worker_t *worker_spawn(int id, connection_t *connection, message_queue_t *queue) {
  worker_t *worker = malloc(sizeof(worker_t));
  if (worker == 0)
    return 0;
  memset(worker, 0, sizeof(worker_t));

  worker->id = id;
  worker->connection = connection;
  worker->queue = queue;

  pthread_t thread;

  if (pthread_create(&thread, NULL, worker_entryPoint, worker) != 0) {
    log(LOG_ERROR, "Unable to start thread for worker");
    free(worker);
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
    log(LOG_DEBUG, "Putting worker to sleep, waiting for a connection");
    worker->status = WORKER_STATUS_IDLE;

    // Wait for a connection to be available (may lock indefinitely)
    worker->connection = message_queue_pop(worker->queue);

    log(LOG_DEBUG, "Worker process interrupted by parent to handle a connection (worker %d)", worker->id);

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
  http_t *request = http_create();

  // Start reading the header from the client
  string_t *currentLine = 0;
  size_t line = 0;
  size_t headerSize = 0;
  while (true) {
    currentLine = connection_readLine(connection, REQUEST_READ_TIMEOUT, REQUEST_MAX_HEADER_SIZE - headerSize);
    if (currentLine != 0)
      log(LOG_DEBUG, "Got line %s", string_getBuffer(currentLine));
    // Stop if there was no line read or the line was empty (all headers were read)
    if (currentLine == 0 || string_getSize(currentLine) == 0)
      break;
    headerSize += string_getSize(currentLine);
    if (line == 0) {
      // Parse the request line
      bool parsed = http_parseRequestLine(request, currentLine);
      if (!parsed) {
        log(LOG_ERROR, "Failed to parse request line '%s'. Closing connection", string_getBuffer(currentLine));
        string_free(currentLine);
        http_free(request);
        return 1;
      }
    } else {
      // Parse the header
      bool parsed = http_parseHeader(request, currentLine);
      if (!parsed) {
        log(LOG_ERROR, "Failed to parse header '%s'. Closing connection", string_getBuffer(currentLine));
        string_free(currentLine);
        http_free(request);
        return 1;
      }
    }
    string_free(currentLine);
    currentLine = 0;
    line++;
  }
  bool parsedHost = http_parseHost(request);
  if (!parsedHost) {
    log(LOG_DEBUG, "Could not parse Host header");
    http_free(request);
    return 1;
  }

  if (request->url == 0) {
    log(LOG_ERROR, "Didn't receive a header from the request");
    http_free(request);
    return 1;
  }

  // Handle expect header (rudimentary support)
  string_t *expectHeader = string_fromCopy("Expect");
  string_t *expects = http_getHeader(request, expectHeader);
  string_free(expectHeader);
  if (expects != 0) {
    log(LOG_DEBUG, "Got expect '%s'", string_getBuffer(expects));
    // TODO: Handle actual expects here
    connection_write(connection, "HTTP/1.1 100 Continue\r\n", 23);
  }

  // Read the body if one exists
  string_t *body = 0;
  string_t *contentLengthHeader = string_fromCopy("Content-Length");
  string_t *contentLengthString = http_getHeader(request, contentLengthHeader);
  string_free(contentLengthHeader);
  if (contentLengthString != 0) {
    log(LOG_DEBUG, "Content-Length: %s", string_getBuffer(contentLengthString));
    int contentLength = atoi(string_getBuffer(contentLengthString));
    log(LOG_DEBUG, "The request has a body size of %d bytes", contentLength);
    if (contentLength > REQUEST_MAX_BODY_SIZE) {
      log(LOG_WARNING, "The client wanted to write %d bytes which is above maximum %d", contentLength, REQUEST_MAX_BODY_SIZE);
      http_free(request);
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
  hash_table_setValue(environment, string_fromCopy("REMOTE_ADDR"), string_fromCopy(string_getBuffer(connection->sourceAddress)));
  hash_table_setValue(environment, string_fromCopy("REMOTE_PORT"), string_fromInt(connection->sourcePort));

  uint8_t method = http_getMethod(request);
  if (method == HTTP_METHOD_GET)
    hash_table_setValue(environment, string_fromCopy("REQUEST_METHOD"), string_fromCopy("GET"));
  else if (method == HTTP_METHOD_POST)
    hash_table_setValue(environment, string_fromCopy("REQUEST_METHOD"), string_fromCopy("POST"));

  string_t *cookie = http_getHeader(request, string_fromCopy("Cookie"));
  if (cookie != 0)
    hash_table_setValue(environment, string_fromCopy("HTTP_COOKIE"), cookie);

  string_t *referer = http_getHeader(request, string_fromCopy("Referer"));
  if (referer != 0)
    hash_table_setValue(environment, string_fromCopy("HTTP_REFERER"), referer);

  string_t *userAgent = http_getHeader(request, string_fromCopy("User-Agent"));
  if (userAgent != 0)
    hash_table_setValue(environment, string_fromCopy("HTTP_USER_AGENT"), userAgent);

  url_t *url = http_getUrl(request);
  string_t *domainName = url_getDomainName(url);
  if (domainName != 0) {
    hash_table_setValue(environment, string_fromCopy("HTTP_HOST"), domainName);
    hash_table_setValue(environment, string_fromCopy("SERVER_NAME"), string_fromCopy(string_getBuffer(domainName)));
  }

  // TODO: This does not seem to work. Due to how we don't read from the pipe dynamically?
  /*uint16_t port = url_getPort(url);
  if (port != 0)
    hash_table_setValue(environment, string_fromCopy("SERVER_PORT"), string_fromInt(port));*/

  // TODO: This should be realtive to the document root
  /*string_t *path = url_getPath(url);
  if (path != 0)
    hash_table_setValue(environment, string_fromCopy("REQUEST_URI"), path);*/

  http_t *response = http_create();

  page_t *page = page_create501();
  string_t *source = page_getSource(page);
  http_setBody(response, source);
  http_setResponseCode(response, 501);
  http_setVersion(response, string_fromCopy("1.1"));

  string_t *responseString = http_toResponseString(response);
  connection_write(connection, string_getBuffer(responseString), string_getSize(responseString));
  string_free(responseString);
  page_free(page);
  // Freeing the page also frees the source, which we gave to http.
  // Not having this line would cause a double free
  response->body = 0;
  http_free(response);
  http_free(request);

  return 0;
}

void worker_waitForExit(worker_t *worker) {
  pthread_join(worker->thread, NULL);
}

void worker_kill(worker_t *worker) {
  pthread_cancel(worker->thread);
}

void worker_free(worker_t *worker) {
  if (worker->connection != 0)
    connection_free(worker->connection);
  // Free the worker itself
  free(worker);
}
