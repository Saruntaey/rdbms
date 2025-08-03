#include <string.h>
#include "parser_export.h"
#include "sql_enum.h"
#include "sql_math_exp_interface.h"
#include "qep.h"

qep select_qep;

static parse_status COLS();
static parse_status COL();
static parse_status TABS();
static parse_status TAB();

// select_query_parser -> select COLS from TABS
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
	return PARSE_SUCCESS;
}

// COLS -> COL,COLS | COL
parse_status COLS() {
	PARSE_INIT;
	parse_status s;
	int n_cols = select_qep.select.n;

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
		memset(&select_qep.select.cols[--select_qep.select.n], 0, sizeof(select_qep.select.cols[0]));
	}

	// COLS -> COL
	do {
		s = COL();
		if (s == PARSE_ERROR) break;
		return PARSE_SUCCESS;
	} while(0);

	while (select_qep.select.n > n_cols) {
		memset(&select_qep.select.cols[--select_qep.select.n], 0, sizeof(select_qep.select.cols[0]));
	}
	RETURN_PARSE_ERROR;
}

parse_status COL() {
	sql_exp_tree *tree;

	tree = create_sql_exp_tree();
	if (!tree) {
		return PARSE_ERROR;
	}
	select_qep.select.cols[select_qep.select.n++].tree = tree;
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
