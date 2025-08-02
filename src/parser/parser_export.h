#ifndef _PARSER_EXPORT_
#define _PARSER_EXPORT_

extern "C" int yylex();
#include <assert.h>
#define MAX_STR_SIZE 512
#define STACK_CAP 512
#define PARSER_EOL  10000
#define PARSER_QUIT 10001
#define PARSER_WHITE_SPACE 10002

typedef enum parse_status {
	PARSE_ERROR,
	PARSE_SUCCESS,
} parse_status;

typedef struct lex_data {
	int token_code;
	int len;
	char *text;
} lex_data;

typedef struct stack {
	int top;
	lex_data data[STACK_CAP];
} stack;

extern char lex_buffer[MAX_STR_SIZE];
extern char *curr_ptr;
extern stack lex_stack;

lex_data cyylex(); 
void yyrewind(int n); 
void stack_reset(); 
void restore(int _chkp); 

#define PARSE_INIT \
	lex_data d; \
	int _chkp = lex_stack.top;

#define RETURN_PARSE_ERROR { \
	restore(_chkp); \
	return PARSE_ERROR; \
}

#define RESTORE_CHECK_POINT { \
	restore(_chkp); \
}

#endif
