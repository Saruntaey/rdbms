#include <stdio.h>
#include "parser_export.h"
#include "sql_enum.h"
#include "sql_create.h"

extern parse_status create_query_parser();

int main(void) {
	PARSE_INIT;
	parse_status s;

	while (1) {
		printf("dbms=#");
		fgets(lex_buffer, sizeof(lex_buffer), stdin);
		if (lex_buffer[0] == '\n') continue;
		stack_reset();
		d = cyylex(); 
		yyrewind(1);
		switch (d.token_code) {
			case SQL_SELECT_Q: 
				break;
			case SQL_UPDATE_Q:
				break;
			case SQL_CREATE_Q:
				s = create_query_parser();
				if (s == PARSE_SUCCESS) {
					process_create_query();
				}
				break;
			case SQL_DELETE_Q:
				break;
			case SQL_INSERT_Q:
				break;
			default:
				printf("error: unrecognize input\n");
		}
	}
	return 0;
}
