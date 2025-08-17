#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>

#include "input.h"
#include "table.h"


typedef enum
{
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

MetaCommandResult do_meta_command(InputBuffer* input_buffer, Table* table);


typedef enum
{
    PREPARE_SUCCESS,
    PREPARE_NEGATIVE_ID,
    PREPARE_STRING_TOO_LONG,
    PREPARE_UNRECOGNIZED_STATEMENT,
    PREPARE_SYNTAX_ERROR
} PrepareResult;

typedef enum
{
    STATEMENT_SELECT,
    STATEMENT_INSERT,
    STATEMENT_DELETE
} StatementType;

typedef struct
{
    StatementType type;
    Row row_to_insert;
    uint32_t id_to_delete;
} Statement;

PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement);


typedef enum
{
    EXECUTE_SUCCESS,
    EXECUTE_TABLE_FULL,
    EXECUTE_ID_NOT_FOUND
} ExecuteResult;

ExecuteResult execute_statement(Statement* statement, Table* table);


#endif // PARSER_H
