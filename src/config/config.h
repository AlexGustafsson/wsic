#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

#include "../includes/tomlc99/toml.h"

#include "../datastructures/list/list.h"

enum parallelMode {PARALLEL_MODE_FORK, PARALLEL_MODE_PRE_FORK, PARALLEL_MODE_UNKNOWN};

typedef struct {
  char *name;
  char *domain;
  char *rootDirectory;
  uint16_t port;
  char *logfile;
} server_config_t;

typedef struct {
  enum parallelMode parallelMode;
  list_t *serverConfigs;
} config_t;

config_t *config_parse(char *configString);

server_config_t *config_parseServerTable(toml_table_t *serverTable);

const char *config_parseString(toml_table_t *table, const char *key);
int64_t config_parseInt(toml_table_t *table, const char *key);

void config_free(config_t *config);

#endif
