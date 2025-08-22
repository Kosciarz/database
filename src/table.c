#include "table.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


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


Pager* pager_open(const char* filename)
{
	FILE* file_ptr = fopen(filename, "r+b");
	if (!file_ptr)
	{
		file_ptr = fopen(filename, "w+b");
		if (!file_ptr)
		{
			perror("fopen error");
			exit(EXIT_FAILURE);
		}
	}

	fseek(file_ptr, 0, SEEK_END);
	long file_length = ftell(file_ptr);
	rewind(file_ptr);

	Pager* pager = malloc(sizeof(Pager));
	if (!pager)
	{
		perror("malloc error");
		exit(EXIT_FAILURE);
	}

	pager->file_ptr = file_ptr;
	pager->file_length = file_length;
	pager->num_pages = (file_length / PAGE_SIZE);
	if (file_length % PAGE_SIZE != 0)
	{
		fprintf(stderr, "Error: DB file is not a whole number of pages. Corrupt file.\n");
		exit(EXIT_FAILURE);
	}

	for (uint32_t i = 0; i < TABLE_MAX_PAGES; ++i)
		pager->pages[i] = NULL;

	return pager;
}

void* get_page(Pager* pager, uint32_t page_num)
{
	if (page_num > TABLE_MAX_PAGES)
	{
		fprintf(stderr, "Error: Tried to fetch page number out of bounds: %d\n", TABLE_MAX_PAGES);
		exit(EXIT_FAILURE);
	}

	if (!pager->pages[page_num])
	{
		void* page = malloc(PAGE_SIZE);
		uint32_t num_pages = pager->file_length / PAGE_SIZE;
		
		if (pager->file_length % PAGE_SIZE)
			num_pages++;

		if (page_num <= num_pages)
		{
            if (fseek(pager->file_ptr, page_num * PAGE_SIZE, SEEK_SET) != 0)
            {
				perror("fseek error");
				exit(EXIT_FAILURE);
            }

            size_t bytes_read = fread(page, 1, PAGE_SIZE, pager->file_ptr);
            if (bytes_read < PAGE_SIZE && ferror(pager->file_ptr))
            {
				perror("fread error");
				exit(EXIT_FAILURE);
            }
		}

		pager->pages[page_num] = page;

		if (page_num >= pager->num_pages)
			pager->num_pages = page_num + 1;
	}

	return pager->pages[page_num];
}

void pager_flush(Pager* pager, uint32_t page_num)
{
    if (!pager->pages[page_num])
    {
		fprintf(stderr, "Error: Tried to flush NULL page\n");
		exit(EXIT_FAILURE);
    }

    if (fseek(pager->file_ptr, page_num * PAGE_SIZE, SEEK_SET) != 0)
    {
		perror("fseek error");
		exit(EXIT_FAILURE);
    }

    size_t bytes_written = fwrite(pager->pages[page_num], 1, PAGE_SIZE, pager->file_ptr);
    if (bytes_written < PAGE_SIZE)
    {
		perror("fwrite error");
		exit(EXIT_FAILURE);
    }
}

uint32_t get_unused_page_num(Pager* pager)
{
	return pager->num_pages;
}


Table* db_open(const char* filename)
{
	Pager* pager = pager_open(filename);
	Table* table = malloc(sizeof(Table));
	if (!table)
	{
		perror("Malloc failed");
		exit(EXIT_FAILURE);
	}

	table->pager = pager;
	table->root_page_num = 0;

	if (pager->num_pages == 0)
	{
		void* root_node = get_page(pager, 0);
		initialize_node(root_node, NODE_LEAF);
		set_node_root(root_node, true);
	}

	return table;
}

void db_close(Table* table)
{
	Pager* pager = table->pager;

	for (uint32_t i = 0; i < pager->num_pages; ++i)
	{
		if (pager->pages[i] == NULL)
			continue;

		pager_flush(pager, i);
		free(pager->pages[i]);
		pager->pages[i] = NULL;
	}

	if (fclose(pager->file_ptr))
	{
		perror("fclose error");
		exit(EXIT_FAILURE);
	}

	for (uint32_t i = 0; i < TABLE_MAX_PAGES; ++i)
	{
		void* page = pager->pages[i];
		if (page)
		{
			free(page);
			pager->pages[i] = NULL;
		}
	}

	free(pager);
	free(table);
}


Cursor* table_start(Table* table)
{
	Cursor* cursor = malloc(sizeof(Cursor));
	if (!cursor)
	{
		perror("malloc error");
		exit(EXIT_FAILURE);
	}

	cursor->table = table;
	cursor->page_num = table->root_page_num;
	cursor->cell_num = 0;

	void* root_node = get_page(table->pager, table->root_page_num);
	uint32_t num_cells = *leaf_node_num_cells(root_node);
	cursor->end_of_table = (num_cells == 0);

	return cursor;
}

