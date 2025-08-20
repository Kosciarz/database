#ifndef TABLE_H
#define TABLE_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>


#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255

#define SIZE_OF_ATTRIBUTE(struct, attribute) sizeof(((struct*)0)->attribute)

#define ID_SIZE SIZE_OF_ATTRIBUTE(Row, id)
#define USERNAME_SIZE SIZE_OF_ATTRIBUTE(Row, username)
#define EMAIL_SIZE SIZE_OF_ATTRIBUTE(Row, email)
#define ID_OFFSET 0
#define USERNAME_OFFSET (ID_OFFSET + ID_SIZE)
#define EMAIL_OFFSET (USERNAME_OFFSET + USERNAME_SIZE)
#define ROW_SIZE (ID_SIZE + USERNAME_SIZE + EMAIL_SIZE)

#define PAGE_SIZE 4096
#define TABLE_MAX_PAGES 100


typedef struct
{
	uint32_t id;
	char username[COLUMN_USERNAME_SIZE + 1];
	char email[COLUMN_EMAIL_SIZE + 1];
} Row;

void serialize_row(Row* source, void* destination);
void deserialize_row(void* source, Row* destination);


typedef struct
{
	FILE* file_ptr;
	uint32_t file_length;
	uint32_t num_pages;
	void* pages[TABLE_MAX_PAGES];
} Pager;

Pager* pager_open(const char* filename);
void* get_page(Pager* pager, uint32_t page_num);
void pager_flush(Pager* pager, uint32_t page_num);
uint32_t get_unused_page_num(Pager* pager);


typedef struct
{
	Pager* pager;
	uint32_t root_page_num;
} Table;

Table* db_open(const char* filename);
void db_close(Table* table);


typedef struct
{
	Table* table;
	uint32_t page_num;
	uint32_t cell_num;
	bool end_of_table;
} Cursor;

Cursor* table_start(Table* table);
Cursor* table_find(Table* table, uint32_t key);
void* cursor_value(Cursor* cursor);
void cursor_advance(Cursor* cursor);


// B-Tree implementation

typedef enum
{
	NODE_INTERNAL,
	NODE_LEAF
} NodeType;

// Common Node Layout

#define NODE_TYPE_SIZE sizeof(uint8_t)
#define NODE_TYPE_OFFSET 0
#define IS_ROOT_SIZE sizeof(uint8_t)
#define IS_ROOT_OFFSET IS_ROOT_SIZE
#define PARENT_POINTER_SIZE sizeof(uint32_t)
#define PARENT_POINTER_OFFSET (IS_ROOT_OFFSET + IS_ROOT_SIZE)
#define COMMON_NODE_HEADER_SIZE (NODE_TYPE_SIZE + IS_ROOT_SIZE + PARENT_POINTER_SIZE)

// Leaf Node Layout

#define LEAF_NODE_NUM_CELLS_SIZE sizeof(uint32_t)
#define LEAF_NODE_NUM_CELLS_OFFSET COMMON_NODE_HEADER_SIZE
#define LEAF_NODE_HEADER_SIZE (COMMON_NODE_HEADER_SIZE + LEAF_NODE_NUM_CELLS_SIZE)

// Leaf Node Body Layout

#define LEAF_NODE_KEY_SIZE sizeof(uint32_t)
#define LEAF_NODE_KEY_OFFSET 0
#define LEAF_NODE_VALUE_SIZE ROW_SIZE
#define LEAF_NODE_VALUE_OFFSET (LEAF_NODE_KEY_OFFSET + LEAF_NODE_VALUE_SIZE)
#define LEAF_NODE_CELL_SIZE (LEAF_NODE_KEY_SIZE + LEAF_NODE_VALUE_SIZE)
#define LEAF_NODE_SPACE_FOR_CELLS (PAGE_SIZE - LEAF_NODE_HEADER_SIZE)
#define LEAF_NODE_MAX_CELLS (LEAF_NODE_SPACE_FOR_CELLS / LEAF_NODE_CELL_SIZE)

#define LEAF_NODE_RIGHT_SPLIT_COUNT ((LEAF_NODE_MAX_CELLS + 1) / 2)
#define LEAF_NODE_LEFT_SPLIT_COUNT ((LEAF_NODE_MAX_CELLS + 1) - LEAF_NODE_RIGHT_SPLIT_COUNT)

// Internal Node Header Layout

#define INTERNAL_NODE_NUM_KEYS_SIZE sizeof(uint8_t)
#define INTERNAL_NODE_NUM_KEYS_OFFSET COMMON_NODE_HEADER_SIZE
#define INTERNAL_NODE_RIGHT_CHILD_SIZE sizeof(uint32_t)
#define INTERNAL_NODE_RIGHT_CHILD_OFFSET (INTERNAL_NODE_NUM_KEYS_OFFSET + INTERNAL_NODE_NUM_KEYS_SIZE)
#define INTERNAL_NODE_HEADER_SIZE (COMMON_NODE_HEADER_SIZE + INTERNAL_NODE_NUM_KEYS_SIZE + INTERNAL_NODE_RIGHT_CHILD_SIZE)

// Internal Node Body Layout

#define INTERNAL_NODE_KEY_SIZE sizeof(uint32_t)
#define INTERNAL_NODE_CHILD_SIZE sizeof(uint32_t)
#define INTERNAL_NODE_CELL_SIZE (INTERNAL_NODE_KEY_SIZE + INTERNAL_NODE_CHILD_SIZE)


NodeType get_node_type(void* node);
void set_node_type(void* node, NodeType type);

void create_new_root(Table* table, uint32_t right_child_page_num);
bool is_node_root(void* node);
void set_node_root(void* node, bool value);

void initialize_leaf_node(void* node);
uint32_t* leaf_node_num_cells(void* node);
void* leaf_node_cell(void* node, uint32_t cell_num);
uint32_t* leaf_node_key(void* node, uint32_t cell_num);
void* leaf_node_value(void* node, uint32_t cell_num);

void leaf_node_insert(Cursor* cursor, uint32_t key, Row* value);
void leaf_node_split_and_insert(Cursor* cursor, uint32_t key, Row* value);
Cursor* leaf_node_find(Table* table, uint32_t page_num, uint32_t key);

void initialize_internal_node(void* node);
uint32_t* internal_node_num_keys(void* node);
uint32_t* internal_node_right_child(void* node);
uint32_t* internal_node_cell(void* node, uint32_t cell_num);
uint32_t* internal_node_child(void* node, uint32_t child_num);
uint32_t* internal_node_key(void* node, uint32_t key_num);

uint32_t get_node_max_key(void* node);


#endif // TABLE_H	
