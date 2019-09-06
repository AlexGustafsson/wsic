#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

typedef struct config {
  // The root www directory
  const char *rootDirectory;
  uint16_t port;
  const char *parallelMode;
} config_t;

config_t *parseConfig(const char *filename);
void freeConfig(config_t *config);

#endif
