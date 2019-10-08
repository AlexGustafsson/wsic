#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "tomlc99/toml.h"
#include "unity/unity.h"

#include "../src/datastructures/list/list.h"

#include "../src/config/config.h"

void config_test_canParseString() {
  // Parse a table containg a string, "string" = "Foo Bar"
  char *tableString = "[test]\nstring=\"Foo Bar\"";
  toml_table_t *toml = toml_parse(tableString, 0, 0);
  toml_table_t *table = toml_table_in(toml, "test");

  string_t *string = config_parseString(table, "string");
  // Assert that the value was correctly parsed
  TEST_ASSERT_NOT_NULL(string);
  // Assert that it matches our expected string
  TEST_ASSERT_EQUAL_STRING("Foo Bar", string_getBuffer(string));

  string_free(string);
  toml_free(table);
}

void config_test_cannotParseNonExistingString() {
  // Parse a table
  char *tableString = "[test]\n";
  toml_table_t *toml = toml_parse(tableString, 0, 0);
  toml_table_t *table = toml_table_in(toml, "test");

  string_t *string = config_parseString(table, "string");
  // Assert that the value was not found and therefore returned null
  TEST_ASSERT_NULL(string);

  toml_free(table);
}

void config_test_cannotParseInvalidString() {
  // Parse a table containg a string, "string" = "Foo Bar"
  char *tableString = "[test]\nstring=true";
  toml_table_t *toml = toml_parse(tableString, 0, 0);
  toml_table_t *table = toml_table_in(toml, "test");

  string_t *string = config_parseString(table, "string");
  // Assert that the value was not parsed correctly and therefore returned null
  TEST_ASSERT_NULL(string);

  toml_free(table);
}

void config_test_canParseInt() {
  // Parse a table containg an int, "integer" = 1337
  char *tableString = "[test]\ninteger=1337";
  toml_table_t *toml = toml_parse(tableString, 0, 0);
  toml_table_t *table = toml_table_in(toml, "test");

  int64_t integer = config_parseInt(table, "integer");
  // Assert that it matches our expected integer
  TEST_ASSERT_EQUAL_INT64(1337, integer);

  toml_free(table);
}

void config_test_cannotParseNonExistingInt() {
  // Parse a table
  char *tableString = "[test]\n";
  toml_table_t *toml = toml_parse(tableString, 0, 0);
  toml_table_t *table = toml_table_in(toml, "test");

  int64_t integer = config_parseInt(table, "integer");
  // Assert that it matches our expected integer
  TEST_ASSERT_EQUAL_INT64(0, integer);

  toml_free(table);
}

void config_test_cannotParseInvalidInt() {
  // Parse a table
  char *tableString = "[test]\ninteger=false";
  toml_table_t *toml = toml_parse(tableString, 0, 0);
  toml_table_t *table = toml_table_in(toml, "test");

  int64_t integer = config_parseInt(table, "integer");
  // Assert that it matches our expected integer
  TEST_ASSERT_EQUAL_INT64(0, integer);

  toml_free(table);
}

void config_test_canParseBool() {
  // Parse a table containg a bool, "bool" = true
  char *tableString = "[test]\nbool=true";
  toml_table_t *toml = toml_parse(tableString, 0, 0);
  toml_table_t *table = toml_table_in(toml, "test");

  int8_t value = config_parseBool(table, "bool");
  // Assert that it matches our expected integer
  TEST_ASSERT_EQUAL_INT8(true, value);

  toml_free(table);
}

void config_test_cannotParseNonExistingBool() {
  // Parse a table
  char *tableString = "[test]\n";
  toml_table_t *toml = toml_parse(tableString, 0, 0);
  toml_table_t *table = toml_table_in(toml, "test");

  int8_t value = config_parseBool(table, "bool");
  // Assert that it matches our expected integer
  TEST_ASSERT_EQUAL_INT8(-1, value);

  toml_free(table);
}

