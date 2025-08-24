#include <string.h>
#include "parser_export.h"
#include "sql_enum.h"
#include "sql_math_exp_interface.hpp"
#include "qep.hpp"

qep select_qep;

static parse_status COLS();
static parse_status COL();
static parse_status TABS();
static parse_status TAB();
static parse_status where_PREDICATE();
static parse_status as_variable();

// select_query_parser -> select COLS from TABS [where PREDICATE]
// COLS -> COL,COLS | COL
// COL -> <MathExpr>
// TABS -> TAB,TABS | TAB
// TAB -> <tableName>
parse_status select_query_parser() {
	PARSE_INIT;
	parse_status s;

	memset(&select_qep, 0, sizeof(select_qep));
	d = cyylex();
	if (d.token_code != SQL_SELECT_Q) RETURN_PARSE_ERROR;
	s = COLS();
	if (s == PARSE_ERROR) RETURN_PARSE_ERROR;
	d = cyylex();
	if (d.token_code != SQL_FROM) RETURN_PARSE_ERROR;
	s = TABS();
	if (s == PARSE_ERROR) RETURN_PARSE_ERROR;
	s = where_PREDICATE();
	if (s == PARSE_ERROR) RETURN_PARSE_ERROR;
	d = cyylex();
	if (d.token_code != PARSER_EOL) RETURN_PARSE_ERROR;
	return PARSE_SUCCESS;
}

parse_status where_PREDICATE() {
	PARSE_INIT;
	sql_predicate_exp_tree *tree;
	
	do {
		d = cyylex();
		if (d.token_code != SQL_WHERE) break;
		tree = create_sql_predicate_exp_tree();
		if (!tree) break; 
		select_qep.where.tree = tree;
		return PARSE_SUCCESS;
	}while(0);
	RESTORE_CHECK_POINT;

	return PARSE_SUCCESS;
}

// COLS -> COL,COLS | COL
parse_status COLS() {
	PARSE_INIT;
	parse_status s;
	int n_cols = select_qep.select.n;
	int i;

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
	while (select_qep.select.n > n_cols) {
		i = --select_qep.select.n;
		free(select_qep.select.cols[i].display_name);
		memset(&select_qep.select.cols[i], 0, sizeof(select_qep.select.cols[0]));
	}

	// COLS -> COL
	do {
		s = COL();
		if (s == PARSE_ERROR) break;
		return PARSE_SUCCESS;
	} while(0);

	while (select_qep.select.n > n_cols) {
		i = --select_qep.select.n;
		free(select_qep.select.cols[i].display_name);
		memset(&select_qep.select.cols[i], 0, sizeof(select_qep.select.cols[0]));
	}
	RETURN_PARSE_ERROR;
}

char *get_display_name(int exp_start) {
	char *display_name;
	int n;
	int i;
	int count;
	int j;

	count = 0;
	n = 0;
	for (i = exp_start; i <= lex_stack.top; i++) {
		if (lex_stack.data[i].token_code == PARSER_WHITE_SPACE) {
			continue;
		}
		count++;
		n += lex_stack.data[i].len;
	}
	display_name = (char *) malloc(n + count);
	j = 0;
	for (i = exp_start; i <= lex_stack.top; i++) {
		if (lex_stack.data[i].token_code == PARSER_WHITE_SPACE) {
			continue;
		}
		strncpy(display_name + j, lex_stack.data[i].text, lex_stack.data[i].len);
		j += lex_stack.data[i].len;
		display_name[j++] = ' ';
	}
	display_name[j] = '\0';
	return display_name;
}

// COL -> <MathExpr> [as <vairable>]
parse_status COL() {
	PARSE_INIT;
	parse_status s;
	sql_exp_tree *tree;
	int exp_start;

	exp_start = lex_stack.top + 1;
	tree = create_sql_exp_tree();
	if (!tree) {
		return PARSE_ERROR;
	}
	select_qep.select.cols[select_qep.select.n].tree = tree;
	s = as_variable();
	if (s == PARSE_ERROR) {
		select_qep.select.cols[select_qep.select.n].display_name = get_display_name(exp_start);
	}
	select_qep.select.n++;
	return PARSE_SUCCESS;
}

parse_status as_variable() {
	PARSE_INIT;
	char **ptr;

	ptr = &select_qep.select.cols[select_qep.select.n].display_name;
	d = cyylex();
	if (d.token_code != SQL_AS) RETURN_PARSE_ERROR;
	d = cyylex();
	if (d.token_code != SQL_IDENTIFIER) RETURN_PARSE_ERROR;
	*ptr = (char *) malloc(d.len+1);
	strncpy(*ptr, d.text, d.len);
	(*ptr)[d.len] = '\0';
	return PARSE_SUCCESS;
}

// TABS -> TAB,TABS | TAB
parse_status TABS() {
	PARSE_INIT;
	parse_status s;
	int n_tabs = select_qep.join.n;

	// TABS -> TAB,TABS
	do {
		s = TAB();
		if (s == PARSE_ERROR) break;
		d = cyylex();
		if (d.token_code != SQL_COMMA) break;
		s = TABS();
		if (s == PARSE_ERROR) break;
		return PARSE_SUCCESS;
	} while(0);
	RESTORE_CHECK_POINT;
	while (select_qep.join.n > n_tabs) {
		memset(&select_qep.join.tables[--select_qep.join.n], 0, sizeof(select_qep.join.tables[0]));
	}

	// TABS -> TAB
	do {
		s = TAB();
		if (s == PARSE_ERROR) break;
		return PARSE_SUCCESS;
	} while(0);
	while (select_qep.join.n > n_tabs) {
		memset(&select_qep.join.tables[--select_qep.join.n], 0, sizeof(select_qep.join.tables[0]));
	}
	RETURN_PARSE_ERROR;
}

parse_status TAB() {
	PARSE_INIT;

	d = cyylex();
	if (d.token_code != SQL_IDENTIFIER) RETURN_PARSE_ERROR;
	strncpy(select_qep.join.tables[select_qep.join.n].name, d.text, TABLE_NAME_SIZE - 1);
	select_qep.join.tables[select_qep.join.n].name[TABLE_NAME_SIZE - 1] = '\0';
	select_qep.join.tables[select_qep.join.n].c_rec = nullptr;
	select_qep.join.n++;
	return PARSE_SUCCESS;
}
