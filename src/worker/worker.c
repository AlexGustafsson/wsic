#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "../config/config.h"
#include "../http/http.h"
#include "../logging/logging.h"
#include "../path/path.h"
#include "../resources/resources.h"
#include "../string/string.h"
#include "../www/www.h"

#include "worker.h"

// Private methods
// The main entry point of a worker
int worker_entryPoint();
int worker_handleConnection(worker_t *worker, connection_t *connection);
hash_table_t *worker_createEnvironment(connection_t *connection, http_t *request);
size_t worker_return500(connection_t *connection, http_t *request, string_t *description);
size_t worker_return404(connection_t *connection, http_t *request, string_t *path);
size_t worker_return400(connection_t *connection, http_t *request, string_t *path, string_t *description);
size_t worker_return413(connection_t *connection, http_t *request, string_t *path);
size_t worker_return200(connection_t *connection, http_t *request, string_t *resolvedPath);
size_t worker_returnCGI(worker_t *worker, connection_t *connection, http_t *request, string_t *resolvedPath, string_t *body);

worker_t *worker_spawn(int id, connection_t *connection, message_queue_t *queue) {
  worker_t *worker = malloc(sizeof(worker_t));
  if (worker == 0)
    return 0;
  memset(worker, 0, sizeof(worker_t));

  worker->id = id;
  worker->connection = connection;
  worker->queue = queue;
  worker->shouldRun = true;

  pthread_t thread;

  if (pthread_create(&thread, NULL, (void *(*)(void *))worker_entryPoint, worker) != 0) {
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
    int exitCode = worker_handleConnection(worker, worker->connection);
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

    // Lock, waiting for a connection
    worker->connection = message_queue_pop(worker->queue);

    // Exit if the worker was unlocked and should no longer run
    if (!worker->shouldRun) {
      log(LOG_DEBUG, "Suspending worker %d", worker->id);
      pthread_exit(0);
    }

    // Go back to sleep if the worker was unlocked without receiving a connection
    if (worker->connection == 0) {
      log(LOG_DEBUG, "Worker %d was woken up without receiving a connection, going back to sleep", worker->id);
      continue;
    }

    log(LOG_DEBUG, "Worker process interrupted by parent to handle a connection (worker %d)", worker->id);

    worker->status = WORKER_STATUS_WORKING;

    worker_handleConnection(worker, worker->connection);
    log(LOG_DEBUG, "Handled connection - closing it");
    // Free the connection as it's of no further use
    connection_free(worker->connection);
    worker->connection = 0;
  }

  return 0;
}

