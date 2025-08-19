#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#include <unity.h>

#include "table.h"
#include "parser.h"
#include "input.h"


void setUp(void)
{
}

void tearDown(void)
{
}

static void handles_unrecognized_meta_command(void)
{
#ifdef _WIN32
	char temp_path[MAX_PATH];
	char temp_file_name[MAX_PATH];

	if (!GetTempPathA(MAX_PATH, temp_path))
	{
		perror("GetTempPath error");
		exit(EXIT_FAILURE);
	}

	if (!GetTempFileNameA(temp_path, "tmpfile", 0, temp_file_name))
	{
		perror("GetTempFileName error");
		exit(EXIT_FAILURE);
	}
#endif

	Table* table = db_open(temp_file_name);

	InputBuffer* input_buffer = new_input_buffer();
	const char* data = ".";
	size_t len = strlen(data);
	input_buffer->buffer = malloc(len + 1);
	if (!input_buffer->buffer)
	{
		perror("malloc error");
		exit(EXIT_FAILURE);
	}

	memcpy(input_buffer->buffer, data, len);
	input_buffer->buffer[len] = '\0';
	
	
	TEST_ASSERT_EQUAL_INT(META_COMMAND_UNRECOGNIZED_COMMAND, do_meta_command(input_buffer, table));
	
	db_close(table);
	free_input_buffer(input_buffer);
}

static void defines_correct_constants(void)
{
	TEST_ASSERT_EQUAL_INT(293, ROW_SIZE);
	TEST_ASSERT_EQUAL_INT(6, COMMON_NODE_HEADER_SIZE);
	TEST_ASSERT_EQUAL_INT(10, LEAF_NODE_HEADER_SIZE);
	TEST_ASSERT_EQUAL_INT(297, LEAF_NODE_CELL_SIZE);
	TEST_ASSERT_EQUAL_INT(4086, LEAF_NODE_SPACE_FOR_CELLS);
	TEST_ASSERT_EQUAL_INT(13, LEAF_NODE_MAX_CELLS);
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(handles_unrecognized_meta_command);
	RUN_TEST(defines_correct_constants);
	return UNITY_END();
}
