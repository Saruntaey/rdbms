#include <string.h>
#include "parser_export.h"
#include "sql_enum.h"

parse_status drop_table_parser(char *table_name, int size) {
	PARSE_INIT;

	d = cyylex();
	if (d.token_code != SQL_DELETE_Q) RETURN_PARSE_ERROR;
	d = cyylex();
	if (d.token_code != SQL_IDENTIFIER || strcmp("table", d.text) != 0) RETURN_PARSE_ERROR;
	d = cyylex();
	if (d.token_code != SQL_IDENTIFIER) RETURN_PARSE_ERROR;
	strncpy(table_name, d.text, size - 1);
	table_name[size - 1] = '\0';
	return PARSE_SUCCESS;
}
