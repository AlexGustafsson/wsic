#include <string.h>

#include "../logging/logging.h"

#include "config.h"

config_t *config_parse(char *configString) {
  char parseError[200];
  toml_table_t *toml = toml_parse(configString, parseError, sizeof(parseError));
  if (toml == 0) {
    log(LOG_ERROR, "Could not parse config file '%s'", parseError);
    return 0;
  }

  config_t *config = malloc(sizeof(config_t));
  config->serverConfigs = list_create();

  // Parse the server table if it exists
  toml_table_t *serverTable = toml_table_in(toml, "server");
  if (serverTable != 0) {
    // Parse the parallel mode string into the parallel mode enum
    const char *parallelModeString = config_parseString(serverTable, "parallelMode");
    enum parallelMode parallelMode = PARALLEL_MODE_UNKNOWN;
    if (parallelModeString != 0) {
      if (strcmp(parallelModeString, "fork") == 0)
        parallelMode = PARALLEL_MODE_FORK;
      else if (strcmp(parallelModeString, "pre-fork") == 0)
        parallelMode = PARALLEL_MODE_PRE_FORK;
      else
        log(LOG_WARNING, "Unknown parallel mode '%s'", parallelModeString);
    }

    free(parallelModeString);
    config->parallelMode = parallelMode;

    config->daemon = config_parseBool(serverTable, "daemon");
  }

  toml_table_t *serversTable = toml_table_in(toml, "servers");
  if (serversTable == 0) {
    log(LOG_WARNING, "Missing table 'servers' - no server will be enabled");
  } else {
    int serversTables = toml_table_ntab(serversTable);

    for (int i = 0; i < serversTables; i++) {
      const char *tableKey = toml_key_in(serversTable, i);
      toml_table_t *serverTable = toml_table_in(serversTable, tableKey);
      if (serverTable == 0) {
        log(LOG_ERROR, "Got a corrupt server table when parsing config");
      } else {
        server_config_t *serverConfig = config_parseServerTable(serverTable);
        list_addValue(config->serverConfigs, serverConfig);
      }
    }
  }

  toml_free(toml);

  return config;
}

server_config_t *config_parseServerTable(toml_table_t * serverTable) {
  server_config_t *config = malloc(sizeof(server_config_t));
  if (config == 0)
    return 0;

  char *name = toml_table_key(serverTable);
  size_t nameLength = strlen(name);
  char *nameCopy = malloc(sizeof(char) * (nameLength + 1));
  memcpy(nameCopy, name, nameLength);
  nameCopy[nameLength] = 0;

  config->name = nameCopy;
  config->domain = (char *)config_parseString(serverTable, "domain");
  config->rootDirectory = (char *)config_parseString(serverTable, "rootDirectory");
  config->logfile = (char *)config_parseString(serverTable, "logfile");
  config->enabled = config_parseBool(serverTable, "enabled");

  log(LOG_DEBUG, "> %d", toml_table_nkval(serverTable));
  if (toml_raw_in(serverTable, "port") != 0) {
    int64_t port = config_parseInt(serverTable, "port");
    if (port > 1<<16)
      log(LOG_ERROR, "The port in config for server '%s' was too large", config->name);
    else
      config->port = port;
  }

  return config;
}

const char *config_parseString(toml_table_t *table, const char *key) {
  if (table == 0)
    return 0;

  const char *rawValue = toml_raw_in(table, key);
  // The value is missing
  if (rawValue == 0)
    return 0;

  char *value;
  // The value is not a string
  if (toml_rtos(rawValue, &value)) {
    log(LOG_ERROR, "Bad string in config: table '%s', key '%s'", toml_table_key(table), key);
    return 0;
  }

  return value;
}

int64_t config_parseInt(toml_table_t *table, const char *key) {
  const char *rawValue = toml_raw_in(table, key);
  // The value is missing
  if (rawValue == 0)
    return 0;

  int64_t value = 0;
  if (toml_rtoi(rawValue, &value)) {
    log(LOG_ERROR, "Bad integer in config: table '%s', key '%s'", toml_table_key(table), key);
    return 0;
  }

  return value;
}

int config_parseBool(toml_table_t *table, const char *key) {
  const char *rawValue = toml_raw_in(table, key);
  // The value is missing
  if (rawValue == 0)
    return -1;

  int value = 0;
  if (toml_rtob(rawValue, &value)) {
    log(LOG_ERROR, "Bad bool in config: table '%s', key '%s'", toml_table_key(table), key);
    return -1;
  }

  return value;
}

enum parallelMode config_getParallelMode(config_t *config) {
  return config->parallelMode;
}

void config_setParallelMode(config_t *config, enum parallelMode parallelMode) {
  config->parallelMode = parallelMode;
}

int16_t config_getIsDeamon(config_t *config) {
  return config->daemon;
}

server_config_t *config_getServerConfig(config_t *config, size_t index) {
  return list_getValue(config->serverConfigs, index);
}

const char *config_getName(server_config_t *config) {
  return config->name;
}

void config_setName(server_config_t *config, const char *name) {
  // TODO: Fix after string implementation
}

const char *config_getDomain(server_config_t *config) {
  return config->domain;
}

void config_setDomain(server_config_t *config, const char *domain) {
  // TODO: Fix after string implementation
}

const char *config_getRootDirectory(server_config_t *config) {
  return config->rootDirectory;
}

void config_setRootDirectory(server_config_t *config, const char *rootDirectory) {
  // TODO: Fix after string implementation
}

int16_t config_getPort(server_config_t *config) {
  return config->port;
}

void config_setPort(server_config_t *config, int16_t port) {
  // TODO: Fix after string implementation
}

const char *config_getLogfile(server_config_t *config) {
  return config->logfile;
}

void config_setLogfile(server_config_t *config, const char *logfile) {
  // TODO: Fix after string implementation
}

void config_free(config_t *config) {
  for (size_t i = 0; i < config->serverConfigs->length; i++) {
    server_config_t *serverConfig = list_removeValue(config->serverConfigs, i);
    free(serverConfig->domain);
    free(serverConfig->logfile);
    free(serverConfig->name);
    free(serverConfig->rootDirectory);
    free(serverConfig);
  }

  list_free(config->serverConfigs);
  free(config);
}