Cursor* table_find(Table* table, uint32_t key)
{
	void* root_node = get_page(table->pager, table->root_page_num);
	if (get_node_type(root_node) == NODE_LEAF)
		return leaf_node_find(table, table->root_page_num, key);
	else	
		return internal_node_find(table, table->root_page_num, key);
}

void* cursor_value(Cursor* cursor)
{
	void* page = get_page(cursor->table->pager, cursor->page_num);
	return leaf_node_value(page, cursor->cell_num);
}

void cursor_advance(Cursor* cursor)
{
	void* node = get_page(cursor->table->pager, cursor->page_num);
	cursor->cell_num++;

	if (cursor->cell_num >= *leaf_node_num_cells(node))
		cursor->end_of_table = true;
}


NodeType get_node_type(void* node)
{
	uint8_t value = *((uint8_t*)node + NODE_TYPE_OFFSET);
	return (NodeType)value;
}

void set_node_type(void* node, NodeType type)
{
	*((uint8_t*)node + NODE_TYPE_OFFSET) = type;
}


void create_new_root(Table* table, uint32_t right_child_page_num)
{
	void* root = get_page(table->pager, table->root_page_num);
	uint32_t left_child_page_num = get_unused_page_num(table->pager);
	void* left_child = get_page(table->pager, left_child_page_num);

	memcpy(left_child, root, PAGE_SIZE);
	set_node_root(left_child, false);

	initialize_node(root, NODE_INTERNAL);
	set_node_root(root, true);
	*internal_node_num_keys(root) = 1;
	*internal_node_child(root, 0) = left_child_page_num;
	uint32_t left_child_max_key = get_node_max_key(left_child);
	*internal_node_key(root, 0) = left_child_max_key;
	*internal_node_right_child(root) = right_child_page_num;
}

bool is_node_root(void* node)
{
	uint8_t value = *((uint8_t*)node + IS_ROOT_OFFSET);
	return (bool)value;
}

void set_node_root(void* node, bool value)
{
	*((uint8_t*)node + IS_ROOT_OFFSET) = value;
}


void initialize_node(void* node, NodeType type)
{
	set_node_type(node, type);
	set_node_root(node, false);
	switch (type)
	{
	case NODE_LEAF:
		*leaf_node_num_cells(node) = 0;
		break;
	case NODE_INTERNAL:
		*internal_node_num_keys(node) = 0;
		break;
	}
}


uint8_t* leaf_node_num_cells(void* node)
{
	return (uint8_t*)node + LEAF_NODE_NUM_CELLS_OFFSET;
}

void* leaf_node_cell(void* node, uint32_t cell_num)
{
	return (uint8_t*)node + LEAF_NODE_HEADER_SIZE + cell_num * LEAF_NODE_CELL_SIZE;
}

uint32_t* leaf_node_key(void* node, uint32_t cell_num)
{
	return leaf_node_cell(node, cell_num);
}

void* leaf_node_value(void* node, uint32_t cell_num)
{
	return (uint8_t*)leaf_node_cell(node, cell_num) + LEAF_NODE_KEY_SIZE;
}

void leaf_node_insert(Cursor* cursor, uint32_t key, Row* value)
{
	void* node = get_page(cursor->table->pager, cursor->page_num);
	uint32_t num_cells = *leaf_node_num_cells(node);

	if (num_cells >= LEAF_NODE_MAX_CELLS)
	{
		leaf_node_split_and_insert(cursor, key, value);
		return;
	}

	if (cursor->cell_num < num_cells)
		for (uint32_t i = num_cells; i > cursor->cell_num; --i)
			memcpy(leaf_node_cell(node, i), leaf_node_cell(node, i - 1), LEAF_NODE_CELL_SIZE);
	 
	*leaf_node_num_cells(node) += 1;
	*leaf_node_key(node, cursor->cell_num) = key;
	serialize_row(value, leaf_node_value(node, cursor->cell_num));
}

