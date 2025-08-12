#ifndef TABLE_H
#define TABLE_H

#include <stdint.h>
#include <stdio.h>


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
#define ROWS_PER_PAGE (PAGE_SIZE / ROW_SIZE)
#define TABLE_MAX_ROWS (ROWS_PER_PAGE * TABLE_MAX_PAGES)


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
	void* pages[TABLE_MAX_ROWS];
} Pager;

Pager* pager_open(const char* filename);
void* get_page(Pager* pager, uint32_t page_num);
void pager_flush(Pager* pager, uint32_t page_num, uint32_t size);


typedef struct
{
	uint32_t num_rows;
	Pager* pager;
} Table;

Table* db_open(const char* filename);
void db_close(Table* table);
void* row_slot(Table* table, uint32_t row_num);


#endif // TABLE_H
