#include <string.h>

#include "unity/unity.h"

#include "../src/string/string.h"

void string_test_canCreateStringFromBuffer() {
  // A null terminated buffer-based string
  const char *buffer = "Hello world";
  // Create a string from the buffer
  string_t *string = string_fromBuffer(buffer);

  // Ensure that the buffer size is at least as large as the string and a null byte
  TEST_ASSERT(string->bufferSize >= 11 + 1);

  // Ensure that the content's length is equal to the string's length
  TEST_ASSERT(string->size == 11);

  // Ensure that the buffer is null terminated
  TEST_ASSERT(string->buffer[string->size] == 0);

  // Ensure that the contents are the same
  TEST_ASSERT_EQUAL_STRING(buffer, string->buffer);

  string_free(string);
}

void string_test_canCreateStringFromBufferWithLength() {
  // A null terminated buffer-based string
  const char *buffer = "Hello world - not everything is added";
  const char *expectedStoredString = "Hello world";
  // Create a string from the buffer
  string_t *string = string_fromBufferWithLength(buffer, 11);

  // Ensure that the buffer size is at least as large as the string and a null byte
  TEST_ASSERT(string->bufferSize >= 11 + 1);

  // Ensure that the content's length is equal to the string's length
  TEST_ASSERT(string->size == 11);

  // Ensure that the buffer is null terminated
  TEST_ASSERT(string->buffer[string->size] == 0);

  // Ensure that the contents are the same
  TEST_ASSERT_EQUAL_STRING(expectedStoredString, string->buffer);

  string_free(string);
}

void string_test_canClearString() {
  // A null terminated buffer-based string
  const char *buffer = "Hello world";
  // Create a string from the buffer
  string_t *string = string_fromBuffer(buffer);

  // Clear the string
  string_clear(string);

  // Ensure that the content's length is cleared
  TEST_ASSERT(string->size == 0);

  // Ensure that the buffer is null terminated
  TEST_ASSERT(string->buffer[0] == 0);

  string_free(string);
}

void string_test_canCreateStringFromPositiveInt() {
  // An arbitrary, positive int to parse
  int positive = 5812491;
  const char *expectedPositiveString = "5812491";
  // Create a string by parsing the positive int
  string_t *string = string_fromInt(positive);

  // Ensure that the buffer size is at least as large as the string and a null byte
  TEST_ASSERT(string->bufferSize >= 7 + 1);

  // Ensure that the content's length is equal to the string's length
  TEST_ASSERT(string->size == 7);

  // Ensure that the buffer is null terminated
  TEST_ASSERT(string->buffer[string->size] == 0);

  // Ensure that the contents are the same
  TEST_ASSERT_EQUAL_STRING(expectedPositiveString, string->buffer);

  string_free(string);
}

void string_test_canCreateStringFromNegativeInt() {
  // An arbitrary, negative int to parse
  int negative = -5167283;
  const char *expectedPositiveString = "-5167283";
  // Create a string by parsing the negative int
  string_t *string = string_fromInt(negative);

  // Ensure that the buffer size is at least as large as the string and a null byte
  TEST_ASSERT(string->bufferSize >= 8 + 1);

  // Ensure that the content's length is equal to the string's length
  TEST_ASSERT(string->size == 8);

  // Ensure that the buffer is null terminated
  TEST_ASSERT(string->buffer[string->size] == 0);

  // Ensure that the contents are the same
  TEST_ASSERT_EQUAL_STRING(expectedPositiveString, string->buffer);

  string_free(string);
}

void string_test_canAppendString() {
  // The base string to append to
  string_t *base = string_fromBuffer("Foo");
  // The string to append
  string_t *stringToAppend = string_fromBuffer(" Bar");
  // The expected result after appending
  const char *expectedResult = "Foo Bar";

  // Perform the appending
  string_append(base, stringToAppend);

  // Ensure that the buffer size is at least as large as the string and a null byte
  TEST_ASSERT(base->bufferSize >= 7 + 1);

  // Ensure that the content's length is equal to the results's length
  TEST_ASSERT(base->size == 7);

  // Ensure that the buffer is null terminated
  TEST_ASSERT(base->buffer[base->size] == 0);

  // Ensure that the contents are the same
  TEST_ASSERT_EQUAL_STRING(expectedResult, base->buffer);

  string_free(base);
  string_free(stringToAppend);
}