void config_test_cannotParseInvalidBool() {
  // Parse a table containing a faulty bool
  char *tableString = "[test]\nbool=flse";
  toml_table_t *toml = toml_parse(tableString, 0, 0);
  toml_table_t *table = toml_table_in(toml, "test");

  int8_t value = config_parseBool(table, "bool");
  // Assert that it matches our expected integer
  TEST_ASSERT_EQUAL_INT8(-1, value);

  toml_free(table);
}

void config_test_canParseStringArray() {
  // Parse a table containing an array of strings
  char *tableString = "[test]\nstrings=[\"Foo\", \"Bar\"]";
  toml_table_t *toml = toml_parse(tableString, 0, 0);
  toml_table_t *table = toml_table_in(toml, "test");

  list_t *strings = config_parseArray(table, "strings");
  // Assert that it was parsed corretly
  TEST_ASSERT_NOT_NULL(strings);
  TEST_ASSERT_EQUAL_STRING("Foo", string_getBuffer(list_getValue(strings, 0)));
  TEST_ASSERT_EQUAL_STRING("Bar", string_getBuffer(list_getValue(strings, 1)));

  list_free(strings);
  toml_free(table);
}

void config_test_canParseIntArray() {
  // Parse a table containing an array of strings
  char *tableString = "[test]\nintegers=[4, 2, 0]";
  toml_table_t *toml = toml_parse(tableString, 0, 0);
  toml_table_t *table = toml_table_in(toml, "test");

  list_t *integers = config_parseArray(table, "integers");
  // Assert that it was parsed corretly
  TEST_ASSERT_NOT_NULL(integers);
  TEST_ASSERT_EQUAL_INT64(4, list_getValue(integers, 0));
  TEST_ASSERT_EQUAL_INT64(2, list_getValue(integers, 1));
  TEST_ASSERT_EQUAL_INT64(0, list_getValue(integers, 2));

  list_free(integers);
  toml_free(table);
}

void config_test_canParseBoolArray() {
  // Parse a table containing an array of strings
  char *tableString = "[test]\nbools=[true, true, false]";
  toml_table_t *toml = toml_parse(tableString, 0, 0);
  toml_table_t *table = toml_table_in(toml, "test");

  list_t *bools = config_parseArray(table, "bools");
  // Assert that it was parsed corretly
  TEST_ASSERT_NOT_NULL(bools);
  TEST_ASSERT_EQUAL_INT8(true, list_getValue(bools, 0));
  TEST_ASSERT_EQUAL_INT8(true, list_getValue(bools, 1));
  TEST_ASSERT_EQUAL_INT8(false, list_getValue(bools, 2));

  list_free(bools);
  toml_free(table);
}

void config_test_canGetIsDaemon() {
  config_t config;
  memset(&config, 0, sizeof(config));
  config.daemon = true;

  TEST_ASSERT_EQUAL_INT(true, config_getIsDaemon(&config));
}

void config_test_canSetIsDaemon() {
  config_t config;
  memset(&config, 0, sizeof(config));
  config_setIsDaemon(&config, true);

  TEST_ASSERT_EQUAL_INT(true, config.daemon);
}

void config_test_canGetName() {
  server_config_t config;
  memset(&config, 0, sizeof(config));
  config.name = string_fromBuffer("Foo Bar");

  TEST_ASSERT_EQUAL_STRING("Foo Bar", string_getBuffer(config_getName(&config)));

  string_free(config.name);
}

void config_test_canGetDomain() {
  server_config_t config;
  memset(&config, 0, sizeof(config));
  config.domain = string_fromBuffer("Foo Bar");

  TEST_ASSERT_EQUAL_STRING("Foo Bar", string_getBuffer(config_getDomain(&config)));

  string_free(config.domain);
}

