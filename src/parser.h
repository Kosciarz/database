#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>

#include "input.h"

typedef enum {
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;


typedef enum {
    PREPARE_SUCCESS,
    PREPARE_UNRECOGNIZED_STATEMENT,
    PREPARE_SYNTAX_ERROR
} PrepareResult;

typedef enum {
    STATEMENT_SELECT,
    STATEMENT_INSERT
} StatementType;

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255

typedef struct {
    uint32_t id;
    char username[COLUMN_USERNAME_SIZE];
    char email[COLUMN_EMAIL_SIZE];
} Row;

#define SIZE_OF_ATTRIBUTE(struct, attribute) sizeof(((struct*)0)->attribute)

#define ID_SIZE SIZE_OF_ATTRIBUTE(Row, id)
#define USERNAME_SIZE SIZE_OF_ATTRIBUTE(Row, username)
#define EMAIL_SIZE SIZE_OF_ATTRIBUTE(Row, email)
#define ID_OFFSET 0
#define USERNAME_OFFSET ID_OFFSET + ID_SIZE
#define EMAIL_OFFSET USERNAME_OFFSET + USERNAME_SIZE
#define ROW_SIZE ID_SIZE + USERNAME_SIZE + EMAIL_SIZE

void serialize_row(Row* source, void* destination);
void deserialize_row(void* source, Row* destination);

#define PAGE_SIZE 4096
#define TABLE_MAX_PAGES 100
#define ROWS_PER_PAGE PAGE_SIZE / ROW_SIZE
#define TABLE_MAX_ROWS ROWS_PER_PAGE * TABLE_MAX_PAGES

typedef struct {
    uint32_t num_rows;
    void* pages[TABLE_MAX_PAGES];
} Table;

void* row_slot(Table* table, uint32_t row_num);

typedef struct {
    StatementType type;
    Row row_to_insert;
} Statement;

MetaCommandResult do_meta_command(InputBuffer* input_buffer);
PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement);
void execute_statement(Statement* statement);

#endif // PARSER_H
