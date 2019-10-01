#include <stdbool.h>
#include <string.h>

#include "../logging/logging.h"

#include "config.h"

static config_t *config_globalConfig = 0;

config_t *config_getGlobalConfig() {
  return config_globalConfig;
}

void config_setGlobalConfig(config_t *config) {
  config_globalConfig = config;
}

void config_freeGlobalConfig() {
  if (config_globalConfig != 0)
    config_free(config_globalConfig);
}

config_t *config_parse(char *configString) {
  char parseError[200];
  toml_table_t *toml = toml_parse(configString, parseError, sizeof(parseError));
  if (toml == 0) {
    log(LOG_ERROR, "Could not parse config file '%s'", parseError);
    return 0;
  }

  config_t *config = malloc(sizeof(config_t));
  if (config == 0)
    return 0;
  memset(config, 0, sizeof(config_t));
  config->serverConfigs = list_create();

  // Parse the server table if it exists
  toml_table_t *serverTable = toml_table_in(toml, "server");
  if (serverTable != 0)
    config->daemon = config_parseBool(serverTable, "daemon");

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
        if (serverConfig == 0) {
          log(LOG_ERROR, "Could not parse server config %d", i + 1);
          continue;
        }

        bool duplicate = false;
        for (int j = 0; j < serversTables; j++) {
          if (j == i)
            continue;

          server_config_t *otherConfig = list_getValue(config->serverConfigs, j);
          if (otherConfig == 0)
            continue;

          bool isSameDomain = string_equals(serverConfig->domain, otherConfig->domain);
          bool bothHaveSSL = serverConfig->sslContext != 0 && otherConfig->sslContext != 0;
          if (isSameDomain && bothHaveSSL) {
            log(LOG_ERROR, "Multiple server configuration listening to domain '%s'. Configuration '%s' will be disabled", string_getBuffer(serverConfig->domain), string_getBuffer(serverConfig->name));
            duplicate = true;
            break;
          }
        }

        if (!duplicate)
          list_addValue(config->serverConfigs, serverConfig);
      }
    }
  }

  toml_free(toml);

  return config;
}

server_config_t *config_parseServerTable(toml_table_t *serverTable) {
  server_config_t *config = malloc(sizeof(server_config_t));
  if (config == 0)
    return 0;
  memset(config, 0, sizeof(server_config_t));

  const char *rawName = toml_table_key(serverTable);
  string_t *name = string_fromCopy(rawName);

  config->name = name;
  config->domain = config_parseString(serverTable, "domain");
  config->rootDirectory = config_parseString(serverTable, "rootDirectory");
  config->logfile = config_parseString(serverTable, "logfile");
  config->enabled = config_parseBool(serverTable, "enabled");

  if (toml_raw_in(serverTable, "port") != 0) {
    int64_t port = config_parseInt(serverTable, "port");
    if (port < 0 || port > 1 << 16)
      log(LOG_ERROR, "The port in config for server '%s' was unexpected", string_getBuffer(config->name));
    else
      config->port = port;
  }

  string_t *privateKey = config_parseString(serverTable, "privateKey");
  string_t *certificate = config_parseString(serverTable, "certificate");
  if ((privateKey == 0 && certificate != 0) || (privateKey != 0 && certificate == 0)) {
    log(LOG_ERROR, "Both 'privateKey' and 'certificate' must be set in order to use TLS");
    return 0;
  }

  if (privateKey != 0 && certificate != 0) {
    log(LOG_DEBUG, "Setting up TLS configuration");

    // Setup TLS
    const SSL_METHOD *method = TLS_method();
    config->sslContext = SSL_CTX_new(method);
    if (!config->sslContext) {
      log(LOG_ERROR, "Unable to create TLS context");
      return 0;
    }

    // Only allow TLS 1.2 and above (TLS 1.3)
    SSL_CTX_set_min_proto_version(config->sslContext, TLS1_2_VERSION);

    string_t *ellipticCurves = config_parseString(serverTable, "ellipticCurves");
    if (ellipticCurves == 0) {
      SSL_CTX_set1_curves_list(config->sslContext, CONFIG_TLS_DEFAULT_ELLIPTIC_CURVES);
    } else {
      SSL_CTX_set1_curves_list(config->sslContext, string_getBuffer(ellipticCurves));
      string_free(ellipticCurves);
    }

    // Disable server certificate validation if validateCertificate is false
    int8_t verifyCertificates = config_parseBool(serverTable, "validateCertificate");
    if (verifyCertificates == 0)
      ;
    SSL_CTX_set_verify(config->sslContext, SSL_VERIFY_NONE, NULL);

    // Configure the TLS cipher suite
    string_t *cipherSuite = config_parseString(serverTable, "cipherSuite");
    if (cipherSuite == 0) {
      // Set the cipher suite to use for TLS 1.2
      SSL_CTX_set_cipher_list(config->sslContext, CONFIG_TLS_DEFAULT_TLS_1_2_CIPHER_SUITE);
      // Set the cipher suite to use for TLS 1.3
      SSL_CTX_set_ciphersuites(config->sslContext, CONFIG_TLS_DEFAULT_TLS_1_3_CIPHER_SUITE);
    } else {
      // Set the cipher suite to use for TLS 1.2
      SSL_CTX_set_cipher_list(config->sslContext, string_getBuffer(cipherSuite));
      string_free(cipherSuite);
    }

    // Setup Diffie Hellman (DH) parameters from a file
    // Use openssl dhparam -out dh_param_1024.pem -2 1024 or the like
    string_t *dhparams = config_parseString(serverTable, "dhparams");
    if (dhparams != 0) {
      FILE *file = fopen(string_getBuffer(dhparams), "r");
      if (file) {
        config->dhparams = PEM_read_DHparams(file, NULL, NULL, NULL);
        fclose(file);
      } else {
        log(LOG_ERROR, "Unable to read Diffie Hellman parameters file '%s'", string_getBuffer(dhparams));
        return 0;
      }
      string_free(dhparams);
    }

    if (SSL_CTX_use_PrivateKey_file(config->sslContext, string_getBuffer(privateKey), SSL_FILETYPE_PEM) <= 0) {
      log(LOG_ERROR, "Unable to create TLS context - could not read server private key");
      return 0;
    }

    if (SSL_CTX_use_certificate_file(config->sslContext, string_getBuffer(certificate), SSL_FILETYPE_PEM) <= 0) {
      log(LOG_ERROR, "Unable to create TLS context - could not read server certificate");
      return 0;
    }
  }

  return config;
}

