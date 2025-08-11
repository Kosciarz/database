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
    Table* table = new_table();
    InputBuffer* input_buffer = create_input_buffer_with_data(".");
    TEST_ASSERT_EQUAL_INT(META_COMMAND_UNRECOGNIZED_COMMAND, do_meta_command(input_buffer, table));
    free_table(table);
    free_input_buffer(input_buffer);
}

static void handles_unrecognized_statement(void)
{
    Statement statement = {0};
    InputBuffer* input_buffer = create_input_buffer_with_data("");
    TEST_ASSERT_EQUAL_INT(PREPARE_UNRECOGNIZED_STATEMENT, prepare_statement(input_buffer, &statement));
    free_input_buffer(input_buffer);
}

static void handles_insert_command(void)
{
    Table* table = new_table();

    Statement statement1 = {0};
    statement1.row_to_insert.id = 1;
    strcpy(statement1.row_to_insert.username, "person1");
    strcpy(statement1.row_to_insert.email, "person1@example.com");
   
    TEST_ASSERT_EQUAL_INT(EXECUTE_SUCCESS, execute_insert(&statement1, table));
    TEST_ASSERT_EQUAL_INT(1, table->num_rows);

    Statement statement2 = {0};
    statement2.type = STATEMENT_INSERT;
    statement2.row_to_insert.id = 2;
    strcpy(statement2.row_to_insert.username, "person2");
    strcpy(statement2.row_to_insert.email, "person2@example.com");

    TEST_ASSERT_EQUAL_INT(EXECUTE_SUCCESS, execute_insert(&statement2, table));
    TEST_ASSERT_EQUAL_INT(2, table->num_rows);
    
    free_table(table);
}

static void handles_select_command(void)
{
    Table* table = new_table();

    Statement insert_statement = {0};
    insert_statement.row_to_insert.id = 1;
    strcpy(insert_statement.row_to_insert.username, "person1");
    strcpy(insert_statement.row_to_insert.email, "person1@example.com");
    execute_insert(&insert_statement, table);

    Statement select_statement = {0};

    TEST_ASSERT_EQUAL_INT(EXECUTE_SUCCESS, execute_select(&select_statement, table));

    free_table(table);
}

static void handles_delete_command(void)
{
    Table* table = new_table();

    Statement insert_statement = {0};
    insert_statement.row_to_insert.id = 1;
    strcpy(insert_statement.row_to_insert.username, "person1");
    strcpy(insert_statement.row_to_insert.email, "person1@example.com");
    execute_insert(&insert_statement, table);

    Statement delete_statement = {0};
    delete_statement.id_to_delete = 1;
    static const char test_block[ROW_SIZE] = {0};

    TEST_ASSERT_EQUAL_INT(EXECUTE_SUCCESS, execute_delete(&delete_statement, table));
    TEST_ASSERT_EQUAL_INT(0, memcmp(test_block, row_slot(table, 0), ROW_SIZE));

    free_table(table);
}

static void handles_valid_insert_input(void)
{
    Statement statement = {0};
    InputBuffer* input_buffer1 = create_input_buffer_with_data("insert 1 foo foo@foo.com");
    TEST_ASSERT_EQUAL_INT(PREPARE_SUCCESS, prepare_statement(input_buffer1, &statement));
    TEST_ASSERT_EQUAL_INT(STATEMENT_INSERT, statement.type);
    free_input_buffer(input_buffer1);
}

static void handles_valid_select_input(void)
{
    Statement statement = {0};
    InputBuffer* input_buffer1 = create_input_buffer_with_data("select");
    TEST_ASSERT_EQUAL_INT(PREPARE_SUCCESS, prepare_statement(input_buffer1, &statement));
    TEST_ASSERT_EQUAL_INT(STATEMENT_SELECT, statement.type);
    free_input_buffer(input_buffer1);
}

static void handles_valid_delete_input(void)
{
    Statement statement = {0};
    InputBuffer* input_buffer1 = create_input_buffer_with_data("delete 1");
    TEST_ASSERT_EQUAL_INT(PREPARE_SUCCESS, prepare_statement(input_buffer1, &statement));
    TEST_ASSERT_EQUAL_INT(STATEMENT_DELETE, statement.type);
    free_input_buffer(input_buffer1);
}

static void handles_maximum_insert_input_sizes(void)
{
    Table* table = new_table();

    Statement insert_statement = {0};
    insert_statement.row_to_insert.id = 1;
    memset(insert_statement.row_to_insert.username, 'a', COLUMN_USERNAME_SIZE);
    insert_statement.row_to_insert.username[COLUMN_USERNAME_SIZE] = '\0';
    memset(insert_statement.row_to_insert.email, 'a', COLUMN_EMAIL_SIZE);
    insert_statement.row_to_insert.email[COLUMN_EMAIL_SIZE] = '\0';
    execute_insert(&insert_statement, table);

    Row row = {0};
    deserialize_row(row_slot(table, 0), &row);

    TEST_ASSERT_EQUAL_INT(COLUMN_USERNAME_SIZE, strlen(row.username));
    TEST_ASSERT_EQUAL_INT(COLUMN_EMAIL_SIZE, strlen(row.email));
    
    free_table(table);
}

