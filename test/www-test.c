#include "../src/resources/resources.h"
#include "unity/unity.h"

#include "../src/www/www.h"

void www_test_canGetAndSetSource() {
  page_t *page = page_create();

  // Set the source to the template
  page_setSource(page, string_fromBuffer("Welcome to my website this is the source page!"));
  // Try to get the source page
  TEST_ASSERT_EQUAL_STRING("Welcome to my website this is the source page!", string_getBuffer(page_getSource(page)));

  // Set the source to the template
  page_setSource(page, string_fromBuffer("Now i have a new frontpage"));
  // Try to get the source page
  TEST_ASSERT_EQUAL_STRING("Now i have a new frontpage", string_getBuffer(page_getSource(page)));

  page_free(page);
}

void www_test_canSetTemplate() {
  page_t *page = page_create();
  string_t *key = string_fromBuffer("content");
  string_t *value = string_fromBuffer("I know secure PHP");

  page_setTemplate(page, key, value);
  TEST_ASSERT_EQUAL_STRING("I know secure PHP", string_getBuffer(hash_table_getValue(page->templates, key)));

  page_free(page);
}

void www_test_canResolveTemplate() {
  page_t *page = page_create();

  // Set the page to template
  page_setSource(page, string_fromBuffer("Hello {{content}}!"));
  // Set the parameter content to World
  page_setTemplate(page, string_fromBuffer("content"), string_fromBuffer("World"));
  // resolve the content, change {{content}} to World
  page_resolveTemplate(page, 0);

  // Check to find if the source page has changed
  TEST_ASSERT_EQUAL_STRING("Hello World!", string_getBuffer(page->source));

  page_free(page);
}

void www_test_canResolveMultipleTemplates() {
  // Normal use of resovle
  page_t *page = page_create();

  page_setSource(page, string_fromBuffer("Hello my name is {{firstname}}! And i'm {{years}} old."));
  page_setTemplate(page, string_fromBuffer("firstname"), string_fromBuffer("Marcus {{lastname}}"));
  page_setTemplate(page, string_fromBuffer("lastname"), string_fromBuffer("Lenander"));
  page_setTemplate(page, string_fromBuffer("years"), string_fromBuffer("23"));
  page_resolveTemplates(page);

  TEST_ASSERT(page->templates == 0);

  // Check to find if the source page has changed
  TEST_ASSERT_EQUAL_STRING("Hello my name is Marcus Lenander! And i'm 23 old.", string_getBuffer(page->source));

  page_free(page);
}

void www_test_resolveTemplatesWithThreeCurlyBrackets() {
  page_t *page = page_create();

  page_setSource(page, string_fromBuffer("Hello my name is {{{firstname}} {{lastname}}}!"));
  page_setTemplate(page, string_fromBuffer("firstname"), string_fromBuffer("Marcus"));
  page_setTemplate(page, string_fromBuffer("lastname"), string_fromBuffer("Lenander"));
  page_resolveTemplates(page);

  // Check to find if the source page has changed
  TEST_ASSERT_EQUAL_STRING("Hello my name is {Marcus Lenander}!", string_getBuffer(page->source));

  page_free(page);
}

void www_test_resolveTemplateWithMissingCurlyBracket() {
  page_t *page = page_create();

  page_setSource(page, string_fromBuffer("Hello my name is {firstname}} {{lastname}!"));
  page_setTemplate(page, string_fromBuffer("firstname"), string_fromBuffer("Marcus"));
  page_setTemplate(page, string_fromBuffer("lastname"), string_fromBuffer("Lenander"));
  page_resolveTemplates(page);

  // Check to find if the source page has changed
  TEST_ASSERT_EQUAL_STRING("Hello my name is {firstname}} {{lastname}!", string_getBuffer(page->source));

  page_free(page);
}

void www_test_resolveTemplatesWhenThereIsNoTemplates() {
  page_t *page = page_create();

  page_setSource(page, string_fromBuffer("Insert joke here: {{joke}}"));
  page_resolveTemplates(page);

  // Check to find if the source page has changed
  TEST_ASSERT_EQUAL_STRING("Insert joke here: ", string_getBuffer(page->source));

  page_free(page);
}

void www_test_resolveTemplatesWhenTemplateIsEmpty() {
  page_t *page = page_create();

  page_setSource(page, string_fromBuffer("Insert joke here: {{joke}}"));
  page_setTemplate(page, string_fromBuffer("joke"), string_fromBuffer(""));
  page_resolveTemplates(page);

  // Check to find if the source page has changed
  TEST_ASSERT_EQUAL_STRING("Insert joke here: ", string_getBuffer(page->source));

  page_free(page);
}

void www_test_resolveTemplateWithEmptyCurlyBrackets() {
  page_t *page = page_create();

  page_setSource(page, string_fromBuffer("Insert joke here: {{}}"));
  page_setTemplate(page, string_fromBuffer("joke"), string_fromBuffer("Me"));
  page_resolveTemplates(page);

  // Check to find if the source page has changed
  TEST_ASSERT_EQUAL_STRING("Insert joke here: ", string_getBuffer(page->source));

  page_free(page);
}

void www_test_canSetEmptyTemplate() {
  page_t *page = page_create();
  string_t *key = string_fromBuffer("content");
  string_t *value = string_fromBuffer("");

  page_setTemplate(page, key, value);

  TEST_ASSERT_EQUAL_STRING("", string_getBuffer(hash_table_getValue(page->templates, key)));

  page_free(page);
}

void www_test_page_canClearTemplates() {
}

void www_test_canCreatePage400() {
  page_t *page = page_create400(string_fromBuffer("Test description"));

  TEST_ASSERT(page->templates == 0);

  page_free(page);
}

void www_test_canCreatePage403() {
  page_t *page = page_create403();

  TEST_ASSERT(page->templates == 0);

  page_free(page);
}

void www_test_canCreatePage404() {
  page_t *page = page_create404(string_fromBuffer("Test path"));

  TEST_ASSERT(page->templates == 0);

  page_free(page);
}

void www_test_canCreatePage500() {
  page_t *page = page_create500(string_fromBuffer("Test description"));

  TEST_ASSERT(page->templates == 0);

  page_free(page);
}

void www_test_canCreatePage501() {
  page_t *page = page_create501();

  TEST_ASSERT(page->templates == 0);

  page_free(page);
}

void www_test_run() {
  RUN_TEST(www_test_canGetAndSetSource);
  RUN_TEST(www_test_canSetTemplate);
  RUN_TEST(www_test_canSetEmptyTemplate);
  RUN_TEST(www_test_canResolveTemplate);
  RUN_TEST(www_test_canResolveMultipleTemplates);
  RUN_TEST(www_test_page_canClearTemplates);

  // Special case tests for page_resolveTemplates()
  RUN_TEST(www_test_resolveTemplatesWithThreeCurlyBrackets);
  RUN_TEST(www_test_resolveTemplateWithMissingCurlyBracket);
  RUN_TEST(www_test_resolveTemplatesWhenThereIsNoTemplates);
  RUN_TEST(www_test_resolveTemplatesWhenTemplateIsEmpty);
  RUN_TEST(www_test_resolveTemplateWithEmptyCurlyBrackets);

  RUN_TEST(www_test_canCreatePage400);
  RUN_TEST(www_test_canCreatePage403);
  RUN_TEST(www_test_canCreatePage404);
  RUN_TEST(www_test_canCreatePage500);
  RUN_TEST(www_test_canCreatePage501);
}
