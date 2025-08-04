#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>

#include <unity.h>

#include "getline.h"

void setUp(void)
{
}

void tearDown(void)
{
}

static void test_handles_null_values(void)
{
}

static void test_reads_line_from_stream(void)
{
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_handles_null_values);
    RUN_TEST(test_reads_line_from_stream);
    return UNITY_END();
}
