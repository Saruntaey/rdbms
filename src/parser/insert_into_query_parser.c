#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "parser_export.h"
#include "sql_enum.h"
#include "sql_insert_into.h"
#include "sql_const.h"

static parse_status VALUES();
static parse_status VALUE();

// insert_into_query_parser -> insert into <tableName> values(VALUES)
// VALUES -> VALUE | VALUE, VALUES
// VALUE -> <int> | <double> | "<string>" | '<string>'
parse_status insert_into_query_parser() {
	PARSE_INIT;
	parse_status s;

	memset(&insert_data, 0, sizeof(insert_data));
	d = cyylex();
	if (d.token_code != SQL_INSERT_Q) RETURN_PARSE_ERROR;
	d = cyylex();
	if (d.token_code != SQL_IDENTIFIER) RETURN_PARSE_ERROR;
	strncpy(insert_data.table_name, d.text, TABLE_NAME_SIZE - 1);
	insert_data.table_name[TABLE_NAME_SIZE - 1] = '\0';
	d = cyylex();
	if (d.token_code != SQL_IDENTIFIER || strcmp("values", d.text) != 0) RETURN_PARSE_ERROR;
	d = cyylex();
	if (d.token_code != SQL_BRACKET_START) RETURN_PARSE_ERROR;
	s = VALUES();
	if (s == PARSE_ERROR) RETURN_PARSE_ERROR;
	d = cyylex();
	if (d.token_code != SQL_BRACKET_END) RETURN_PARSE_ERROR;
	return PARSE_SUCCESS;
}

// VALUES -> VALUE | VALUE, VALUES
parse_status VALUES() {
	PARSE_INIT;
	parse_status s;
	int n_cols = insert_data.n_cols;

	// VALUES -> VALUE, VALUES
	do {
		s = VALUE();
		if (s == PARSE_ERROR) break;
		d = cyylex();
		if (d.token_code != SQL_COMMA) break;
		s = VALUES();
		if (s == PARSE_ERROR) break;
		return PARSE_SUCCESS;
	} while(0);
	RESTORE_CHECK_POINT;
	while (insert_data.n_cols > n_cols) {
		memset(&insert_data.cols[--insert_data.n_cols], 0, sizeof(insert_data.cols[0]));
	}

	// VALUES -> VALUE
	do {
		s = VALUE();
		if (s == PARSE_ERROR) break;
		return PARSE_SUCCESS;
	} while(0);

	while (insert_data.n_cols > n_cols) {
		memset(&insert_data.cols[--insert_data.n_cols], 0, sizeof(insert_data.cols[0]));
	}
	RETURN_PARSE_ERROR;
}

// VALUE -> <int> | <double> | "<string>" | '<string>'
parse_status VALUE() {
	PARSE_INIT;
	sql_val *v = &insert_data.cols[insert_data.n_cols];

	d = cyylex();
	switch (d.token_code) {
		case SQL_INTEGER_VALUE:
			v->dtype = SQL_INT;
			v->val.i = atoi(d.text);
			break;
		case SQL_DOUBLE_VALUE:
			v->dtype = SQL_DOUBLE;
			v->val.d = atof(d.text);
			break;
		case SQL_STRING_VALUE:
			v->dtype = SQL_STRING;
			if (sizeof(v->val.str) <= d.len - 2) {
				printf("error: %s(%d) buffer overflow\n", __FUNCTION__, __LINE__);
				RETURN_PARSE_ERROR;
			}
			strncpy(v->val.str, d.text + 1, d.len-2);
			break;
		default:
			RETURN_PARSE_ERROR;
	}
	insert_data.n_cols++;
	return PARSE_SUCCESS;
}
