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

	free_input_buffer(input_buffer);
}

// redefining the function overshadows the getline.c definition
ssize_t getline(char** lineptr, size_t* n, FILE* stream)
{
	const char* test_input = "test input\n";
	size_t len = strlen(test_input);

	if (*lineptr == NULL || *n < len + 1)
	{
		*n = len + 1;
		*lineptr = realloc(*lineptr, *n);
	}
	strncpy(*lineptr, test_input, len + 1);
	return (ssize_t)len;
}

static void test_reads_input(void) {
	InputBuffer* input_buffer = new_input_buffer();
	read_input(input_buffer);

	TEST_ASSERT_EQUAL_STRING("test input", input_buffer->buffer);
	TEST_ASSERT_EQUAL_INT(10, input_buffer->input_length);

	free_input_buffer(input_buffer);
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_new_buffer_is_empty);
	RUN_TEST(test_reads_input);
	return UNITY_END();
}
