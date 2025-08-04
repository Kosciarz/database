#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>

#include <unity.h>

void setUp(void)
{
}

void tearDown(void)
{
}

static int add(int a, int b)
{
	return a + b;
}

static void test_adding_numbers_returns_sum(void)
{
	TEST_ASSERT_EQUAL_INT(add(2, 3), 5);
	TEST_ASSERT_EQUAL_INT(add(-2, -3), -5);
}

static void test_adding_zero_and_number_returns_number(void)
{
	TEST_ASSERT_EQUAL_INT(add(0, 3), 3);
	TEST_ASSERT_EQUAL_INT(add(3, 0), 3);
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_adding_numbers_returns_sum);
	RUN_TEST(test_adding_zero_and_number_returns_number);
	return UNITY_END();
}
