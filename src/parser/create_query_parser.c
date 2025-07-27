#include <string.h>
#include "parser_export.h"
#include "sql_enum.h"

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

	d = cyylex();
	if (d.token_code != SQL_CREATE_Q) RETURN_PARSE_ERROR;
	d = cyylex();
	if (d.token_code != SQL_IDENTIFIER || strcmp("table", d.text) != 0) RETURN_PARSE_ERROR;
	d = cyylex();
	if (d.token_code != SQL_IDENTIFIER) RETURN_PARSE_ERROR;
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

	// COLS -> COL
	do {
		s = COL();
		if (s == PARSE_ERROR) break;
		return PARSE_SUCCESS;
	} while(0);

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
		s = TYPE();
		if (s == PARSE_ERROR) break;
		d = cyylex();
		if (d.token_code != SQL_PRIMARY_KEY) break;
		return PARSE_SUCCESS;
	} while(0);
	RESTORE_CHECK_POINT;

	// COL -> <col_name> TYPE
	do {
		d = cyylex();
		if (d.token_code != SQL_IDENTIFIER) break;
		s = TYPE();
		if (s == PARSE_ERROR) break;
		return PARSE_SUCCESS;
	} while(0);

	RETURN_PARSE_ERROR;
}

// TYPE -> int| double| varchar(<number>)
parse_status TYPE() {
	PARSE_INIT;

	d = cyylex();
	switch (d.token_code) {
		case SQL_INT:
		case SQL_DOUBLE:
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
	d = cyylex();
	if (d.token_code != SQL_BRACKET_END) RETURN_PARSE_ERROR;
	return PARSE_SUCCESS;
}
