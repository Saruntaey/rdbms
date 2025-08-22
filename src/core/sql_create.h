#ifndef _SQL_CREATE_
#define _SQL_CREATE_

#include <stdbool.h>
#include "sql_enum.h"
#include "sql_const.h"


typedef struct sql_create_data {
	char table_name[TABLE_NAME_SIZE];
	int n_cols;
	struct {
		char name[COL_NAME_SIZE];
		sql_dtype_t type;
		int size;
		bool is_primary_key;
	} cols[MAX_COLS_PER_TABLE];
} sql_create_data;

extern sql_create_data create_data;

void process_create_query();

#endif
