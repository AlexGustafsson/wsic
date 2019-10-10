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

config_t *config_parse(const char *configString) __attribute__((nonnull(1)));

// Get and set the globally defined config
config_t *config_getGlobalConfig();
void config_setGlobalConfig(config_t *config) __attribute__((nonnull(1)));
void config_freeGlobalConfig();

server_config_t *config_parseServerTable(const toml_table_t *serverTable) __attribute__((nonnull(1)));

int8_t config_getIsDaemon(const config_t *config) __attribute__((nonnull(1)));
void config_setIsDaemon(config_t *config, int8_t isDaemon) __attribute__((nonnull(1)));

uint8_t config_getLoggingLevel(const config_t *config) __attribute__((nonnull(1)));
void config_setLoggingLevel(config_t *config, uint8_t loggingLevel) __attribute__((nonnull(1)));

server_config_t *config_getServerConfig(const config_t *config, size_t index) __attribute__((nonnull(1)));
server_config_t *config_getServerConfigByHTTPSDomain(const config_t *config, const string_t *domain) __attribute__((nonnull(1, 2)));
server_config_t *config_getServerConfigByHTTPDomain(const config_t *config, const string_t *domain) __attribute__((nonnull(1, 2)));
server_config_t *config_getServerConfigByDomain(const config_t *config, const string_t *domain, uint16_t port) __attribute__((nonnull(1, 2)));
server_config_t *config_getServerConfigBySSLContext(const config_t *config, const SSL_CTX *sslContext) __attribute__((nonnull(1, 2)));
size_t config_getServers(const config_t *config) __attribute__((nonnull(1)));

size_t config_getNumberOfThreads(const config_t *config) __attribute__((nonnull(1)));

size_t config_getBacklogSize(const config_t *config) __attribute__((nonnull(1)));

string_t *config_getName(const server_config_t *config) __attribute__((nonnull(1)));

string_t *config_getDomain(const server_config_t *config) __attribute__((nonnull(1)));

string_t *config_getRootDirectory(const server_config_t *config) __attribute__((nonnull(1)));

int16_t config_getPort(const server_config_t *config) __attribute__((nonnull(1)));
void config_setPort(server_config_t *config, int16_t port) __attribute__((nonnull(1)));

string_t *config_getLogfile(const config_t *config) __attribute__((nonnull(1)));
// Config owns logfile
void config_setLogfile(config_t *config, string_t *logfile) __attribute__((nonnull(1)));

SSL_CTX *config_getSSLContext(const server_config_t *config) __attribute__((nonnull(1)));
DH *config_getDiffieHellmanParameters(const server_config_t *config) __attribute__((nonnull(1)));

list_t *config_getDirectoryIndex(const server_config_t *config) __attribute__((nonnull(1)));

string_t *config_parseString(const toml_table_t *table, const char *key) __attribute__((nonnull(1, 2)));
int64_t config_parseInt(const toml_table_t *table, const char *key) __attribute__((nonnull(1, 2)));
int8_t config_parseBool(const toml_table_t *table, const char *key) __attribute__((nonnull(1, 2)));
list_t *config_parseArray(const toml_table_t *table, const char *key) __attribute__((nonnull(1, 2)));

// NOTE: Called by config_free automatically
void config_freeServerConfig(server_config_t *serverConfig) __attribute__((nonnull(1)));
void config_free(config_t *config) __attribute__((nonnull(1)));

#endif
