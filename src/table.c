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


Table* new_table(void)
{
	Table* table = malloc(sizeof(Table));
	if (!table)
	{
		perror("Malloc failed");
		exit(EXIT_FAILURE);
	}

	table->num_rows = 0;
	for (uint32_t i = 0; i < TABLE_MAX_PAGES; ++i)
		table->pages[i] = NULL;

	return table;
}

void free_table(Table* table)
{
	for (uint32_t i = 0; i < TABLE_MAX_PAGES; ++i)
		if (table->pages[i] != NULL)
			free(table->pages[i]);
	free(table);
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
	return (uint8_t*)(page)+byte_offset;
}
