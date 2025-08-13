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
	}

	return pager->pages[page_num];
}

void pager_flush(Pager* pager, uint32_t page_num, uint32_t size)
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

    size_t bytes_written = fwrite(pager->pages[page_num], 1, size, pager->file_ptr);
    if (bytes_written < size)
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
	table->num_rows = pager->file_length / ROW_SIZE;

	return table;
}

void db_close(Table* table)
{
	Pager* pager = table->pager;
	uint32_t num_full_pages = table->num_rows / ROWS_PER_PAGE;

	for (uint32_t i = 0; i < num_full_pages; ++i)
	{
		if (pager->pages[i] == NULL)
			continue;

		pager_flush(pager, i, PAGE_SIZE);
		free(pager->pages[i]);
		pager->pages[i] = NULL;
	}

	uint32_t num_additional_rows = table->num_rows % ROWS_PER_PAGE;
	if (num_additional_rows > 0)
	{
		uint32_t page_num = num_full_pages;
		if (pager->pages[page_num])
		{
			pager_flush(pager, page_num, num_additional_rows * ROW_SIZE);
			free(pager->pages[page_num]);
			pager->pages[page_num] = NULL;
		}
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

void* row_slot(Table* table, uint32_t row_num)
{
	uint32_t page_num = row_num / ROWS_PER_PAGE;
	void* page = get_page(table->pager, page_num);

	uint32_t row_offset = row_num % ROWS_PER_PAGE;
	uint32_t byte_offset = row_offset * ROW_SIZE;
	return (uint8_t*)(page) + byte_offset;
}
