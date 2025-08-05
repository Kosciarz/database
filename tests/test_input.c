#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <unity.h>

#include "input.h"

void setUp(void)
{
}

void tearDown(void)
{
}

static void test_new_buffer_is_empty(void)
{
    InputBuffer* input_buffer = new_input_buffer();

    TEST_ASSERT_NULL(input_buffer->buffer);
    TEST_ASSERT_EQUAL_INT(input_buffer->buffer_length, 0);
    TEST_ASSERT_EQUAL_INT(input_buffer->input_length, 0);

    close_input_buffer(input_buffer);
}

// Mock getline for testing
static ssize_t mock_getline(char **lineptr, size_t *n, FILE *stream) {
    const char *test_input = "test input";
    size_t len = strlen(test_input);

    if (*lineptr == NULL || *n < len + 1) {
        *n = len + 1;
        *lineptr = realloc(*lineptr, *n);
    }
    strncpy(*lineptr, test_input, len + 1);
    return (ssize_t)len;
}

// Redefine getline to our mock
#define getline mock_getline

static void test_reads_input(void) {
    InputBuffer* input_buffer = new_input_buffer();
    read_input(input_buffer);

    TEST_ASSERT_EQUAL_STRING(input_buffer->buffer, "test input");
    TEST_ASSERT_EQUAL_INT(input_buffer->input_length, 11);

    close_input_buffer(input_buffer);
}

int main(void)
{
	UNITY_BEGIN();
    RUN_TEST(test_new_buffer_is_empty);
    RUN_TEST(test_reads_input);
	return UNITY_END();
}