int worker_handleConnection(worker_t *worker, connection_t *connection) {
  http_t *request = http_create();
  if (request == 0)
    return 1;

  // Start reading the header from the client
  string_t *currentLine = 0;
  size_t line = 0;
  size_t headerSize = 0;
  while (true) {
    currentLine = connection_readLine(connection, REQUEST_READ_TIMEOUT, REQUEST_MAX_HEADER_SIZE - headerSize);
    if (currentLine != 0)
      log(LOG_DEBUG, "Got line %s", string_getBuffer(currentLine));
    // Stop if there was no line read or the line was empty (all headers were read)
    if (currentLine == 0)
      break;
    if (string_getSize(currentLine) == 0) {
      string_free(currentLine);
      break;
    }

    headerSize += string_getSize(currentLine);

    if (line == 0) {
      // Parse the request line
      bool parsed = http_parseRequestLine(request, currentLine);
      if (!parsed) {
        log(LOG_ERROR, "Failed to parse request line '%s'. Closing connection", string_getBuffer(currentLine));
        worker_return400(connection, request, 0, string_fromCopy("Bad header received"));
        string_free(currentLine);
        http_free(request);
        return 0;
      }
    } else {
      // Parse the header
      bool parsed = http_parseHeader(request, currentLine);
      if (!parsed) {
        log(LOG_ERROR, "Failed to parse header '%s'. Closing connection", string_getBuffer(currentLine));
        worker_return400(connection, request, 0, string_fromCopy("Bad header received"));
        string_free(currentLine);
        http_free(request);
        return 0;
      }
    }

    string_free(currentLine);
    currentLine = 0;
    line++;
  }

  bool parsedHost = http_parseHost(request);
  if (!parsedHost) {
    log(LOG_DEBUG, "Could not parse Host header");
    worker_return400(connection, request, 0, string_fromCopy("Bad header received"));
    http_free(request);
    return 0;
  }

  if (request->url == 0) {
    log(LOG_ERROR, "Didn't receive a header from the request");
    worker_return400(connection, request, 0, string_fromCopy("No header received"));
    http_free(request);
    return 0;
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

  string_t *domainName = url_getDomainName(http_getUrl(request));
  uint16_t port = url_getPort(http_getUrl(request));

  // Resolve server config - 500 if config not found
  config_t *config = config_getGlobalConfig();
  server_config_t *serverConfig = config_getServerConfigByDomain(config, domainName, port);
  if (serverConfig == 0) {
    log(LOG_ERROR, "Got request to serve an unknown domain '%s'", string_getBuffer(domainName));
    worker_return500(connection, request, string_fromCopy("The requested domain is not served by this server"));
    http_free(request);
    return 0;
  }

  string_t *path = url_getPath(http_getUrl(request));

  // Resolve path - 404 if not found or failed
  string_t *rootDirectory = config_getRootDirectory(serverConfig);
  string_t *resolvedPath = path_resolve(path, rootDirectory);
  if (resolvedPath == 0) {
    worker_return404(connection, request, path);
    http_free(request);
    return 0;
  }

  log(LOG_DEBUG, "Got request for file '%s'", string_getBuffer(resolvedPath));

  bool isFile = resources_isFile(resolvedPath);
  bool isExecutable = resources_isExecutable(resolvedPath);

  // If the request is to a directory that exists, try to handle directory index
  if (!isFile) {
    list_t *directoryIndex = config_getDirectoryIndex(serverConfig);
    if (directoryIndex != 0) {
      for (size_t i = 0; i < list_getLength(directoryIndex); i++) {
        // Append current directory index to resolved path, resolve it again
        string_t *filePath = list_getValue(directoryIndex, i);
        string_t *indexPath = path_resolve(filePath, resolvedPath);

        // If the resolved directory index exists, handle it as a regular file
        if (indexPath != 0) {
          if (resolvedPath != 0)
            string_free(resolvedPath);
          resolvedPath = indexPath;
          isFile = resources_isFile(resolvedPath);
          isExecutable = resources_isExecutable(resolvedPath);
          break;
        }
      }
    }
  }

  // CGI can handle any method
  if (isFile && isExecutable) {
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
        worker_return413(connection, request, 0);
        http_free(request);
        return 0;
      } else if (contentLength == 0) {
        log(LOG_WARNING, "Got empty body");
        worker_return400(connection, request, 0, string_fromCopy("Empty body when headers specified content length"));
        http_free(request);
        return 0;
      } else {
        body = connection_read(connection, REQUEST_READ_TIMEOUT, contentLength);
        if (body == 0) {
          log(LOG_ERROR, "Reading body timed out or failed");
          worker_return400(connection, request, 0, string_fromCopy("Request timed out"));
          http_free(request);
          return 0;
        }
      }
    }

    // The file exists, is a regular file and executable - run it
    worker_returnCGI(worker, connection, request, resolvedPath, body);
    if (body != 0)
      string_free(body);
  } else {
    if (request->method == HTTP_METHOD_GET) {
      if (isFile) {
        // The file exists and is a regular file, serve it
        worker_return200(connection, request, resolvedPath);
      } else {
        // The file exists but is not a regular file - 404 as per
        // https://en.wikipedia.org/wiki/Webserver_directory_index
        worker_return404(connection, request, path);
      }
    } else {
      // Only GET works for files (right now)
      worker_return400(connection, request, path, string_fromCopy("Method not supported"));
    }
  }

  http_free(request);
  string_free(resolvedPath);
  return 0;
}