string_t *config_parseString(toml_table_t *table, const char *key) {
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

  string_t *string = string_fromCopy(value);
  free(value);

  // Could not allocate string
  if (string == 0)
    return 0;

  return string;
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

int16_t config_getIsDaemon(config_t *config) {
  return config->daemon;
}

void config_setIsDaemon(config_t *config, int16_t isDaemon) {
  config->daemon = isDaemon;
}

server_config_t *config_getServerConfig(config_t *config, size_t index) {
  return list_getValue(config->serverConfigs, index);
}

server_config_t *config_getServerConfigBySNI(config_t *config, string_t *domain) {
  size_t servers = list_getLength(config->serverConfigs);
  for (size_t i = 0; i < servers; i++) {
    server_config_t *serverConfig = config_getServerConfig(config, i);
    bool isSameDomain = string_equals(config_getDomain(serverConfig), domain);
    bool hasSSL = serverConfig->sslContext != 0;
    if (isSameDomain && hasSSL)
      return serverConfig;
  }

  return 0;
}

server_config_t *config_getServerConfigBySSLContext(config_t *config, SSL_CTX *sslContext) {
  size_t servers = list_getLength(config->serverConfigs);
  for (size_t i = 0; i < servers; i++) {
    server_config_t *serverConfig = config_getServerConfig(config, i);
    SSL_CTX *context = config_getSSLContext(serverConfig);
    if (sslContext == context)
      return serverConfig;
  }

  return 0;
}

size_t config_getServers(config_t *config) {
  return list_getLength(config->serverConfigs);
}

string_t *config_getName(server_config_t *config) {
  return config->name;
}

void config_setName(server_config_t *config, string_t *name) {
  // TODO: Fix after string implementation
}

string_t *config_getDomain(server_config_t *config) {
  return config->domain;
}

void config_setDomain(server_config_t *config, string_t *domain) {
  // TODO: Fix after string implementation
}

string_t *config_getRootDirectory(server_config_t *config) {
  return config->rootDirectory;
}

void config_setRootDirectory(server_config_t *config, string_t *rootDirectory) {
  // TODO: Fix after string implementation
}

int16_t config_getPort(server_config_t *config) {
  return config->port;
}

void config_setPort(server_config_t *config, int16_t port) {
  config->port = port;
}

string_t *config_getLogfile(server_config_t *config) {
  return config->logfile;
}

void config_setLogfile(server_config_t *config, string_t *logfile) {
  config->logfile = logfile;
}

SSL_CTX *config_getSSLContext(server_config_t *config) {
  return config->sslContext;
}

DH *config_getDiffieHellmanParameters(server_config_t *config) {
  return config->dhparams;
}

void config_free(config_t *config) {
  for (size_t i = 0; i < config->serverConfigs->length; i++) {
    server_config_t *serverConfig = list_removeValue(config->serverConfigs, i);
    string_free(serverConfig->domain);
    string_free(serverConfig->logfile);
    string_free(serverConfig->name);
    string_free(serverConfig->rootDirectory);
    free(serverConfig);
  }

  list_free(config->serverConfigs);
  free(config);
}
