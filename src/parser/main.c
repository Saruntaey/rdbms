#include <stdio.h>
#include "parser_export.h"
#include "sql_enum.h"
#include "sql_create.h"
#include "sql_drop.h"
#include "sql_insert_into.h"
#include "qep.h"

extern qep select_qep;
extern parse_status create_query_parser();
extern parse_status drop_table_parser(char *table_name, int size);
extern parse_status insert_into_query_parser();
extern parse_status select_query_parser();

int main(void) {
	PARSE_INIT;
	parse_status s;
	char table_name[TABLE_NAME_SIZE];

	while (1) {
		printf("dbms=#");
		fgets(lex_buffer, sizeof(lex_buffer), stdin);
		if (lex_buffer[0] == '\n') continue;
		stack_reset();
		d = cyylex(); 
		yyrewind(1);
		switch (d.token_code) {
			case SQL_SELECT_Q: 
				s = select_query_parser();
				if (s == PARSE_SUCCESS) {
					// todo
					printf("select parsed success\n");
				}
				qep_destroy(&select_qep);
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
				s = drop_table_parser(table_name, TABLE_NAME_SIZE);
				if (s == PARSE_SUCCESS) {
					drop_table(table_name);
				}
				break;
			case SQL_INSERT_Q:
				s = insert_into_query_parser();
				if (s == PARSE_SUCCESS) {
					process_insert_into_query();
				}
				break;
			default:
				printf("error: unrecognize input\n");
		}
	}
	return 0;
}