hash_table_t *worker_createEnvironment(connection_t *connection, http_t *request) {
  hash_table_t *environment = hash_table_create();
  hash_table_setValue(environment, string_fromCopy("HTTPS"), string_fromCopy("off"));
  hash_table_setValue(environment, string_fromCopy("SERVER_SOFTWARE"), string_fromCopy("WSIC"));
  if (connection->sourceAddress != 0)
    hash_table_setValue(environment, string_fromCopy("REMOTE_ADDR"), string_copy(connection->sourceAddress));
  hash_table_setValue(environment, string_fromCopy("REMOTE_PORT"), string_fromInt(connection->sourcePort));

  uint8_t method = http_getMethod(request);
  if (method == HTTP_METHOD_GET)
    hash_table_setValue(environment, string_fromCopy("REQUEST_METHOD"), string_fromCopy("GET"));
  else if (method == HTTP_METHOD_POST)
    hash_table_setValue(environment, string_fromCopy("REQUEST_METHOD"), string_fromCopy("POST"));

  string_t *cookieHeader = string_fromCopy("Cookie");
  string_t *cookie = http_getHeader(request, cookieHeader);
  string_free(cookieHeader);
  if (cookie != 0)
    hash_table_setValue(environment, string_fromCopy("HTTP_COOKIE"), string_copy(cookie));

  string_t *refererHeader = string_fromCopy("Referer");
  string_t *referer = http_getHeader(request, refererHeader);
  string_free(refererHeader);
  if (referer != 0)
    hash_table_setValue(environment, string_fromCopy("HTTP_REFERER"), string_copy(referer));

  string_t *userAgentHeader = string_fromCopy("User-Agent");
  string_t *userAgent = http_getHeader(request, userAgentHeader);
  string_free(userAgentHeader);
  if (userAgent != 0)
    hash_table_setValue(environment, string_fromCopy("HTTP_USER_AGENT"), string_copy(userAgent));

  url_t *url = http_getUrl(request);
  string_t *domainName = url_getDomainName(url);
  if (domainName != 0) {
    hash_table_setValue(environment, string_fromCopy("HTTP_HOST"), string_copy(domainName));
    hash_table_setValue(environment, string_fromCopy("SERVER_NAME"), string_copy(domainName));
  }

  uint16_t port = url_getPort(url);
  if (port != 0)
    hash_table_setValue(environment, string_fromCopy("SERVER_PORT"), string_fromInt(port));

  string_t *path = url_getPath(url);
  if (path != 0)
    hash_table_setValue(environment, string_fromCopy("REQUEST_URI"), string_copy(path));

  return environment;
}

size_t worker_return500(connection_t *connection, http_t *request, string_t *description) {
  http_t *response = http_create();
  page_t *page = page_create500(description);
  string_t *source = page_getSource(page);
  http_setBody(response, source);
  http_setResponseCode(response, 500);
  http_setVersion(response, string_fromCopy("1.1"));

  string_t *responseString = http_toResponseString(response);
  size_t bytesWritten = connection_write(connection, string_getBuffer(responseString), string_getSize(responseString));
  string_t *path = url_getPath(http_getUrl(request));
  logging_request(connection_getSourceAddress(connection), http_getMethod(request), path, http_getVersion(request), 500, bytesWritten);
  string_free(responseString);
  page_free(page);
  // Freeing the page also frees the source, which we gave to http.
  // Not having this line would cause a double free
  response->body = 0;
  http_free(response);

  return bytesWritten;
}

size_t worker_return404(connection_t *connection, http_t *request, string_t *path) {
  http_t *response = http_create();
  // Copy the path since we want to keep ownership
  page_t *page = page_create404(string_copy(path));
  string_t *source = page_getSource(page);
  http_setBody(response, source);
  http_setResponseCode(response, 404);
  http_setVersion(response, string_fromCopy("1.1"));

  string_t *responseString = http_toResponseString(response);
  size_t bytesWritten = connection_write(connection, string_getBuffer(responseString), string_getSize(responseString));
  logging_request(connection_getSourceAddress(connection), http_getMethod(request), path, http_getVersion(request), 404, bytesWritten);
  string_free(responseString);
  page_free(page);
  // Freeing the page also frees the source, which we gave to http.
  // Not having this line would cause a double free
  response->body = 0;
  http_free(response);

  return bytesWritten;
}

size_t worker_return400(connection_t *connection, http_t *request, string_t *path, string_t *description) {
  http_t *response = http_create();
  // Copy the path since we want to keep ownership
  page_t *page = page_create400(string_copy(description));
  string_t *source = page_getSource(page);
  http_setBody(response, source);
  http_setResponseCode(response, 400);
  http_setVersion(response, string_fromCopy("1.1"));

  string_t *responseString = http_toResponseString(response);
  size_t bytesWritten = connection_write(connection, string_getBuffer(responseString), string_getSize(responseString));
  logging_request(connection_getSourceAddress(connection), http_getMethod(request), path, http_getVersion(request), 400, bytesWritten);
  string_free(responseString);
  page_free(page);
  // Freeing the page also frees the source, which we gave to http.
  // Not having this line would cause a double free
  response->body = 0;
  http_free(response);

  return bytesWritten;
}

size_t worker_return413(connection_t *connection, http_t *request, string_t *path) {
  http_t *response = http_create();
  // Copy the path since we want to keep ownership
  page_t *page = page_create413();
  string_t *source = page_getSource(page);
  http_setBody(response, source);
  http_setResponseCode(response, 413);
  http_setVersion(response, string_fromCopy("1.1"));

  string_t *responseString = http_toResponseString(response);
  size_t bytesWritten = connection_write(connection, string_getBuffer(responseString), string_getSize(responseString));
  logging_request(connection_getSourceAddress(connection), http_getMethod(request), path, http_getVersion(request), 413, bytesWritten);
  string_free(responseString);
  page_free(page);
  // Freeing the page also frees the source, which we gave to http.
  // Not having this line would cause a double free
  response->body = 0;
  http_free(response);

  return bytesWritten;
}