void config_test_canGetRootDirectory() {
  server_config_t config;
  memset(&config, 0, sizeof(config));
  config.rootDirectory = string_fromBuffer("Foo Bar");

  TEST_ASSERT_EQUAL_STRING("Foo Bar", string_getBuffer(config_getRootDirectory(&config)));

  string_free(config.rootDirectory);
}

void config_test_canGetPort() {
  server_config_t config;
  memset(&config, 0, sizeof(config));
  config.port = 4040;

  TEST_ASSERT_EQUAL_UINT16(4040, config_getPort(&config));
}

void config_test_canSetPort() {
  server_config_t config;
  memset(&config, 0, sizeof(config));
  config_setPort(&config, 4040);

  TEST_ASSERT_EQUAL_UINT16(4040, config.port);
}

void config_test_canGetLogfile() {
  config_t config;
  memset(&config, 0, sizeof(config));
  config.logfile = string_fromBuffer("foo");

  TEST_ASSERT_EQUAL_STRING("foo", string_getBuffer(config_getLogfile(&config)));

  string_free(config.logfile);
}

void config_test_canSetLogfile() {
  config_t config;
  memset(&config, 0, sizeof(config));
  config_setLogfile(&config, string_fromBuffer("foo"));

  TEST_ASSERT_EQUAL_STRING("foo", string_getBuffer(config.logfile));

  string_free(config.logfile);
}

void config_test_canGetSSLContext() {
  server_config_t config;
  memset(&config, 0, sizeof(config));
  config.sslContext = (SSL_CTX *)10;

  TEST_ASSERT(config.sslContext == config_getSSLContext(&config));
}

void config_test_canGetDiffieHellmanParameters() {
  server_config_t config;
  memset(&config, 0, sizeof(config));
  config.dhparams = (DH *)10;

  TEST_ASSERT(config.dhparams == config_getDiffieHellmanParameters(&config));
}

void config_test_canGetDirectoryIndex() {
  server_config_t config;
  memset(&config, 0, sizeof(config));
  config.directoryIndex = (list_t *)10;

  TEST_ASSERT(config.directoryIndex == config_getDirectoryIndex(&config));
}

void config_test_canAccessGlobalConfig() {
  config_t *config = malloc(sizeof(config_t));
  memset(config, 0, sizeof(config_t));
  config->serverConfigs = list_create();

  config_setGlobalConfig(config);

  TEST_ASSERT(config_getGlobalConfig() == config);

  config_freeGlobalConfig();
}

void config_test_canParseConfig() {
  char *configString = "\
  [server]\n\
  daemon = false\n\
  \n\
  [servers]\n\
  [servers.default]\n\
  domain = \"localhost\"\n\
  rootDirectory = \"www\"\n\
  port = 8080\n\
  directoryIndex = [\"index.html\"]\n\
  [servers.defaultTLS]\n\
  domain = \"localhost\"\n\
  rootDirectory = \"www\"\n\
  port = 8443\n\
  directoryIndex = [\"index.html\"]\n\
  certificate = \"server.cert\"\n\
  privateKey = \"server.key\"\n\
  ellipticCurves = \"P-384:P-521\"\n\
  validateCertificate = false";

  config_t *config = config_parse(configString);
  TEST_ASSERT_NOT_NULL(config);

  // Ensure that both servers were parsed
  TEST_ASSERT(config_getServers(config) == 2);

  config_free(config);
}

void config_test_cannotParseInvalidConfig() {
  char *configString = "\
  [server]\n\
  a\n\
  [b\n";

  config_t *config = config_parse(configString);
  TEST_ASSERT_NULL(config);
}

void config_test_canParseWithoutServerTable() {
  char *configString = "[server]";

  config_t *config = config_parse(configString);
  TEST_ASSERT_NOT_NULL(config);
}

