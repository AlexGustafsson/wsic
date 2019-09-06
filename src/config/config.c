#include <string.h>

#include "../includes/tomlc99/toml.h"

#include "../logging/logging.h"

#include "config.h"

config_t *parseConfig(const char *filename) {
  FILE *file = fopen(filename, "r");
  if (file == 0) {
    log(LOG_ERROR, "Could not read file %s", filename);
    return 0;
  }

  char parseError[200];
  toml_table_t *toml = toml_parse_file(file, parseError, sizeof(parseError));
  fclose(file);
  if (toml == 0) {
    log(LOG_ERROR, "Could not parse config file: %s", parseError);
    return 0;
  }

  config_t *config = malloc(sizeof(config_t));

  toml_table_t *serverBlock = toml_table_in(toml, "server");
  if (serverBlock == 0) {
    log(LOG_WARNING, "Could not parse config - missing server block");
  } else {
    const char *rawPort = toml_raw_in(serverBlock, "port");
    if (rawPort != 0) {
      int64_t port = 0;
      if (toml_rtoi(rawPort, &port)) {
        log(LOG_ERROR, "Bad value for config server:port");
      } else if (port > 2 << 16) {
        log(LOG_ERROR, "Bad value for config server:port - value to big");
      } else {
        config->port = (uint16_t)port;
      }
    }

    const char *rawParallelMode = toml_raw_in(serverBlock, "parallelMode");
    if (rawParallelMode != 0) {
      char *parallelMode;
      if (toml_rtos(rawParallelMode, &parallelMode)) {
        log(LOG_ERROR, "Bad value for config server:parallelMode");
      } else {
        config->parallelMode = parallelMode;
      }
    }
  }

  toml_table_t *webBlock = toml_table_in(toml, "web");
  if (webBlock == 0) {
    log(LOG_WARNING, "Could not parse config - missing web block");
  } else {
    const char *rawRootDirectory = toml_raw_in(webBlock, "rootDirectory");
    if (rawRootDirectory != 0) {
      char *rootDirectory;
      if (toml_rtos(rawRootDirectory, &rootDirectory)) {
        log(LOG_ERROR, "Bad value for config web:rootDirectory");
      } else {
        config->rootDirectory = rootDirectory;
      }
    }
  }

  toml_free(toml);

  return config;
}

void freeConfig(config_t *config) {
  if (config->rootDirectory != 0)
    free((void *)config->rootDirectory);

  if (config->parallelMode != 0)
    free((void *)config->parallelMode);

  free(config);
}
