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
	    // New database file. Initialize page 0 as leaf node.
		void* root_node = get_page(pager, 0);
		initialize_leaf_node(root_node);
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

Cursor* table_end(Table* table)
{
	Cursor* cursor = malloc(sizeof(Cursor));
	if (!cursor)
	{
		perror("malloc error");
		exit(EXIT_FAILURE);
	}

	cursor->table = table;
	cursor->page_num = table->root_page_num;
	
	void* root_node = get_page(table->pager, table->root_page_num);
	uint32_t num_cells = *leaf_node_num_cells(root_node);
	cursor->cell_num = num_cells;
	cursor->end_of_table = true;

	return cursor;
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

	if (cursor->cell_num >= (*leaf_node_num_cells(node)))
		cursor->end_of_table = true;
}

uint32_t* leaf_node_num_cells(void* node)
{
	return (uint32_t*)node + LEAF_NODE_NUM_CELLS_OFFSET;
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

void initialize_leaf_node(void* node)
{
	*(leaf_node_num_cells(node)) = 0;
}

void leaf_node_insert(Cursor* cursor, uint32_t key, Row* value)
{
	void* node = get_page(cursor->table->pager, cursor->page_num);
	uint32_t num_cells = *leaf_node_num_cells(node);

	if (num_cells >= LEAF_NODE_MAX_CELLS)
	{
		printf("Need to implement splitting a leaf node.\n");
		exit(EXIT_FAILURE);
	}

	if (cursor->cell_num < num_cells)
		for (uint32_t i = num_cells; i > cursor->cell_num; --i)
			memcpy(leaf_node_cell(node, i), leaf_node_cell(node, i - 1), LEAF_NODE_CELL_SIZE);

	*(leaf_node_num_cells(node)) += 1;
	*(leaf_node_key(node, cursor->cell_num)) = key;
	serialize_row(value, leaf_node_value(node, cursor->cell_num));
}