void config_test_canParseDuplicateServerTables() {
  char *configString = "\
  [server]\n\
  daemon = false\n\
  \n\
  [servers]\n\
  [servers.default]\n\
  domain = \"localhost\"\n\
  rootDirectory = \"www\"\n\
  port = 8443\n\
  directoryIndex = [\"index.html\"]\n\
  certificate = \"server.cert\"\n\
  privateKey = \"server.key\"\n\
  ellipticCurves = \"P-384:P-521\"\n\
  validateCertificate = false\n\
  [servers.default2]\n\
  domain = \"localhost\"\n\
  rootDirectory = \"www\"\n\
  port = 8080\n\
  directoryIndex = [\"index.html\"]\n\
  certificate = \"server.cert\"\n\
  privateKey = \"server.key\"\n\
  ellipticCurves = \"P-384:P-521\"\n\
  validateCertificate = false";

  config_t *config = config_parse(configString);
  TEST_ASSERT_NOT_NULL(config);

  // Ensure that only one server was parsed
  TEST_ASSERT(config_getServers(config) == 1);

  config_free(config);
}

void config_test_cannotParseInvalidRootDirectory() {
  char *configString = "\
  [server]\n\
  [servers.test]\n\
  rootDirectory=\"thispathdoesnotexist\"";

  config_t *config = config_parse(configString);
  TEST_ASSERT_NOT_NULL(config);

  TEST_ASSERT(config_getServers(config) == 0);

  config_free(config);
}

void config_test_cannotParseInvalidPort() {
  char *configString = "\
  [server]\n\
  [servers.test]\n\
  port=9999999";

  config_t *config = config_parse(configString);
  TEST_ASSERT_NOT_NULL(config);

  TEST_ASSERT(config_getServers(config) == 0);

  config_free(config);
}

void config_test_cannotParseInvalidTLSConfig() {
  char *configString = "\
  [server]\n\
  [servers.test]\n\
  privateKey=\"someprivatekey\"";

  config_t *config = config_parse(configString);
  TEST_ASSERT_NOT_NULL(config);

  TEST_ASSERT(config_getServers(config) == 0);

  config_free(config);
}

void config_test_run() {
  RUN_TEST(config_test_canParseString);
  RUN_TEST(config_test_cannotParseNonExistingString);
  RUN_TEST(config_test_cannotParseInvalidString);

  RUN_TEST(config_test_canParseInt);
  RUN_TEST(config_test_cannotParseNonExistingInt);
  RUN_TEST(config_test_cannotParseInvalidInt);

  RUN_TEST(config_test_canParseBool);
  RUN_TEST(config_test_cannotParseNonExistingBool);
  RUN_TEST(config_test_cannotParseInvalidBool);

  RUN_TEST(config_test_canParseStringArray);
  RUN_TEST(config_test_canParseIntArray);
  RUN_TEST(config_test_canParseBoolArray);

  RUN_TEST(config_test_canGetIsDaemon);
  RUN_TEST(config_test_canSetIsDaemon);

  RUN_TEST(config_test_canGetName);

  RUN_TEST(config_test_canGetDomain);

  RUN_TEST(config_test_canGetPort);
  RUN_TEST(config_test_canSetPort);

  RUN_TEST(config_test_canGetRootDirectory);

  RUN_TEST(config_test_canGetLogfile);
  RUN_TEST(config_test_canSetLogfile);

  RUN_TEST(config_test_canGetSSLContext);

  RUN_TEST(config_test_canGetDiffieHellmanParameters);

  RUN_TEST(config_test_canGetDirectoryIndex);

  RUN_TEST(config_test_canAccessGlobalConfig);

  RUN_TEST(config_test_canParseConfig);
  RUN_TEST(config_test_cannotParseInvalidConfig);
  RUN_TEST(config_test_canParseWithoutServerTable);
  RUN_TEST(config_test_canParseDuplicateServerTables);
  RUN_TEST(config_test_cannotParseInvalidRootDirectory);
  RUN_TEST(config_test_cannotParseInvalidPort);
  RUN_TEST(config_test_cannotParseInvalidTLSConfig);
}
