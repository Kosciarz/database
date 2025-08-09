#include "parser.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "input.h"

void serialize_row(Row* source, void* destination)
{
    memcpy((uint8_t*)(destination) + ID_OFFSET, &(source->id), ID_SIZE);
    memcpy((uint8_t*)(destination) + USERNAME_OFFSET, &(source->username), USERNAME_SIZE);
    memcpy((uint8_t*)(destination) + USERNAME_OFFSET, &(source->email), EMAIL_SIZE);
}

void deserialize_row(void* source, Row* destination)
{
    memcpy(&(destination->id), (uint8_t*)(source) + ID_OFFSET, ID_SIZE);
    memcpy(&(destination->username), (uint8_t*)(source) + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&(destination->email), (uint8_t*)(source) + EMAIL_OFFSET, EMAIL_SIZE);
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
    return (uint8_t*)(page) + byte_offset;
}

MetaCommandResult do_meta_command(InputBuffer* input_buffer)
{
    if (strcmp(input_buffer->buffer, ".exit") == 0)
    {
        close_input_buffer(input_buffer);
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

void execute_statement(Statement* statement)
{
    switch (statement->type)
    {
    case STATEMENT_INSERT:
        printf("This is where we would do an insert.\n");
        break;
    case STATEMENT_SELECT:
        printf("This is where we would do a select.\n");
        break;
    }
}
