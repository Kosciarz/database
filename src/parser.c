#include "parser.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

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
    for (uint32_t i = 0; table->pages[i] != NULL; ++i)
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

MetaCommandResult do_meta_command(InputBuffer* input_buffer)
{
    if (strcmp(input_buffer->buffer, ".exit") == 0)
    {
        free_input_buffer(input_buffer);
        exit(EXIT_SUCCESS);
    }
    else
    {
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}

PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement)
{
    if (strncmp(input_buffer->buffer, "insert", 6) == 0)
    {
        statement->type = STATEMENT_INSERT;
        const int args_assigned = sscanf(input_buffer->buffer, "insert %d %32s %255s",
            (int*)(&(statement->row_to_insert.id)), statement->row_to_insert.username, statement->row_to_insert.email);
        return args_assigned < 3 ? PREPARE_SYNTAX_ERROR : PREPARE_SUCCESS;
    }
    if (strcmp(input_buffer->buffer, "select") == 0)
    {
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }
    return PREPARE_UNRECOGNIZED_STATEMENT;
}

ExecuteResult execute_statement(Statement* statement, Table* table)
{
    switch (statement->type)
    {
    case STATEMENT_INSERT:
        return execute_insert(statement, table);
    case STATEMENT_SELECT:
        return execute_select(statement, table);
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

ExecuteResult execute_select(Statement* statement, Table* table)
{
    Row row;
    for (uint32_t i = 0; i < table->num_rows; ++i)
    {
        deserialize_row(row_slot(table, i), &row);
        print_row(&row);
    }
    return EXECUTE_SUCCESS;
}