void string_test_canAppendChar() {
  // The base string to append to
  string_t *base = string_fromBuffer("Hello World");
  // The char to append
  char character = '!';
  // The expected result after appending
  const char *expectedResult = "Hello World!";

  // Perform the appending
  string_appendChar(base, character);

  // Ensure that the buffer size is at least as large as the string and a null byte
  TEST_ASSERT(base->bufferSize >= 12 + 1);

  // Ensure that the content's length is equal to the results's length
  TEST_ASSERT(base->size == 12);

  // Ensure that the buffer is null terminated
  TEST_ASSERT(base->buffer[base->size] == 0);

  // Ensure that the contents are the same
  TEST_ASSERT_EQUAL_STRING(expectedResult, base->buffer);

  string_free(base);
}

void string_test_canAppendBuffer() {
  // The base string to append to
  string_t *base = string_fromBuffer("Foo");
  // The buffer to append
  const char *buffer = " Bar";
  // The expected result after appending
  const char *expectedResult = "Foo Bar";

  // Perform the appending
  string_appendBuffer(base, buffer);

  // Ensure that the buffer size is at least as large as the string and a null byte
  TEST_ASSERT(base->bufferSize >= 7 + 1);

  // Ensure that the content's length is equal to the results's length
  TEST_ASSERT(base->size == 7);

  // Ensure that the buffer is null terminated
  TEST_ASSERT(base->buffer[base->size] == 0);

  // Ensure that the contents are the same
  TEST_ASSERT_EQUAL_STRING(expectedResult, base->buffer);

  string_free(base);
}

void string_test_canAppendBufferWithLength() {
  // The base string to append to
  string_t *base = string_fromBuffer("Foo");
  // The buffer to append
  const char *buffer = " Bar Baz";
  // The expected result after appending
  const char *expectedResult = "Foo Bar";

  // Perform the appending
  string_appendBufferWithLength(base, buffer, 4);

  // Ensure that the buffer size is at least as large as the string and a null byte
  TEST_ASSERT(base->bufferSize >= 7 + 1);

  // Ensure that the content's length is equal to the results's length
  TEST_ASSERT(base->size == 7);

  // Ensure that the buffer is null terminated
  TEST_ASSERT(base->buffer[base->size] == 0);

  // Ensure that the contents are the same
  TEST_ASSERT_EQUAL_STRING(expectedResult, base->buffer);

  string_free(base);

  string_t *string = string_fromBuffer("Hello Wolrd!");
  char *b = "";
  string_appendBufferWithLength(string, b, 1);

  string_free(string);
}