static void handles_missing_id_in_insert_input(void)
{
    Statement statement = {0};
    InputBuffer* input_buffer = create_input_buffer_with_data("insert person1 person1@example.com");
    TEST_ASSERT_EQUAL_INT(PREPARE_SYNTAX_ERROR, prepare_statement(input_buffer, &statement));
    free_input_buffer(input_buffer);
}

static void handles_missing_username_in_insert_input(void)
{
    Statement statement = {0};
    InputBuffer* input_buffer = create_input_buffer_with_data("insert 1 person1");
    TEST_ASSERT_EQUAL_INT(PREPARE_SYNTAX_ERROR, prepare_statement(input_buffer, &statement));
    free_input_buffer(input_buffer);
}

static void handles_missing_email_in_insert_input(void)
{
    Statement statement = {0};
    InputBuffer* input_buffer = create_input_buffer_with_data("insert 1 person1@example.com");
    TEST_ASSERT_EQUAL_INT(PREPARE_SYNTAX_ERROR, prepare_statement(input_buffer, &statement));
    free_input_buffer(input_buffer);
}

static void handles_negative_id_in_insert_input(void)
{
    Statement statement = {0};
    InputBuffer* input_buffer = create_input_buffer_with_data("insert -1 person1 person1@example.com");
    TEST_ASSERT_EQUAL_INT(PREPARE_NEGATIVE_ID, prepare_statement(input_buffer, &statement));
    free_input_buffer(input_buffer);
}

static void handles_invalid_insert_input_sizes(void)
{
    Statement statement = {0};

    char too_long_username[COLUMN_USERNAME_SIZE + 1];
    memset(too_long_username, 'a', sizeof(too_long_username));

    char too_long_email[COLUMN_EMAIL_SIZE + 1];
    memset(too_long_email, 'a', sizeof(too_long_email));

    char input[350];
    snprintf(input, sizeof(input), "insert 1 %s %s", too_long_username, too_long_email);
    
    InputBuffer* input_buffer = create_input_buffer_with_data(input);
    TEST_ASSERT_EQUAL_INT(PREPARE_STRING_TOO_LONG, prepare_statement(input_buffer, &statement));
    free_input_buffer(input_buffer);
}

static void handles_missing_id_in_delete_input(void)
{
    Statement statement = {0};
    InputBuffer* input_buffer = create_input_buffer_with_data("delete");
    TEST_ASSERT_EQUAL_INT(PREPARE_SYNTAX_ERROR, prepare_statement(input_buffer, &statement));
    free_input_buffer(input_buffer);
}

static void handles_negative_id_in_delete_input(void)
{
    Statement statement = {0};
    InputBuffer* input_buffer = create_input_buffer_with_data("delete -1");
    TEST_ASSERT_EQUAL_INT(PREPARE_NEGATIVE_ID, prepare_statement(input_buffer, &statement));
    free_input_buffer(input_buffer);
}

static void handles_invalid_id_in_delete_command(void)
{
    Table* table = new_table();
    Statement statement = {0};
    statement.type = STATEMENT_DELETE;
    statement.id_to_delete = 1;

    TEST_ASSERT_EQUAL_INT(EXECUTE_ID_NOT_FOUND, execute_statement(&statement, table));
    free_table(table);
}

static ExecuteResult insert_row(Table* table, int n)
{
    Statement statement = {0};
    statement.type = STATEMENT_INSERT;
    statement.row_to_insert.id = n;
    snprintf(statement.row_to_insert.username, sizeof(statement.row_to_insert.username), "person%d", n);
    snprintf(statement.row_to_insert.email, sizeof(statement.row_to_insert.email), "person%d@example.com", n);
    return execute_statement(&statement, table);
}

static void handles_inserting_when_table_is_full(void)
{
    Table* table = new_table();

    for (int i = 1; i <= TABLE_MAX_ROWS; ++i)
        insert_row(table, i);

    TEST_ASSERT_EQUAL_INT(EXECUTE_TABLE_FULL, insert_row(table, TABLE_MAX_ROWS + 1));
    TEST_ASSERT_EQUAL_INT(TABLE_MAX_ROWS, table->num_rows);

    free_table(table);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(handles_unrecognized_meta_command);

    RUN_TEST(handles_unrecognized_statement);
    RUN_TEST(handles_insert_command);
    RUN_TEST(handles_select_command);
    RUN_TEST(handles_delete_command);

    RUN_TEST(handles_valid_insert_input);
    RUN_TEST(handles_valid_select_input);
    RUN_TEST(handles_valid_delete_input);

    RUN_TEST(handles_missing_id_in_insert_input);
    RUN_TEST(handles_missing_username_in_insert_input);
    RUN_TEST(handles_missing_email_in_insert_input);
    RUN_TEST(handles_negative_id_in_insert_input);
    RUN_TEST(handles_maximum_insert_input_sizes);
    RUN_TEST(handles_invalid_insert_input_sizes);

    RUN_TEST(handles_missing_id_in_delete_input);
    RUN_TEST(handles_negative_id_in_delete_input);
    RUN_TEST(handles_invalid_id_in_delete_command);

    RUN_TEST(handles_inserting_when_table_is_full);
    return UNITY_END();
}
