#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

#include <openssl/ssl.h>

#include "tomlc99/toml.h"

#include "../datastructures/list/list.h"
#include "../string/string.h"

enum parallelMode { PARALLEL_MODE_FORK,
                    PARALLEL_MODE_PRE_FORK,
                    PARALLEL_MODE_UNKNOWN };

typedef struct {
  string_t *name;
  string_t *domain;
  string_t *rootDirectory;
  // -1 if not set, 0 - 65536 otherwise
  int16_t port;
  string_t *logfile;
  // -1 if not set, 0 or 1 otherwise
  int8_t enabled;
  SSL_CTX *sslContext;
} server_config_t;

typedef struct {
  enum parallelMode parallelMode;
  // -1 if not set, 0 or 1 otherwise
  int8_t daemon;
  list_t *serverConfigs;
} config_t;

config_t *config_parse(char *configString);

// Get and set the globally defined config
config_t *config_getGlobalConfig();
void config_setGlobalConfig(config_t *config);
void config_freeGlobalConfig();

server_config_t *config_parseServerTable(toml_table_t *serverTable);

enum parallelMode config_getParallelMode(config_t *config);
void config_setParallelMode(config_t *config, enum parallelMode parallelMode);

int16_t config_getIsDaemon(config_t *config);
void config_setIsDaemon(config_t *config, int16_t isDaemon);

server_config_t *config_getServerConfig(config_t *config, size_t index);
server_config_t *config_getServerConfigBySNI(config_t *config, string_t *domain);
size_t config_getServers(config_t *config);

string_t *config_getName(server_config_t *config);
void config_setName(server_config_t *config, string_t *name);

string_t *config_getDomain(server_config_t *config);
void config_setDomain(server_config_t *config, string_t *domain);

string_t *config_getRootDirectory(server_config_t *config);
void config_setRootDirectory(server_config_t *config, string_t *rootDirectory);

int16_t config_getPort(server_config_t *config);
void config_setPort(server_config_t *config, int16_t port);

string_t *config_getLogfile(server_config_t *config);
void config_setLogfile(server_config_t *config, string_t *logfile);

SSL_CTX *config_getSSLContext(server_config_t *config);

string_t *config_parseString(toml_table_t *table, const char *key);
int64_t config_parseInt(toml_table_t *table, const char *key);
int config_parseBool(toml_table_t *table, const char *key);

void config_free(config_t *config);

#endif
