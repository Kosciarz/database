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
    InputBuffer* input_buffer = new_input_buffer();
    while (true)
    {
        print_prompt();
        read_input(input_buffer);

        if (input_buffer->buffer[0] == '.')
        {
            switch (do_meta_command(input_buffer))
            {
            case META_COMMAND_SUCCESS:
                continue;
            case META_COMMAND_UNRECOGNIZED_COMMAND:
                fprintf(stderr, "Unrecognized command '%s'\n", input_buffer->buffer);
                continue;
            }
        }

        Statement statement;
        switch (prepare_statement(input_buffer, &statement))
        {
        case PREPARE_SUCCESS:
            break;
        case PREPARE_UNRECOGNIZED_STATEMENT:
            fprintf(stderr, "Unrecongnized keyword at start of '%s'\n", input_buffer->buffer);
            continue;
        case PREPARE_SYNTAX_ERROR:
            fprintf(stderr, "Syntax error in the command '%s'\n", input_buffer->buffer);
            continue;
        }

        execute_statement(&statement);
        printf("Executed.\n");
    }
}
