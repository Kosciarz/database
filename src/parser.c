#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "input.h"


static void print_constants(void);
static void indent(uint32_t level);
static void print_tree(Pager* pager, uint32_t page_num, uint32_t indent_level);

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
	if (strcmp(input_buffer->buffer, ".constants") == 0)
	{
		printf("Constants:\n");
		print_constants();
		return META_COMMAND_SUCCESS;
	}
	if (strcmp(input_buffer->buffer, ".btree") == 0)
	{
		printf("Tree:\n");
		print_tree(table->pager, 0, 0);
		return META_COMMAND_SUCCESS;
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
	void* node = get_page(table->pager, table->root_page_num);
	uint32_t num_cells = *leaf_node_num_cells(node);

	Row* row_to_insert = &(statement->row_to_insert);
	uint32_t key_to_insert = row_to_insert->id;
	Cursor* cursor = table_find(table, key_to_insert);

	if (cursor->cell_num < num_cells)
	{
		uint32_t key_at_index = *leaf_node_key(node, cursor->cell_num);
		if (key_at_index == key_to_insert)
			return EXECUTE_DUPLICATE_KEY;
	}

	leaf_node_insert(cursor, row_to_insert->id, row_to_insert);
	free(cursor);
	return EXECUTE_SUCCESS;
}

static ExecuteResult execute_select(Statement* statement, Table* table)
{
	Cursor* cursor = table_start(table);
	Row row;

	while (!cursor->end_of_table)
	{
		deserialize_row(cursor_value(cursor), &row);
		if (is_valid_row(&row))
			print_row(&row);

		cursor_advance(cursor);
	}

	free(cursor);
	return EXECUTE_SUCCESS;
}

static ExecuteResult execute_delete(Statement* statement, Table* table)
{
	Cursor* cursor = table_start(table);
	Row row;

	while (!cursor->end_of_table)
	{
		deserialize_row(cursor_value(cursor), &row);
		if (row.id == statement->id_to_delete)
		{
			memset(cursor_value(cursor), 0, ROW_SIZE);
			free(cursor);
			return EXECUTE_SUCCESS;
		}
		cursor_advance(cursor);
	}

	free(cursor);
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


static void print_constants(void)
{
	printf("ROW_SIZE: %lld\n", ROW_SIZE);
	printf("COMMON_NODE_HEADER_SIZE: %lld\n", COMMON_NODE_HEADER_SIZE);
	printf("LEAF_NODE_HEADER_SIZE: %lld\n", LEAF_NODE_HEADER_SIZE);
	printf("LEAF_NODE_CELL_SIZE: %lld\n", LEAF_NODE_CELL_SIZE);
	printf("LEAF_NODE_SPACE_FOR_CELLS: %lld\n", LEAF_NODE_SPACE_FOR_CELLS);
	printf("LEAF_NODE_MAX_CELLS: %lld\n", LEAF_NODE_MAX_CELLS);
}

void indent(uint32_t level)
{
	for (uint32_t i = 0; i < level; ++i)
		printf("  ");
}

void print_tree(Pager* pager, uint32_t page_num, uint32_t indent_level)
{
	void* node = get_page(pager, page_num);
	uint8_t num_keys;
	uint32_t child;

	switch (get_node_type(node))
	{
	case NODE_LEAF:
	{
		num_keys = *leaf_node_num_cells(node);
		indent(indent_level);
		printf("- leaf (size %d)\n", num_keys);
		for (uint32_t i = 0; i < num_keys; ++i)
		{
			indent(indent_level + 1);
			printf("- %d\n", *leaf_node_key(node, i));
		}
		break;
	}
	case NODE_INTERNAL:
	{
		num_keys = *internal_node_num_keys(node);
		indent(indent_level);
		printf("- internal (size %d)\n", num_keys);
		for (uint32_t i = 0; i < num_keys; ++i)
		{
			child = *internal_node_child(node, i);
			print_tree(pager, child, indent_level + 1);

			indent(indent_level + 1);
			printf("- key %d\n", *internal_node_key(node, i));
		}
		child = *internal_node_right_child(node);
		print_tree(pager, child, indent_level + 1);
		break;
	}
	}
}