void leaf_node_split_and_insert(Cursor* cursor, uint32_t key, Row* value)
{
	void* old_node = get_page(cursor->table->pager, cursor->page_num);
	uint32_t new_page_num = get_unused_page_num(cursor->table->pager);
	void* new_node = get_page(cursor->table->pager, new_page_num);
	initialize_node(new_node, NODE_LEAF);

	for (int32_t i = LEAF_NODE_MAX_CELLS; i >= 0; --i)
	{
		void* destination_node;
		if (i >= LEAF_NODE_LEFT_SPLIT_COUNT)
			destination_node = new_node;
		else
			destination_node = old_node;

		uint32_t index_within_node = i % LEAF_NODE_LEFT_SPLIT_COUNT;
		void* destination = leaf_node_cell(destination_node, index_within_node);

		if (i == cursor->cell_num)
			serialize_row(value, destination);
		if (i > cursor->cell_num)
			memcpy(destination, leaf_node_cell(old_node, i - 1), LEAF_NODE_CELL_SIZE);
		if (i < cursor->cell_num)
			memcpy(destination, leaf_node_cell(old_node, i), LEAF_NODE_CELL_SIZE);
	}

	*leaf_node_num_cells(old_node) = LEAF_NODE_LEFT_SPLIT_COUNT;
	*leaf_node_num_cells(new_node) = LEAF_NODE_RIGHT_SPLIT_COUNT;

	if (is_node_root(old_node))
	{
		create_new_root(cursor->table, new_page_num);
	}
	else
	{
		printf("Need to implement updating parent after spliting.\n");
		exit(EXIT_FAILURE);
	}
}

Cursor* leaf_node_find(Table* table, uint32_t page_num, uint32_t key)
{
	void* node = get_page(table->pager, page_num);
	uint32_t num_cells = *leaf_node_num_cells(node);

	Cursor* cursor = malloc(sizeof(Cursor));
	if (!cursor)
	{
		perror("malloc error");
		exit(EXIT_FAILURE);
	}

	cursor->table = table;
	cursor->page_num = page_num;

	uint32_t min_index = 0;
	uint32_t one_past_max_index = num_cells;
	while (one_past_max_index != min_index)
	{
		uint32_t index = (min_index + one_past_max_index) / 2;
		uint32_t key_at_index = *leaf_node_key(node, index);
		if (key == key_at_index)
		{
			cursor->cell_num = index;
			return cursor;
		}
		if (key < key_at_index)
			one_past_max_index = index;
		if (key > key_at_index)
			min_index = index + 1;
	}

	cursor->cell_num = min_index;
	return cursor;
}


uint8_t* internal_node_num_keys(void* node)
{
	return (uint8_t*)node + INTERNAL_NODE_NUM_KEYS_OFFSET;
}

uint32_t* internal_node_right_child(void* node)
{
	return (uint32_t*)((uint8_t*)node + INTERNAL_NODE_RIGHT_CHILD_OFFSET);
}

uint32_t* internal_node_cell(void* node, uint32_t cell_num)
{
	return (uint32_t*)((uint8_t*)node + INTERNAL_NODE_HEADER_SIZE + cell_num * INTERNAL_NODE_CELL_SIZE);
}

uint32_t* internal_node_child(void* node, uint32_t child_num)
{
	uint32_t num_keys = *internal_node_num_keys(node);
	if (child_num > num_keys)
	{
		printf("Tried to access child_num %d > num_keys %d.\n", child_num, num_keys);
		exit(EXIT_FAILURE);
	}
	if (child_num == num_keys)
		return internal_node_right_child(node);
	if (child_num < num_keys)
		return internal_node_cell(node, child_num);

	return NULL;
}

uint32_t* internal_node_key(void* node, uint32_t key_num)
{
	return internal_node_cell(node, key_num) + INTERNAL_NODE_CHILD_SIZE;
}

Cursor* internal_node_find(Table* table, uint32_t page_num, uint32_t key)
{
	void* node = get_page(table->pager, page_num);
	uint8_t num_keys = *internal_node_num_keys(node);

	uint32_t min_index = 0;
	uint32_t max_index = num_keys;
	
	while (min_index != max_index)
	{
		uint8_t index = (min_index + max_index) / 2;
		uint32_t key_at_right = *internal_node_key(node, index);

		if (key_at_right >= key)
			max_index = index;
		else
			min_index = index + 1;
	}

	uint32_t child_num = *internal_node_child(node, min_index);
	void* child = get_page(table->pager, child_num);
	switch (get_node_type(child))
	{
	case NODE_LEAF:
		return leaf_node_find(table, child_num, key);
	case NODE_INTERNAL:
		return internal_node_find(table, child_num, key);
	}

	return NULL;
}


uint32_t get_node_max_key(void* node)
{
	switch (get_node_type(node))
	{
	case NODE_INTERNAL:
		return *internal_node_key(node, *internal_node_num_keys(node));
	case NODE_LEAF:
		return *leaf_node_key(node, *leaf_node_num_cells(node) - 1);
	}
	return -1;
}
