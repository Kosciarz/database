#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#include <unity.h>

#include "getline.h"
#include "input.h"

void setUp(void)
{
}

void tearDown(void)
{
}

static void test_handles_null_values(void)
{
    char* buffer = malloc(3);
    size_t n = 0;

    TEST_ASSERT_EQUAL_INT(-1, getline(NULL, &n, stdin));
    TEST_ASSERT_EQUAL_INT(-1, getline(&buffer, NULL, stdin));
    TEST_ASSERT_EQUAL_INT(-1, getline(&buffer, &n, NULL));
}

static void test_reads_line_from_stream(void)
{
#ifdef _WIN32
    char temp_path[MAX_PATH];
    char temp_file_name[MAX_PATH];

    if (!GetTempPathA(MAX_PATH, temp_path))
    {
        fprintf(stderr, "GetTempPathA failed\n");
        exit(EXIT_FAILURE);
    }

    if (!GetTempFileNameA(temp_path, "tmpfile", 0, temp_file_name))
    {
        fprintf(stderr, "GetTempFileNameA failed\n");
        exit(EXIT_FAILURE);
    }
#endif

    FILE* temp_file = fopen(temp_file_name, "w+");
    if (!temp_file)
    {
        perror("Failed to open temp file:");
        exit(EXIT_FAILURE);
    }

    fprintf(temp_file, "test input\n");
    rewind(temp_file);

    char* buffer = NULL;
    size_t buffer_length;
    ssize_t bytes_read = getline(&buffer, &buffer_length, temp_file);

    TEST_ASSERT_NOT_EQUAL_INT(-1, bytes_read);
    TEST_ASSERT_NOT_NULL(buffer);
    TEST_ASSERT_EQUAL_STRING("test input\n", buffer);
    TEST_ASSERT_EQUAL_INT(11, bytes_read);

    fclose(temp_file);
    remove(temp_file_name);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_handles_null_values);
    RUN_TEST(test_reads_line_from_stream);
    return UNITY_END();
}
