#include <string.h>
#include <stdlib.h>
#include "parser_export.h"
#include "sql_enum.h"
#include "sql_create.h"
#include "sql_const.h"

static parse_status COLS();
static parse_status COL();
static parse_status TYPE();

// S -> create table <table_name> (COLS)
// COLS -> COL | COL,COLS
// COL -> <col_name> TYPE | <col_name> TYPE primary key
// TYPE -> int| double| varchar(<number>)
parse_status create_query_parser() {
	PARSE_INIT;
	parse_status s;

	memset(&create_data, 0, sizeof(create_data));
	d = cyylex();
	if (d.token_code != SQL_CREATE_Q) RETURN_PARSE_ERROR;
	d = cyylex();
	if (d.token_code != SQL_IDENTIFIER || strcmp("table", d.text) != 0) RETURN_PARSE_ERROR;
	d = cyylex();
	if (d.token_code != SQL_IDENTIFIER) RETURN_PARSE_ERROR;
	strncpy(create_data.table_name, d.text, TABLE_NAME_SIZE - 1);
	create_data.table_name[TABLE_NAME_SIZE - 1] = '\0';
	d = cyylex();
	if (d.token_code != SQL_BRACKET_START) RETURN_PARSE_ERROR;
	s = COLS();
	if (s == PARSE_ERROR) RETURN_PARSE_ERROR;
	d = cyylex();
	if (d.token_code != SQL_BRACKET_END) RETURN_PARSE_ERROR;
	return PARSE_SUCCESS;
}

// COLS -> COL | COL,COLS
parse_status COLS() {
	PARSE_INIT;
	parse_status s;
	int n_cols = create_data.n_cols;

	// COLS -> COL,COLS
	do {
		s = COL();
		if (s == PARSE_ERROR) break;
		d = cyylex();
		if (d.token_code != SQL_COMMA) break;
		s = COLS();
		if (s == PARSE_ERROR) break;
		return PARSE_SUCCESS;
	} while(0);
	RESTORE_CHECK_POINT;
	while (create_data.n_cols > n_cols) {
		memset(&create_data.cols[--create_data.n_cols], 0, sizeof(create_data.cols[0]));
	}

	// COLS -> COL
	do {
		s = COL();
		if (s == PARSE_ERROR) break;
		return PARSE_SUCCESS;
	} while(0);

	while (create_data.n_cols > n_cols) {
		memset(&create_data.cols[--create_data.n_cols], 0, sizeof(create_data.cols[0]));
	}
	RETURN_PARSE_ERROR;
}

// COL -> <col_name> TYPE | <col_name> TYPE primary key
parse_status COL() {
	PARSE_INIT;
	parse_status s;

	// COL -> <col_name> TYPE primary key
	do {
		d = cyylex();
		if (d.token_code != SQL_IDENTIFIER) break;
		strncpy(create_data.cols[create_data.n_cols].name, d.text, COL_NAME_SIZE - 1);
		create_data.cols[create_data.n_cols].name[COL_NAME_SIZE - 1] = '\0';
		s = TYPE();
		if (s == PARSE_ERROR) break;
		d = cyylex();
		if (d.token_code != SQL_PRIMARY_KEY) break;
		create_data.cols[create_data.n_cols].is_primary_key = true;
		create_data.n_cols++;
		return PARSE_SUCCESS;
	} while(0);
	RESTORE_CHECK_POINT;
	memset(&create_data.cols[create_data.n_cols], 0, sizeof(create_data.cols[0]));

	// COL -> <col_name> TYPE
	do {
		d = cyylex();
		if (d.token_code != SQL_IDENTIFIER) break;
		strncpy(create_data.cols[create_data.n_cols].name, d.text, COL_NAME_SIZE - 1);
		create_data.cols[create_data.n_cols].name[COL_NAME_SIZE - 1] = '\0';
		s = TYPE();
		if (s == PARSE_ERROR) break;
		create_data.n_cols++;
		return PARSE_SUCCESS;
	} while(0);

	memset(&create_data.cols[create_data.n_cols], 0, sizeof(create_data.cols[0]));
	RETURN_PARSE_ERROR;
}

// TYPE -> int| double| varchar(<number>)
parse_status TYPE() {
	PARSE_INIT;

	d = cyylex();
	switch (d.token_code) {
		case SQL_INT:
		case SQL_DOUBLE:
			create_data.cols[create_data.n_cols].type = (sql_dtype_t) d.token_code;
			create_data.cols[create_data.n_cols].size = sql_dtype_size((sql_dtype_t) d.token_code);
			return PARSE_SUCCESS;
		case SQL_STRING:
			break;
		default:
			RETURN_PARSE_ERROR;
	}
	d = cyylex();
	if (d.token_code != SQL_BRACKET_START) RETURN_PARSE_ERROR;
	d = cyylex();
	if (d.token_code != SQL_INTEGER_VALUE) RETURN_PARSE_ERROR;
	create_data.cols[create_data.n_cols].type = (sql_dtype_t) d.token_code;
	create_data.cols[create_data.n_cols].size = atoi(d.text) + 1;
	d = cyylex();
	if (d.token_code != SQL_BRACKET_END) RETURN_PARSE_ERROR;
	return PARSE_SUCCESS;
}
