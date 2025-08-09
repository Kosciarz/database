#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>

#include <unity.h>

#include "parser.h"
#include "input.h"

void setUp(void)
{
}

void tearDown(void)
{
}

static InputBuffer* create_input_buffer_with_data(const char* data)
{
    InputBuffer* input_buffer = new_input_buffer();
    size_t len = strlen(data);
    input_buffer->buffer = malloc(len + 1);
    if (!input_buffer->buffer)
    {
        perror("Malloc failed");
        exit(EXIT_FAILURE);
    }

    memcpy(input_buffer->buffer, data, len);
    input_buffer->buffer[len] = '\0';
    input_buffer->buffer_length = len + 1;
    input_buffer->input_length = len;
    return input_buffer;
}

static void handles_unrecognized_meta_command(void)
{
    InputBuffer* input_buffer = create_input_buffer_with_data("");
    Table* table = new_table();

    TEST_ASSERT_EQUAL_INT(META_COMMAND_UNRECOGNIZED_COMMAND, do_meta_command(input_buffer, table));

    free_input_buffer(input_buffer);
    free(table);
}

static void handles_unrecognized_statement(void)
{
    InputBuffer* input_buffer = create_input_buffer_with_data("");
    Statement statement;

    TEST_ASSERT_EQUAL_INT(PREPARE_UNRECOGNIZED_STATEMENT, prepare_statement(input_buffer, &statement));

    free_input_buffer(input_buffer);
}

static void handles_valid_statement(void)
{
    Statement statement;

    InputBuffer* input_buffer1 = create_input_buffer_with_data("insert 1 foo foo@foo.com");
    TEST_ASSERT_EQUAL_INT(PREPARE_SUCCESS, prepare_statement(input_buffer1, &statement));
    TEST_ASSERT_EQUAL_INT(STATEMENT_INSERT, statement.type);
    free_input_buffer(input_buffer1);

    InputBuffer* input_buffer2 = create_input_buffer_with_data("select");
    TEST_ASSERT_EQUAL_INT(PREPARE_SUCCESS, prepare_statement(input_buffer2, &statement));
    TEST_ASSERT_EQUAL_INT(STATEMENT_SELECT, statement.type);
    free_input_buffer(input_buffer2);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(handles_unrecognized_meta_command);
    RUN_TEST(handles_unrecognized_statement);
    RUN_TEST(handles_valid_statement);
    return UNITY_END();
}
