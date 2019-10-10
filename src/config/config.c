#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>

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

config_t *config_parse(const char *configString) {
  char parseError[200];
  toml_table_t *toml = toml_parse((char *)configString, parseError, sizeof(parseError));
  if (toml == 0) {
    log(LOG_ERROR, "Could not parse config file '%s'", parseError);
    return 0;
  }

  config_t *config = malloc(sizeof(config_t));
  if (config == 0)
    return 0;

  memset(config, 0, sizeof(config_t));

  config->serverConfigs = list_create();
  if (config->serverConfigs == 0) {
    log(LOG_ERROR, "Failed to create list for serverConfigs");
    free(config);
    return 0;
  }

  // Parse the server table if it exists
  toml_table_t *serverTable = toml_table_in(toml, "server");
  if (serverTable != 0) {
    config->daemon = config_parseBool(serverTable, "daemon");

    config->logfile = config_parseString(serverTable, "logfile");

    if (toml_raw_in(serverTable, "loggingLevel") != 0)
      config->loggingLevel = config_parseInt(serverTable, "loggingLevel");
    else
      config->loggingLevel = LOG_NOTICE;

    if (toml_raw_in(serverTable, "threads") != 0) {
      int64_t rawThreads = config_parseInt(serverTable, "threads");
      if (rawThreads <= 1) {
        log(LOG_WARNING, "Too few threads specified in server config - using default");
        config->threads = 32;
      } else {
        config->threads = rawThreads;
      }
    } else {
      config->threads = 32;
    }

    if (toml_raw_in(serverTable, "backlog") != 0) {
      int64_t rawBacklog = config_parseInt(serverTable, "backlog");
      if (rawBacklog <= 1) {
        log(LOG_WARNING, "Too small of a backlog specified in server config - using default");
        config->backlog = SOMAXCONN;
      } else if (rawBacklog > SOMAXCONN) {
        log(LOG_WARNING, "Too large of a backlog specified in server config - using default");
        config->backlog = SOMAXCONN;
      } else {
        config->backlog = rawBacklog;
      }
    } else {
      config->backlog = SOMAXCONN;
    }
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
            log(LOG_WARNING, "Multiple server configuration listening to domain '%s'. Configuration '%s' will be disabled", string_getBuffer(serverConfig->domain), string_getBuffer(serverConfig->name));
            duplicate = true;
            break;
          }
        }

        if (!duplicate)
          list_addValue(config->serverConfigs, serverConfig);
        else
          config_freeServerConfig(serverConfig);
      }
    }
  }

  toml_free(toml);

  return config;
}

server_config_t *config_parseServerTable(const toml_table_t *serverTable) {
  server_config_t *config = malloc(sizeof(server_config_t));
  if (config == 0)
    return 0;
  memset(config, 0, sizeof(server_config_t));

  const char *rawName = toml_table_key((toml_table_t *)serverTable);
  config->name = string_fromBuffer(rawName);
  config->domain = config_parseString(serverTable, "domain");
  // Parse and resolve the root directory
  string_t *relativePath = config_parseString(serverTable, "rootDirectory");
  if (relativePath != 0) {
    char *absolutePathBuffer = realpath(string_getBuffer(relativePath), NULL);
    if (absolutePathBuffer == 0) {
      log(LOG_ERROR, "Could not resolve root directory '%s'", string_getBuffer(relativePath));
      config_freeServerConfig(config);
      string_free(relativePath);
      return 0;
    }
    string_free(relativePath);
    config->rootDirectory = string_fromBuffer(absolutePathBuffer);
    free(absolutePathBuffer);
  }
  config->enabled = config_parseBool(serverTable, "enabled");

  if (toml_raw_in((toml_table_t *)serverTable, "port") != 0) {
    int64_t port = config_parseInt(serverTable, "port");
    if (port < 0 || port > 1 << 16) {
      log(LOG_ERROR, "The port in config for server '%s' was unexpected", string_getBuffer(config->name));
      config_freeServerConfig(config);
      return 0;
    }
    config->port = port;
  }

  config->directoryIndex = config_parseArray(serverTable, "directoryIndex");

  string_t *privateKey = config_parseString(serverTable, "privateKey");
  string_t *certificate = config_parseString(serverTable, "certificate");

  if ((privateKey == 0 && certificate != 0) || (privateKey != 0 && certificate == 0)) {
    log(LOG_ERROR, "Both 'privateKey' and 'certificate' must be set in order to use TLS");
    config_freeServerConfig(config);
    if (privateKey != 0)
      string_free(privateKey);
    if (certificate != 0)
      string_free(certificate);
    return 0;
  }

  if (privateKey != 0 && certificate != 0) {
    log(LOG_DEBUG, "Setting up TLS configuration");

    // Setup TLS
    const SSL_METHOD *method = TLS_method();
    config->sslContext = SSL_CTX_new(method);
    if (config->sslContext == 0) {
      log(LOG_ERROR, "Unable to create TLS context");
      config_freeServerConfig(config);
      string_free(privateKey);
      string_free(certificate);
      return 0;
    }

    if (SSL_CTX_use_PrivateKey_file(config->sslContext, string_getBuffer(privateKey), SSL_FILETYPE_PEM) <= 0) {
      log(LOG_ERROR, "Unable to create TLS context - could not read server private key");
      config_freeServerConfig(config);
      string_free(privateKey);
      return 0;
    }
    string_free(privateKey);

    if (SSL_CTX_use_certificate_file(config->sslContext, string_getBuffer(certificate), SSL_FILETYPE_PEM) <= 0) {
      log(LOG_ERROR, "Unable to create TLS context - could not read server certificate");
      config_freeServerConfig(config);
      string_free(certificate);
      return 0;
    }
    string_free(certificate);

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
        config_freeServerConfig(config);
        return 0;
      }
      string_free(dhparams);
    }
  }

  return config;
}

