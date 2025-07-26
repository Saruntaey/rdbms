%{
#include "parser_export.h"
#include "sql_enum.h"

char lex_buffer[MAX_STR_SIZE];
char *curr_ptr = lex_buffer;
stack lex_stack = {.top = -1};


static void stack_push(lex_data v) {
	assert(lex_stack.top+1 < STACK_CAP);
	lex_stack.data[++lex_stack.top] = v;
}

static lex_data stack_pop() {
	assert(lex_stack.top > -1);
	return lex_stack.data[lex_stack.top--];
}

static int stack_size() {
	return lex_stack.top + 1;
}

static lex_data stack_peek() {
	assert(lex_stack.top > -1);
	return lex_stack.data[lex_stack.top];
}

void stack_reset() {
	lex_data d;

	while (stack_size() > 0) {
		d = stack_pop();
		free(d.text);
	}
	yy_scan_string(lex_buffer);
	curr_ptr = lex_buffer;
}

lex_data cyylex() {
	int token_code = yylex();
	lex_data d = {.token_code = token_code, .len = yyleng};
	d.text = (char*) malloc(yyleng+1);
	strncpy(d.text, yytext, yyleng+1);
	stack_push(d);
	curr_ptr += yyleng;
	return d;
}

void yyrewind(int n) {
	int i;

	while (n > 0 && lex_stack.top != -1) {
		i = lex_stack.top;
		free(lex_stack.data[i].text);
		lex_stack.data[i].text = NULL;
		if (lex_stack.data[i].token_code != PARSER_WHITE_SPACE) {
			n--;
		}
		curr_ptr -= lex_stack.data[lex_stack.top--].len;
	}
	yy_scan_string(curr_ptr);
}

void restore(int _chkp) {
	assert(_chkp <= lex_stack.top);
	lex_data *p;
	while (lex_stack.top > _chkp) {
		p = &lex_stack.data[lex_stack.top--];
		free(p->text);
		p->text = NULL;
		curr_ptr -= p->len;
	}
	yy_scan_string(curr_ptr);
}

static void process_white_space(int n) {
	lex_data d = {.token_code = PARSER_WHITE_SPACE, .len = n};
	stack_push(d);
	curr_ptr += n;
}

%}

%%


"select" {
    return SQL_SELECT_Q;
}

"create" {
    return SQL_CREATE_Q;
}

"insert into" {
    return SQL_INSERT_Q;
}

"drop" {
    return SQL_DELETE_Q;
}

"varchar" {
    return SQL_STRING;
}

"int" {
    return SQL_INT;
}

"decimal" {
    return SQL_DOUBLE;
}

"primary key" {
    return SQL_PRIMARY_KEY;
}

"not null" {
    return SQL_NOT_NULL;
}

"max" {
    return SQL_MAX;
}

"sum" {
    return SQL_SUM;
}

"min" {
    return SQL_MIN;
}

"count" {
    return SQL_COUNT;
}

"avg" {
    return SQL_AVG;
}

"from" {
    return SQL_FROM;
}

"where" {
    return SQL_WHERE;
}

"group by" {
    return SQL_GROUP_BY;
}

"order by" {
    return SQL_ORDER_BY;
}

"limit" {
    return SQL_LIMIT;
}

"having" {
    return SQL_HAVING;
}

"asc" {
    return SQL_ORDERBY_ASC;
}

"desc" {
    return SQL_ORDERBY_DSC;
}

"(" {
    return SQL_BRACKET_START;
}

")" {
    return SQL_BRACKET_END;
}

"'" {
    return SQL_QUOTATION_MARK;
}

"<" {
    return SQL_LESS_THAN;
}

">" {
    return SQL_GREATER_THAN;
}

"=" {
    return SQL_EQ;
}

"!=" {
    return SQL_NOT_EQ;
}

"and" {
    return SQL_AND;
}

"or" {
    return SQL_OR;
}

"as" {
    return SQL_AS;
}

"*" {
    return SQL_MATH_MUL;
}

"+" {
    return SQL_MATH_PLUS;
}

"-" {
    return SQL_MATH_MINUS;
}

"/" {
    return SQL_MATH_DIV;
}

"in" {
    return SQL_IN;
}

"LIKE" {
   
    return SQL_LIKE;
}

"sqrt" {
    return SQL_MATH_SQRT;
}

"sqr" {
    return SQL_MATH_SQR;
}

"hmax" {
    return SQL_MATH_MAX;
}

"hmin" {
    return SQL_MATH_MIN;
}

"sin" {
    return SQL_MATH_SIN;
}

"pow" {
    return SQL_MATH_POW;
}

\n {
    return PARSER_EOL;
}

"," {
    return SQL_COMMA;
}

[ ] {
    /* Ignore */
    process_white_space(1);
}

[\t] {
    /*ignore*/
    process_white_space(4);
}

0|-?[1-9][0-9]* {
    return SQL_INTEGER_VALUE;
}

-?[0-9]*\.[0-9]+ {
    return SQL_DOUBLE_VALUE;
}

[a-zA-Z0-9_]+ {
    return SQL_IDENTIFIER;
}

[a-zA-Z0-9_]+\.[a-zA-Z0-9_]+ {
    return SQL_IDENTIFIER_IDENTIFIER;
}

'[^']*' {
    // string enclosed within single quotes. This regular expression allows multiple words separated by spaces
    return SQL_STRING_VALUE;
}

\"[^"]*\" {
    // string enclosed within double quotes. This regular expression allows multiple words separated by spaces
    return SQL_STRING_VALUE;
}

. {
	// ignore
}

%%
