#ifndef _CATALOG_
#define _CATALOG_

#include "BPlusTree.h"
#include "sql_const.h"
#include "sql_enum.h"
#include "sql_create.h"

extern BPlusTree_t catalog;

typedef struct schema_rec {
	char col_name[COL_NAME_SIZE];
	sql_dtype_t dtype;
	int size;
	bool is_primary_key;
	int offset;
} schema_rec;

typedef struct catalog_rec {
	char table_name[TABLE_NAME_SIZE];
	BPlusTree_t *schema_table;
	BPlusTree_t *record_table;
	char col_name[MAX_COLS_PER_TABLE + 1][COL_NAME_SIZE];
} catalog_rec;

bool catalog_insert_new_table(BPlusTree_t *catalog, sql_create_data *create_data);
void catalog_table_print(BPlusTree_t *catalog);
static void schema_table_print(BPlusTree_t *schema_table);

#endif