string_t *config_parseString(const toml_table_t *table, const char *key) {
  const char *rawValue = toml_raw_in((toml_table_t *)table, key);
  // The value is missing
  if (rawValue == 0)
    return 0;

  char *value;
  // The value is not a string
  if (toml_rtos(rawValue, &value)) {
    log(LOG_ERROR, "Bad string in config: table '%s', key '%s'", toml_table_key((toml_table_t *)table), key);
    return 0;
  }

  string_t *string = string_fromBuffer(value);
  free(value);

  return string;
}

int64_t config_parseInt(const toml_table_t *table, const char *key) {
  const char *rawValue = toml_raw_in((toml_table_t *)table, key);
  // The value is missing
  if (rawValue == 0)
    return 0;

  int64_t value = 0;
  if (toml_rtoi(rawValue, &value)) {
    log(LOG_ERROR, "Bad integer in config: table '%s', key '%s'", toml_table_key((toml_table_t *)table), key);
    return 0;
  }

  return value;
}

int8_t config_parseBool(const toml_table_t *table, const char *key) {
  const char *rawValue = toml_raw_in((toml_table_t *)table, key);
  // The value is missing
  if (rawValue == 0)
    return -1;

  int value = 0;
  if (toml_rtob(rawValue, &value)) {
    log(LOG_ERROR, "Bad bool in config: table '%s', key '%s'", toml_table_key((toml_table_t *)table), key);
    return -1;
  }

  return value;
}

list_t *config_parseArray(const toml_table_t *table, const char *key) {
  toml_array_t *array = toml_array_in((toml_table_t *)table, key);
  if (array == 0)
    return 0;

  list_t *list = list_create();
  if (list == 0)
    return 0;

  // i = int, d = double, b = bool, s = string, t = time, D = date, T = timestamp, 0 otherwise
  char type = toml_array_type(array);

  for (int i = 0; i < toml_array_nelem(array); i++) {
    const char *rawValue = toml_raw_at(array, i);
    if (rawValue == 0) {
      log(LOG_ERROR, "Unable to parse array");
      list_free(list);
      return 0;
    }

    if (type == 'i') {
      int64_t value = 0;
      toml_rtoi(rawValue, &value);

      list_addValue(list, (void *)value);
    } else if (type == 's') {
      char *value;
      // The value is not a string
      toml_rtos(rawValue, &value);

      string_t *string = string_fromBuffer(value);
      free(value);

      // Could not allocate string
      if (string == 0)
        return 0;

      list_addValue(list, string);
    } else if (type == 'b') {
      int value = 0;
      toml_rtob(rawValue, &value);

      list_addValue(list, (void *)value);
    }
  }

  return list;
}

int8_t config_getIsDaemon(const config_t *config) {
  return config->daemon;
}

