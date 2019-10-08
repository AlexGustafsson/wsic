#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

#include <openssl/ssl.h>

#include "tomlc99/toml.h"

#include "../datastructures/list/list.h"
#include "../string/string.h"

// See:
// https://wiki.mozilla.org/Security/Server_Side_TLS
// https://www.openssl.org/docs/man1.1.1/man1/ciphers.html
#define CONFIG_TLS_DEFAULT_ELLIPTIC_CURVES "P-256:P-384:X25519"
#define CONFIG_TLS_DEFAULT_TLS_1_2_CIPHER_SUITE "ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-CHACHA20-POLY1305:ECDHE-RSA-CHACHA20-POLY1305:DHE-RSA-AES128-GCM-SHA256:DHE-RSA-AES256-GCM-SHA384"
#define CONFIG_TLS_DEFAULT_TLS_1_3_CIPHER_SUITE "TLS_AES_128_GCM_SHA256:TLS_AES_256_GCM_SHA384:TLS_CHACHA20_POLY1305_SHA256"

typedef struct {
  string_t *name;
  string_t *domain;
  string_t *rootDirectory;
  // -1 if not set, 0 - 65536 otherwise
  int16_t port;
  // -1 if not set, 0 or 1 otherwise
  int8_t enabled;
  DH *dhparams;
  SSL_CTX *sslContext;
  list_t *directoryIndex;
} server_config_t;

typedef struct {
  // -1 if not set, 0 or 1 otherwise
  int8_t daemon;
  list_t *serverConfigs;
  string_t *logfile;
  uint8_t loggingLevel;
  size_t threads;
  size_t backlog;
} config_t;

config_t *config_parse(const char *configString);

// Get and set the globally defined config
config_t *config_getGlobalConfig();
void config_setGlobalConfig(config_t *config);
void config_freeGlobalConfig();

server_config_t *config_parseServerTable(toml_table_t *serverTable);

int8_t config_getIsDaemon(config_t *config);
void config_setIsDaemon(config_t *config, int8_t isDaemon);

uint8_t config_getLoggingLevel(config_t *config);
void config_setLoggingLevel(config_t *config, uint8_t loggingLevel);

server_config_t *config_getServerConfig(config_t *config, size_t index);
server_config_t *config_getServerConfigBySNI(config_t *config, string_t *domain);
server_config_t *config_getServerConfigByDomain(config_t *config, string_t *domain, uint16_t port);
server_config_t *config_getServerConfigBySSLContext(config_t *config, SSL_CTX *sslContext);
size_t config_getServers(config_t *config);

size_t config_getNumberOfThreads(config_t *config);

size_t config_getBacklogSize(config_t *config);

string_t *config_getName(server_config_t *config);

string_t *config_getDomain(server_config_t *config);

string_t *config_getRootDirectory(server_config_t *config);

int16_t config_getPort(server_config_t *config);
void config_setPort(server_config_t *config, int16_t port);

string_t *config_getLogfile(config_t *config);
void config_setLogfile(config_t *config, string_t *logfile);

SSL_CTX *config_getSSLContext(server_config_t *config);
DH *config_getDiffieHellmanParameters(server_config_t *config);

list_t *config_getDirectoryIndex(server_config_t *config);

string_t *config_parseString(toml_table_t *table, const char *key);
int64_t config_parseInt(toml_table_t *table, const char *key);
int8_t config_parseBool(toml_table_t *table, const char *key);
list_t *config_parseArray(toml_table_t *table, const char *key);

// NOTE: Called by config_free automatically
void config_freeServerConfig(server_config_t *serverConfig);
void config_free(config_t *config);

#endif
