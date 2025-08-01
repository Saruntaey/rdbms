#ifndef _SQL_INSERT_INTO_
#define _SQL_INSERT_INTO_

#include "sql_enum.h"
#include "sql_const.h"

typedef struct sql_val {
	sql_dtype_t dtype;
	union {
		char str[STRING_VALUE_MAX_SIZE];
		int i;
		double d;
	} val;
} sql_val;

typedef struct sql_insert_data {
	char table_name[TABLE_NAME_SIZE];
	int n_cols;
	sql_val cols[MAX_COLS_PER_TABLE];
} sql_insert_data;

extern sql_insert_data insert_data;
void process_insert_into_query();

#endif