void config_setIsDaemon(config_t *config, int8_t isDaemon) {
  config->daemon = isDaemon;
}

uint8_t config_getLoggingLevel(const config_t *config) {
  return config->loggingLevel;
}

void config_setLoggingLevel(config_t *config, uint8_t loggingLevel) {
  config->loggingLevel = loggingLevel;
}

server_config_t *config_getServerConfig(const config_t *config, size_t index) {
  return list_getValue(config->serverConfigs, index);
}

server_config_t *config_getServerConfigByHTTPSDomain(const config_t *config, const string_t *domain) {
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

server_config_t *config_getServerConfigByHTTPDomain(const config_t *config, const string_t *domain) {
  size_t servers = list_getLength(config->serverConfigs);
  for (size_t i = 0; i < servers; i++) {
    server_config_t *serverConfig = config_getServerConfig(config, i);
    bool isSameDomain = string_equals(config_getDomain(serverConfig), domain);
    bool hasSSL = serverConfig->sslContext != 0;
    if (isSameDomain && !hasSSL)
      return serverConfig;
  }

  return 0;
}

server_config_t *config_getServerConfigByDomain(const config_t *config, const string_t *domain, uint16_t port) {
  size_t servers = list_getLength(config->serverConfigs);
  for (size_t i = 0; i < servers; i++) {
    server_config_t *serverConfig = config_getServerConfig(config, i);
    string_t *otherDomain = config_getDomain(serverConfig);
    uint16_t otherPort = config_getPort(serverConfig);
    if (port == otherPort && string_equals(domain, otherDomain))
      return serverConfig;
  }

  return 0;
}

server_config_t *config_getServerConfigBySSLContext(const config_t *config, const SSL_CTX *sslContext) {
  size_t servers = list_getLength(config->serverConfigs);
  for (size_t i = 0; i < servers; i++) {
    server_config_t *serverConfig = config_getServerConfig(config, i);
    SSL_CTX *context = config_getSSLContext(serverConfig);
    if (sslContext == context)
      return serverConfig;
  }

  return 0;
}

size_t config_getServers(const config_t *config) {
  return list_getLength(config->serverConfigs);
}

size_t config_getNumberOfThreads(const config_t *config) {
  return config->threads;
}

size_t config_getBacklogSize(const config_t *config) {
  return config->backlog;
}

string_t *config_getName(const server_config_t *config) {
  return config->name;
}

string_t *config_getDomain(const server_config_t *config) {
  return config->domain;
}

string_t *config_getRootDirectory(const server_config_t *config) {
  return config->rootDirectory;
}

int16_t config_getPort(const server_config_t *config) {
  return config->port;
}

void config_setPort(server_config_t *config, int16_t port) {
  config->port = port;
}

string_t *config_getLogfile(const config_t *config) {
  return config->logfile;
}

void config_setLogfile(config_t *config, string_t *logfile) {
  if (config->logfile != 0)
    string_free(config->logfile);

  config->logfile = logfile;
}

SSL_CTX *config_getSSLContext(const server_config_t *config) {
  return config->sslContext;
}

DH *config_getDiffieHellmanParameters(const server_config_t *config) {
  return config->dhparams;
}

list_t *config_getDirectoryIndex(const server_config_t *config) {
  return config->directoryIndex;
}

void config_freeServerConfig(server_config_t *serverConfig) {
  if (serverConfig->name != 0)
    string_free(serverConfig->name);
  if (serverConfig->domain != 0)
    string_free(serverConfig->domain);
  if (serverConfig->rootDirectory != 0)
    string_free(serverConfig->rootDirectory);
  if (serverConfig->dhparams != 0)
    DH_free(serverConfig->dhparams);
  if (serverConfig->sslContext != 0)
    SSL_CTX_free(serverConfig->sslContext);
  if (serverConfig->directoryIndex != 0) {
    string_t *index = 0;
    while ((index = list_removeValue(serverConfig->directoryIndex, 0)) != 0)
      string_free(index);
    list_free(serverConfig->directoryIndex);
  }
  free(serverConfig);
}

void config_free(config_t *config) {
  server_config_t *serverConfig = 0;
  while ((serverConfig = list_removeValue(config->serverConfigs, 0)) != 0)
    config_freeServerConfig(serverConfig);
  list_free(config->serverConfigs);
  if (config->logfile != 0)
    string_free(config->logfile);
  free(config);
}
