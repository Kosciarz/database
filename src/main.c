#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "input.h"
#include "parser.h"


static void print_prompt(void)
{
    printf("database> ");
}

int main(void)
{
    Table* table = new_table();
    InputBuffer* input_buffer = new_input_buffer();

    while (true)
    {
        print_prompt();
        read_input(input_buffer);

        if (input_buffer->buffer[0] == '.')
        {
            switch (do_meta_command(input_buffer, table))
            {
            case META_COMMAND_SUCCESS:
                continue;
            case META_COMMAND_UNRECOGNIZED_COMMAND:
                fprintf(stderr, "Unrecognized command '%s'\n", input_buffer->buffer);
                continue;
            }
        }

        Statement statement = {0};
        switch (prepare_statement(input_buffer, &statement))
        {
        case PREPARE_SUCCESS:
            break;
        case PREPARE_NEGATIVE_ID:
            fprintf(stderr, "Error: ID must be positive.\n");
            break;
        case PREPARE_STRING_TOO_LONG:
            fprintf(stderr, "Error: String is too long.\n");
            break;
        case PREPARE_SYNTAX_ERROR:
            fprintf(stderr, "Error: Could not parse statement.\n");
            continue;
        case PREPARE_UNRECOGNIZED_STATEMENT:
            fprintf(stderr, "Error: Unrecongnized keyword at start of '%s'.\n", input_buffer->buffer);
            continue;
        }

        switch (execute_statement(&statement, table))
        {
        case EXECUTE_SUCCESS:
            printf("Executed.\n");
            break;
        case EXECUTE_TABLE_FULL:
            fprintf(stderr, "Error: Table full.\n");
            break;
        case EXECUTE_ID_NOT_FOUND:
            fprintf(stderr, "Error: Row id %d not found.\n", statement.id_to_delete);
            break;
        }
    }
}