size_t worker_return200(connection_t *connection, http_t *request, string_t *resolvedPath) {
  http_t *response = http_create();
  http_setResponseCode(response, 200);
  http_setVersion(response, string_fromCopy("1.1"));

  string_t *fileContent = resources_loadFile(resolvedPath);
  if (fileContent == 0) {
    log(LOG_ERROR, "Could not read file '%s'", string_getBuffer(resolvedPath));
    http_free(response);
    worker_return500(connection, request, string_fromCopy("Unable to access requested file"));
    return 0;
  }

  http_setBody(response, fileContent);
  string_t *mimeType = resources_getMIMEType(resolvedPath);
  if (mimeType != 0)
    log(LOG_DEBUG, "MIME type of '%s' is '%s'", string_getBuffer(resolvedPath), string_getBuffer(mimeType));
  // Default to text/plain if no type was found
  if (mimeType == 0)
    mimeType = string_fromCopy("text/plain");
  http_setHeader(response, string_fromCopy("Content-Type"), mimeType);

  string_t *responseString = http_toResponseString(response);
  size_t bytesWritten = connection_write(connection, string_getBuffer(responseString), string_getSize(responseString));
  string_t *path = url_getPath(http_getUrl(request));
  logging_request(connection_getSourceAddress(connection), http_getMethod(request), path, http_getVersion(request), 200, bytesWritten);
  string_free(responseString);
  http_free(response);

  return bytesWritten;
}

size_t worker_returnCGI(worker_t *worker, connection_t *connection, http_t *request, string_t *resolvedPath, string_t *body) {
  log(LOG_DEBUG, "Spawning CGI process");
  list_t *arguments = 0;
  hash_table_t *environment = worker_createEnvironment(connection, request);
  worker->cgi = cgi_spawn(string_getBuffer(resolvedPath), arguments, environment);
  log(LOG_DEBUG, "Spawned process with pid %d", worker->cgi->pid);

  while (hash_table_getLength(environment) > 0) {
    string_t *key = hash_table_getKeyByIndex(environment, 0);
    string_t *value = hash_table_removeValue(environment, key);
    log(LOG_DEBUG, "Freeing value '%s'", string_getBuffer(value));
    string_free(value);
  }
  hash_table_free(environment);

  // Write body to CGI
  if (body != 0) {
    log(LOG_DEBUG, "Writing request to CGI process");
    log(LOG_DEBUG, "Body content is:\n<%s> of size %zu", string_getBuffer(body), string_getSize(body));
    cgi_write(worker->cgi, string_getBuffer(body), string_getSize(body));
    // Make sure the process receives EOF
    cgi_flushStdin(worker->cgi);
  } else {
    // Close the input to the CGI process
    cgi_flushStdin(worker->cgi);
  }

  log(LOG_DEBUG, "Reading response from CGI process");
  // TODO: Read more than 4096 bytes
  char buffer[2048] = {0};
  cgi_read(worker->cgi, buffer, 2048);
  buffer[2048 - 1] = 0;

  log(LOG_DEBUG, "Got response from CGI process");
  size_t bytesWritten = connection_write(connection, buffer, 2048);
  string_t *path = url_getPath(http_getUrl(request));
  logging_request(connection_getSourceAddress(connection), http_getMethod(request), path, http_getVersion(request), 0, bytesWritten);

  // NOTE: Not necessary, but for debugging it's nice to know
  // that the process is actually exiting (not kept forever)
  // since we don't currently kill spawned processes
  log(LOG_DEBUG, "Waiting for process to exit");
  uint8_t exitCode = cgi_waitForExit(worker->cgi);
  log(LOG_DEBUG, "Process exited with status %d", exitCode);

  // Close and free up CGI process and connection
  cgi_freeProcess(worker->cgi);
  worker->cgi = 0;
  return bytesWritten;
}

void worker_waitForExit(worker_t *worker) {
  pthread_join(worker->thread, NULL);
}

void worker_kill(worker_t *worker) {
  worker->shouldRun = false;
  pthread_cancel(worker->thread);
}

void worker_closeGracefully(worker_t *worker) {
  worker->shouldRun = false;
}

void worker_free(worker_t *worker) {
  if (worker->connection != 0)
    connection_free(worker->connection);
  if (worker->cgi != 0)
    cgi_freeProcess(worker->cgi);
  // Free the worker itself
  free(worker);
}