void string_test_canGetExistingChar() {
  // A base string
  string_t *string = string_fromBuffer("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  char expected = 'M';
  char actual = string_getCharAt(string, 12);

  // Ensure that there is an M on index 12
  TEST_ASSERT(expected == actual);

  string_free(string);
}

void string_test_cannotGetNonExistingChar() {
  // A base string
  string_t *string = string_fromBuffer("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  char expected = 0;
  char actual = string_getCharAt(string, 100);

  // Ensure that we are unable to get a char out of bounds
  TEST_ASSERT(expected == actual);

  string_free(string);
}

void string_test_canSetExistingChar() {
  // A base string
  string_t *string = string_fromBuffer("Foo Car Baz");
  const char *expected = "Foo Bar Baz";

  // Replace the 'C' with a 'B'
  string_setCharAt(string, 4, 'B');

  // Ensure that the C and only the C was replaced
  TEST_ASSERT_EQUAL_STRING(expected, string->buffer);

  string_free(string);
}

void string_test_cannotSetNonExistingChar() {
  // A base string
  string_t *string = string_fromBuffer("Foo Bar Baz");
  const char *expected = "Foo Bar Baz";

  // Try to set a character out of bounds
  string_setCharAt(string, 11, '!');

  // Ensure that the string remains unchanged
  TEST_ASSERT_EQUAL_STRING(expected, string->buffer);
  // Ensure that there was nothing set out of bounds
  TEST_ASSERT(string_getCharAt(string, 11) == 0);

  string_free(string);
}

void string_test_canTrimEnd() {
  // A base string
  string_t *string = string_fromBuffer("Hello world \n\r\t ");
  const char *expected = "Hello world";

  // Perform trim the end
  string_trimEnd(string);

  // Ensure that the buffer size is at least as large as the string and a null byte
  TEST_ASSERT(string->bufferSize >= 11 + 1);

  // Ensure that the content's length is equal to the results's length
  TEST_ASSERT(string->size == 11);

  // Ensure that the buffer is null terminated
  TEST_ASSERT(string->buffer[string->size] == 0);

  // Ensure that the contents are the same
  TEST_ASSERT_EQUAL_STRING(expected, string->buffer);

  string_free(string);
}

void string_test_canCanCompareEqualStrings() {
  // Two equal strings
  string_t *equal1 = string_fromBuffer("Foo Bar");
  string_t *equal2 = string_fromBuffer("Foo Bar");

  // Ensure that the strings are equal
  TEST_ASSERT_TRUE(string_equals(equal1, equal2));

  string_free(equal1);
  string_free(equal2);
}

void string_test_canCompareNonEqualStrings() {
  // Two non-equal strings
  string_t *nonEqual1 = string_fromBuffer("Foo Bar");
  string_t *nonEqual2 = string_fromBuffer("Baz Biz");

  // Ensure that the strings are not equal
  TEST_ASSERT_FALSE(string_equals(nonEqual1, nonEqual2));

  string_free(nonEqual1);
  string_free(nonEqual2);
}

void string_test_canCanCompareEqualStringAndBuffer() {
  // Two equal strings
  string_t *equal1 = string_fromBuffer("Foo Bar");
  const char *equal2 = "Foo Bar";

  // Ensure that the strings are equal
  TEST_ASSERT_TRUE(string_equalsBuffer(equal1, equal2));

  string_free(equal1);
}

void string_test_canCompareNonEqualStringAndBuffer() {
  // Two non-equal strings
  string_t *nonEqual1 = string_fromBuffer("Foo Bar");
  const char *nonEqual2 = "Baz Biz";

  // Ensure that the strings are not equal
  TEST_ASSERT_FALSE(string_equalsBuffer(nonEqual1, nonEqual2));

  string_free(nonEqual1);
}

void string_test_canLoopThroughCharacters() {
  string_t *characters = string_fromBuffer("a1b2c3");
  string_cursor_t *cursor = string_createCursor(characters);

  // Ensure that each character matches
  TEST_ASSERT(string_getNextChar(cursor) == 'a');
  // Ensure that the offset has been changed to be the next character
  TEST_ASSERT(string_getOffset(cursor) == 1);
  TEST_ASSERT(string_getNextChar(cursor) == '1');
  // Ensure that one can move the cursor
  string_setOffset(cursor, 1);
  // Ensure that the same character is read again due to us moving the cursor
  TEST_ASSERT(string_getNextChar(cursor) == '1');
  // Ensure that each character matches
  TEST_ASSERT(string_getNextChar(cursor) == 'b');
  TEST_ASSERT(string_getNextChar(cursor) == '2');
  TEST_ASSERT(string_getNextChar(cursor) == 'c');
  TEST_ASSERT(string_getNextChar(cursor) == '3');
  // Ensure that the function returns null when out of bounds
  TEST_ASSERT_EQUAL_UINT8(0, string_getNextChar(cursor));
  TEST_ASSERT_EQUAL_UINT8(0, string_getNextChar(cursor));

  string_freeCursor(cursor);
  string_free(characters);
}

void string_test_canLoopThroughLines() {
  string_t *lines = string_fromBuffer("Line1\nLine2\r\nLine3");
  string_cursor_t *cursor = string_createCursor(lines);

  string_t *line1 = string_getNextLine(cursor);
  const char *expectedLine1 = "Line1";
  // Ensure that the first line was extracted without a trailing LF
  TEST_ASSERT_EQUAL_STRING(expectedLine1, line1->buffer);
  string_free(line1);

  string_t *line2 = string_getNextLine(cursor);
  const char *expectedLine2 = "Line2";
  // Ensure that the first line was extracted without trailing CRLF
  TEST_ASSERT_EQUAL_STRING(expectedLine2, line2->buffer);
  string_free(line2);

  string_t *line3 = string_getNextLine(cursor);
  const char *expectedLine3 = "Line3";
  // Ensure that the last line was extracted due to EOF
  TEST_ASSERT_EQUAL_STRING(expectedLine3, line3->buffer);
  string_free(line3);

  // Ensure that further calls return null
  TEST_ASSERT_NULL(string_getNextLine(cursor));
  TEST_ASSERT_NULL(string_getNextLine(cursor));

  string_freeCursor(cursor);
  string_free(lines);
}

void string_test_canFindNextChar() {
  string_t *string = string_fromBuffer("3h43SD#!*()DD*");
  string_cursor_t *cursor = string_createCursor(string);
  ssize_t parameter0 = 8;
  ssize_t parameter1 = 13;

  TEST_ASSERT(string_findNextChar(cursor, '*') == parameter0);
  TEST_ASSERT(string_findNextChar(cursor, '*') == parameter1);
  TEST_ASSERT(string_findNextChar(cursor, '*') == -1);

  string_freeCursor(cursor);
  string_free(string);
}

void string_test_canGetSubstring() {
  string_t *string = string_fromBuffer("I've got a really good UDP joke to tell you, but I don't know if you'll get it");
  string_t *stringToCompare = string_substring(string, 23, 26);

  // Testing expected functionality of substring
  TEST_ASSERT(string_equalsBuffer(stringToCompare, "UDP"));

  string_free(stringToCompare);

  // Read out of bounds (Do not need test for negative numbers because of size_t)
  string_t *outOfBoundsString = string_substring(string, 0, 512);
  TEST_ASSERT_NULL(outOfBoundsString);
  if (outOfBoundsString != 0)
    string_free(outOfBoundsString);

  outOfBoundsString = string_substring(string, 512, 0);
  TEST_ASSERT_NULL(outOfBoundsString);
  if (outOfBoundsString != 0)
    string_free(outOfBoundsString);

  outOfBoundsString = string_substring(string, 512, 1024);
  TEST_ASSERT_NULL(outOfBoundsString);
  if (outOfBoundsString != 0)
    string_free(outOfBoundsString);

  string_free(string);
}

void string_test_canGetSize() {
  string_t *string = string_fromBuffer("Hello World!");

  TEST_ASSERT(string_getSize(string) == 12);

  string_free(string);
}

void string_test_canResetCursor() {
  string_t *string = string_fromBuffer("Hello World!");
  string_cursor_t *cursor = string_createCursor(string);

  // Set cursor to 12
  TEST_ASSERT(string_findNextChar(cursor, '!') == 11);
  // Reset cursor to 0
  string_resetCursor(cursor);

  // Should be able to find the same char again
  TEST_ASSERT(string_findNextChar(cursor, '!') == 11);

  string_free(string);
  string_freeCursor(cursor);
}

void string_test_canShrinkStringSize() {
  string_t *string = string_fromBuffer("Hello World!");

  string_setBufferSize(string, 4);

  TEST_ASSERT(string_equalsBuffer(string, "Hell"));
  TEST_ASSERT(string->buffer[string->size] == 0);

  string_free(string);
}

void string_test_canFromCopyOnEmpty() {
  string_t *string = string_fromBuffer("");

  TEST_ASSERT_EQUAL_STRING("", string_getBuffer(string));
  TEST_ASSERT(string_getSize(string) == 0);

  string_free(string);
}

void string_test_canAppendBufferWithLengthOfZero() {
  string_t *string = string_fromBuffer("Hello");
  string_t *buffer = string_fromBuffer("");
  size_t length = string_getSize(buffer);

  string_appendBufferWithLength(string, string_getBuffer(buffer), length);
  TEST_ASSERT_EQUAL_STRING("Hello", string_getBuffer(string));

  string_free(string);
  string_free(buffer);
}

void string_test_canCopyStringToNothing() {
  // kopiera något till en tom sträng
}

void string_test_run() {
  RUN_TEST(string_test_canCreateStringFromBuffer);
  RUN_TEST(string_test_canCreateStringFromBufferWithLength);
  RUN_TEST(string_test_canClearString);
  RUN_TEST(string_test_canCreateStringFromPositiveInt);
  RUN_TEST(string_test_canCreateStringFromNegativeInt);
  RUN_TEST(string_test_canAppendString);
  RUN_TEST(string_test_canAppendChar);
  RUN_TEST(string_test_canAppendBuffer);
  RUN_TEST(string_test_canAppendBufferWithLength);
  RUN_TEST(string_test_canGetExistingChar);
  RUN_TEST(string_test_cannotGetNonExistingChar);
  RUN_TEST(string_test_canSetExistingChar);
  RUN_TEST(string_test_cannotSetNonExistingChar);
  RUN_TEST(string_test_canTrimEnd);
  RUN_TEST(string_test_canCanCompareEqualStrings);
  RUN_TEST(string_test_canCompareNonEqualStrings);
  RUN_TEST(string_test_canCanCompareEqualStringAndBuffer);
  RUN_TEST(string_test_canCompareNonEqualStringAndBuffer);
  RUN_TEST(string_test_canLoopThroughCharacters);
  RUN_TEST(string_test_canLoopThroughLines);
  RUN_TEST(string_test_canFindNextChar);
  RUN_TEST(string_test_canGetSubstring);
  RUN_TEST(string_test_canGetSize);
  RUN_TEST(string_test_canResetCursor);
  RUN_TEST(string_test_canShrinkStringSize);
  RUN_TEST(string_test_canFromCopyOnEmpty);
  RUN_TEST(string_test_canAppendBufferWithLengthOfZero);
}
