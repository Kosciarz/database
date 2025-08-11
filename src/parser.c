#include "parser.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "input.h"

void serialize_row(Row* source, void* destination)
{
    memcpy((uint8_t*)(destination) + ID_OFFSET, &(source->id), ID_SIZE);
    memcpy((uint8_t*)(destination) + USERNAME_OFFSET, &(source->username), USERNAME_SIZE);
    memcpy((uint8_t*)(destination) + EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
}

void deserialize_row(void* source, Row* destination)
{
    memcpy(&(destination->id), (uint8_t*)(source) + ID_OFFSET, ID_SIZE);
    memcpy(&(destination->username), (uint8_t*)(source) + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&(destination->email), (uint8_t*)(source) + EMAIL_OFFSET, EMAIL_SIZE);
}

Table* new_table(void)
{
    Table* table = malloc(sizeof(Table));
    if (!table)
    {
        perror("Malloc failed");
        exit(EXIT_FAILURE);
    }

    table->num_rows = 0;
    for (uint32_t i = 0; i < TABLE_MAX_PAGES; ++i)
        table->pages[i] = NULL;

    return table;
}

void free_table(Table* table)
{
    for (uint32_t i = 0; i < TABLE_MAX_PAGES; ++i)
        if (table->pages[i] != NULL)
            free(table->pages[i]);
    free(table);
}

void* row_slot(Table* table, uint32_t row_num)
{
    uint32_t page_num = row_num / ROWS_PER_PAGE;
    void* page = table->pages[page_num];
    if (!page)
    {
        page = table->pages[page_num] = malloc(PAGE_SIZE);
        if (!page)
        {
            perror("Malloc failed");
            exit(EXIT_FAILURE);
        }
    }

    uint32_t row_offset = row_num % ROWS_PER_PAGE;
    uint32_t byte_offset = row_offset * ROW_SIZE;
    return (uint8_t*)(page)+byte_offset;
}

static void print_row(Row* row)
{
    printf("(%d, %s, %s)\n", row->id, row->username, row->email);
}

MetaCommandResult do_meta_command(InputBuffer* input_buffer, Table* table)
{
    if (strcmp(input_buffer->buffer, ".exit") == 0)
    {
        free_input_buffer(input_buffer);
        free_table(table);
        exit(EXIT_SUCCESS);
    }
    else
    {
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}

static PrepareResult prepare_insert(InputBuffer* input_buffer, Statement* statement);
static PrepareResult prepare_delete(InputBuffer* input_buffer, Statement* statement);

PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement)
{
    if (strncmp(input_buffer->buffer, "insert", 6) == 0)
    {
        return prepare_insert(input_buffer, statement);
    }
    if (strcmp(input_buffer->buffer, "select") == 0)
    {
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }
    if (strncmp(input_buffer->buffer, "delete", 6) == 0)
    {
        return prepare_delete(input_buffer, statement);
    }

    return PREPARE_UNRECOGNIZED_STATEMENT;
}

static PrepareResult prepare_insert(InputBuffer* input_buffer, Statement* statement)
{
    char* keyword = strtok(input_buffer->buffer, " ");
    char* id_string = strtok(NULL, " ");
    char* username = strtok(NULL, " ");
    char* email = strtok(NULL, " ");

    if (!id_string || !username || !email)
        return PREPARE_SYNTAX_ERROR;

    int id = atoi(id_string);

    if (id < 0)
        return PREPARE_NEGATIVE_ID;
    if (strlen(username) > COLUMN_USERNAME_SIZE)
        return PREPARE_STRING_TOO_LONG;
    if (strlen(email) > COLUMN_EMAIL_SIZE)
        return PREPARE_STRING_TOO_LONG;

    statement->type = STATEMENT_INSERT;
    statement->row_to_insert.id = id;
    strcpy(statement->row_to_insert.username, username);
    strcpy(statement->row_to_insert.email, email);

    return PREPARE_SUCCESS;
}

static PrepareResult prepare_delete(InputBuffer* input_buffer, Statement* statement)
{
    char* keyword = strtok(input_buffer->buffer, " ");
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

ExecuteResult execute_insert(Statement* statement, Table* table)
{
    if (table->num_rows >= TABLE_MAX_ROWS)
        return EXECUTE_TABLE_FULL;

    Row* row_to_insert = &(statement->row_to_insert);
    serialize_row(row_to_insert, row_slot(table, table->num_rows));
    table->num_rows++;
    return EXECUTE_SUCCESS;
}

static bool is_valid_row(void* row)
{
    static const char test_block[ROW_SIZE] = {0};
    return memcmp(test_block, row, ROW_SIZE);
}

ExecuteResult execute_select(Statement* statement, Table* table)
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

ExecuteResult execute_delete(Statement* statement, Table* table)
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
