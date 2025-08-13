#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "input.h"


static PrepareResult prepare_insert(InputBuffer* input_buffer, Statement* statement);
static PrepareResult prepare_select(InputBuffer* input_buffer, Statement* statement);
static PrepareResult prepare_delete(InputBuffer* input_buffer, Statement* statement);

static ExecuteResult execute_insert(Statement* statement, Table* table);
static ExecuteResult execute_select(Statement* statement, Table* table);
static ExecuteResult execute_delete(Statement* statement, Table* table);

static bool is_valid_row(void* row);
static void print_row(Row* row);


MetaCommandResult do_meta_command(InputBuffer* input_buffer, Table* table)
{
    if (strcmp(input_buffer->buffer, ".exit") == 0)
    {
        free_input_buffer(input_buffer);
        db_close(table);
        exit(EXIT_SUCCESS);
    }
    return META_COMMAND_UNRECOGNIZED_COMMAND;
}

PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement)
{
    if (strncmp(input_buffer->buffer, "insert", 6) == 0)
        return prepare_insert(input_buffer, statement);

    if (strcmp(input_buffer->buffer, "select") == 0)
        return prepare_select(input_buffer, statement);

    if (strncmp(input_buffer->buffer, "delete", 6) == 0)
        return prepare_delete(input_buffer, statement);

    return PREPARE_UNRECOGNIZED_STATEMENT;
}

static PrepareResult prepare_insert(InputBuffer* input_buffer, Statement* statement)
{
    strtok(input_buffer->buffer, " ");
    char* id_string = strtok(NULL, " ");
    char* username = strtok(NULL, " ");
    char* email = strtok(NULL, " ");

    if (!id_string || !username || !email)
        return PREPARE_SYNTAX_ERROR;

    int id = atoi(id_string);

    if (id < 0)
        return PREPARE_NEGATIVE_ID;
    
    if (strlen(username) > COLUMN_USERNAME_SIZE || strlen(email) > COLUMN_EMAIL_SIZE)
        return PREPARE_STRING_TOO_LONG;
    
    statement->type = STATEMENT_INSERT;
    statement->row_to_insert.id = id;
    strcpy(statement->row_to_insert.username, username);
    strcpy(statement->row_to_insert.email, email);

    return PREPARE_SUCCESS;
}

static PrepareResult prepare_select(InputBuffer* input_buffer, Statement* statement)
{
    statement->type = STATEMENT_SELECT;
    return PREPARE_SUCCESS;
}

static PrepareResult prepare_delete(InputBuffer* input_buffer, Statement* statement)
{
    strtok(input_buffer->buffer, " ");
    char* id_string = strtok(NULL, " ");

    if (!id_string)
        return PREPARE_SYNTAX_ERROR;

    int id = atoi(id_string);

    if (id < 0)
        return PREPARE_NEGATIVE_ID;

    statement->type = STATEMENT_DELETE;
    statement->id_to_delete = id;
    return PREPARE_SUCCESS;
}

ExecuteResult execute_statement(Statement* statement, Table* table)
{
    switch (statement->type)
    {
    case STATEMENT_INSERT:
        return execute_insert(statement, table);
    case STATEMENT_SELECT:
        return execute_select(statement, table);
    case STATEMENT_DELETE:
        return execute_delete(statement, table);
    }
    return 0;
}

static ExecuteResult execute_insert(Statement* statement, Table* table)
{
    if (table->num_rows >= TABLE_MAX_ROWS)
        return EXECUTE_TABLE_FULL;

    serialize_row(&(statement->row_to_insert), row_slot(table, table->num_rows));
    table->num_rows++;
    return EXECUTE_SUCCESS;
}

static ExecuteResult execute_select(Statement* statement, Table* table)
{
    Row row;
    for (uint32_t i = 0; i < table->num_rows; ++i)
    {
        if (is_valid_row(row_slot(table, i)))
        {
            deserialize_row(row_slot(table, i), &row);
            print_row(&row);
        }
    }
    return EXECUTE_SUCCESS;
}

static ExecuteResult execute_delete(Statement* statement, Table* table)
{
    Row row;
    for (uint32_t i = 0; i < table->num_rows; ++i)
    {
        deserialize_row(row_slot(table, i), &row);
        if (row.id == statement->id_to_delete)
        {
            memset(row_slot(table, i), 0, ROW_SIZE);
            return EXECUTE_SUCCESS;
        }
    }
    return EXECUTE_ID_NOT_FOUND;
}

static bool is_valid_row(void* row)
{
    static const char test_block[ROW_SIZE] = {0};
    return memcmp(test_block, row, ROW_SIZE);
}

static void print_row(Row* row)
{
    printf("(%d, %s, %s)\n", row->id, row->username, row->email);
}
